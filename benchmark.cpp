#include<cstring>
#include<chrono>
#include<random>
#include<vector>

#include "wavelet_tree.h"


void benchmark_rank(uint64_t startLen, uint64_t endLen, uint64_t stepSize, uint64_t queryCount)
{
    puts("Benchmarking of rank queries.\n");
    printf("n\t\tTime(s)\t\tOverhead\n==========\n");

    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    double elapsedSecs;
    
    for(uint64_t bitCount = startLen; bitCount <= endLen; bitCount += stepSize)
    {
        bit_vector b;
        b.generate_random_bitvector(bitCount);
        
        rank_support r(&b);

        elapsedSecs = 0;
        for(uint64_t i = 0; i < queryCount; ++i)
        {
            uint64_t queryIdx = std::uniform_int_distribution<uint64_t>(0, bitCount - 1)(rng);

            std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();
            r.rank1(queryIdx);
            std::chrono::high_resolution_clock::time_point t_end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t_end - t_start);
            elapsedSecs += time_span.count();
        }


        printf("%llu\t\t%lf\t\t%llu\n", (unsigned long long)bitCount, elapsedSecs, (unsigned long long)r.overhead());
    }
}



void benchmark_select(uint64_t startLen, uint64_t endLen, uint64_t stepSize, uint64_t queryCount)
{
    puts("Benchmarking of select queries.\n");
    printf("n\t\tTime(s)\t\tOverhead\n==========\n");

    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    double elapsedSecs;
    
    for(uint64_t bitCount = startLen; bitCount <= endLen; bitCount += stepSize)
    {
        bit_vector b;
        b.generate_random_bitvector(bitCount);
        
        rank_support r(&b);
        select_support s(&r);

        uint64_t maxRank1 = r.rank1(bitCount - 1);

        elapsedSecs = 0;
        for(uint64_t i = 0; i < queryCount; ++i)
        {
            uint64_t queryRank = std::uniform_int_distribution<uint64_t>(1, maxRank1)(rng);

            std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();
            s.select1(queryRank);
            std::chrono::high_resolution_clock::time_point t_end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t_end - t_start);
            elapsedSecs += time_span.count();
        }


        printf("%llu\t\t%lf\t\t%llu\n", (unsigned long long)bitCount, elapsedSecs, (unsigned long long)s.overhead());
    }
}



void benchmark_wt_rank_fixed_alphabet(uint64_t startLen, uint64_t endLen, uint64_t stepSize, uint64_t queryCount)
{
    puts("Benchmarking of wavelet tree rank queries with fixed alphabet size (100).\n");
    printf("n\t\tTime(s)\t\tOverhead\n==========\n");

    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    double elapsedSecs;
    
    for(uint64_t len = startLen; len <= endLen; len += stepSize)
    {
        char *s = new char[len + 1];
        for(uint64_t i = 0; i < len; ++i)
            while((s[i] = std::uniform_int_distribution<uint64_t>(1, 100)(rng)) == '\n');

        s[len] = '\0';
       
        // puts(s);
        // for(int i = 0; i < len; ++i)
        //     printf("%d --> %d\n", i, s[i]);

        std::string text(s);
        wavelet_tree w(text);
       

        elapsedSecs = 0;
        for(uint64_t i = 0; i < queryCount; ++i)
        {
            uint64_t queryIdx = std::uniform_int_distribution<uint64_t>(0, len - 1)(rng);

            std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();
            w.rank(queryIdx);
            std::chrono::high_resolution_clock::time_point t_end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t_end - t_start);
            elapsedSecs += time_span.count();
        }


        delete(s);


        printf("%llu\t\t%lf\n", (unsigned long long)len, elapsedSecs);
    }
}



void benchmark_wt_select_fixed_alphabet(uint64_t startLen, uint64_t endLen, uint64_t stepSize, uint64_t queryCount)
{
    puts("Benchmarking of wavelet tree select queries with fixed alphabet size (100).\n");
    printf("n\t\tTime(s)\t\tOverhead\n==========\n");

    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    double elapsedSecs;
    uint64_t H[256];
    
    for(uint64_t len = startLen; len <= endLen; len += stepSize)
    {
        std::map<char, uint8_t> charMap;
        memset(H, 0, sizeof H);

        char *s = new char[len + 1];
        for(uint64_t i = 0; i < len; ++i)
        {
            while((s[i] = std::uniform_int_distribution<uint64_t>(1, 100)(rng)) == '\n');
            H[s[i]]++;
            
            if(charMap.find(s[i]) == charMap.end())
                charMap[s[i]] = 0;
        }

        uint8_t distinct = 0;
        for(auto p = charMap.begin(); p != charMap.end(); ++p)
            p -> second = distinct++;

        s[len] = '\0';

        std::string text(s);
        wavelet_tree w(text);
       

        elapsedSecs = 0;
        for(uint64_t i = 0; i < queryCount; ++i)
        {
            char queryChar = std::uniform_int_distribution<uint64_t>(1, 100)(rng);
            if(!H[queryChar])
                continue;

            uint64_t queryIdx = std::uniform_int_distribution<uint64_t>(1, H[queryChar])(rng);

            std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();
            w.select(H[queryChar], queryIdx);
            std::chrono::high_resolution_clock::time_point t_end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t_end - t_start);
            elapsedSecs += time_span.count();
        }


        delete(s);


        printf("%llu\t\t%lf\n", (unsigned long long)len, elapsedSecs);
    }
}



void benchmark_wt_rank_fixed_len(uint8_t maxSigmaSize, const uint64_t fixedLen, uint64_t queryCount)
{
    puts("Benchmarking of wavelet tree rank queries with fixed len (10M).\n");
    printf("|Sigma|\t\tTime(s)\t\tOverhead\n==========\n");

    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    double elapsedSecs;
    
    char *s = new char[fixedLen + 1];

    for(uint8_t i = 2; i <= maxSigmaSize; ++i)
    {
        for(uint64_t j = 0; j < fixedLen; ++j)
            while((s[j] = std::uniform_int_distribution<uint64_t>(1, i)(rng)) == '\n');
        
        s[fixedLen] = '\0';


        std::string text(s);
        wavelet_tree w(text);


        elapsedSecs = 0;
        for(uint64_t j = 0; j < queryCount; ++j)
        {
            uint64_t queryIdx = std::uniform_int_distribution<uint64_t>(0, fixedLen - 1)(rng);

            std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();
            w.rank(queryIdx);
            std::chrono::high_resolution_clock::time_point t_end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t_end - t_start);
            elapsedSecs += time_span.count();
        }

        printf("%u\t\t%lf\n", (unsigned)i, elapsedSecs);
    }   
}



void benchmark_wt_select_fixed_len(uint8_t maxSigmaSize, const uint64_t fixedLen, uint64_t queryCount)
{
    puts("Benchmarking of wavelet tree select queries with fixed len (10M).\n");
    printf("|Sigma|\t\tTime(s)\t\tOverhead\n==========\n");

    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    double elapsedSecs;
    uint64_t H[256];
    
    char *s = new char[fixedLen + 1];

    for(uint8_t i = 2; i <= maxSigmaSize; ++i)
    {
        std::map<char, uint8_t> charMap;

        memset(H, 0, sizeof H);

        for(uint64_t j = 0; j < fixedLen; ++j)
        {
            while((s[j] = std::uniform_int_distribution<uint64_t>(1, i)(rng)) == '\n');
            H[s[j]]++;

            if(charMap.find(s[j]) == charMap.end())
                charMap[s[j]] = 0;
        }

        uint8_t distinct = 0;
        for(auto p = charMap.begin(); p != charMap.end(); ++p)
            p -> second = distinct++;
        
        s[fixedLen] = '\0';


        std::string text(s);
        wavelet_tree w(text);


        elapsedSecs = 0;
        for(uint64_t j = 0; j < queryCount; ++j)
        {
            char queryChar = std::uniform_int_distribution<uint64_t>(1, i)(rng);
            if(!H[queryChar])
                continue;

            uint64_t queryIdx = std::uniform_int_distribution<uint64_t>(1, H[queryChar])(rng);

            std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();
            w.select(H[queryChar], queryIdx);
            std::chrono::high_resolution_clock::time_point t_end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t_end - t_start);
            elapsedSecs += time_span.count();
        }


        printf("%u\t\t%lf\n", (unsigned)i, elapsedSecs);
    }   
}



int main(int argc, char *argv[])
{
    uint64_t startLen = 101000000;    // 1M
    uint64_t endLen = 200000000;    // 100M
    uint64_t stepSize = 1000000;    // 1M
    uint64_t queryCount = 1000000;  // 1M
    
    
    // benchmark_rank(startLen, endLen, stepSize, queryCount);

    // benchmark_select(startLen, endLen, stepSize, queryCount);

    // benchmark_wt_rank_fixed_alphabet(startLen, endLen, stepSize, queryCount);

    // benchmark_wt_select_fixed_alphabet(startLen, endLen, stepSize, queryCount);


    uint8_t maxSigmaSize = 128;     
    uint64_t fixedSize = 10000000; // 10M

    // benchmark_wt_rank_fixed_len(maxSigmaSize, fixedSize, queryCount);

    benchmark_wt_select_fixed_len(maxSigmaSize, fixedSize, queryCount);

    return 0;
}