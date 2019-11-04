#include<cstdio>
#include<random>
#include<functional>

#include "rank_support.h"

using namespace std;

void unit_test_1()
{
    puts("Unit test");


    uint64_t len = 10;

    auto gen = bind(std::uniform_int_distribution<>(0,1), std::default_random_engine());
    bool *B = new bool[len];

    for(int i = 0; i < len; ++i)
        B[i] = gen();

    bit_vector bits(B, len);
    printf("Bit vector bits instantiated with len = %llu\n", (unsigned long long)bits.get_len());


    for(int i = len - 1; i >= 0; --i)
        putchar(B[i] ? '1' : '0');
    putchar('\n');

    bits.print();

    delete B;
}

void unit_test_2()
{
    bit_vector bits(12);

    bits.set_int(0, 3, 3);
    bits.set_int(3, 3, 5);
    bits.set_int(6, 3, 2);
    bits.set_int(9, 3, 6);

    bits.print();

    for(int i = 0; i + 2 < 12; i += 3)
        printf("get_int(%d, %d) = %llu\n", i, 3, (unsigned long long)bits.get_int(i, 3));

    for(int i = 1; i + 2 < 12; i += 3)
        printf("get_int(%d, %d) = %llu\n", i, 3, (unsigned long long)bits.get_int(i, 3));

    for(int i = 0; i + 3 < 12; i += 4)
        printf("get_int(%d, %d) = %llu\n", i, 4, (unsigned long long)bits.get_int(i, 4));
}


void rank_support_build_test()
{
    uint64_t len = 16;

    bit_vector bits(len);

    bits.set_int(0, 3, 3);
    bits.set_int(3, 3, 5);
    bits.set_int(6, 3, 2);
    bits.set_int(9, 3, 6);

    bits.print();

    rank_support r(&bits);

    // printf("rank6 (%llu) = %llu\n", (unsigned long long)6, (unsigned long long)r.rank1(6));

    for(uint64_t i = 0; i < len; ++i)
        printf("rank1 (%llu) = %llu\n", (unsigned long long)i, (unsigned long long)r.rank1(i));
}

void rank_support_build_test_2(uint64_t n)
{
    auto gen = bind(std::uniform_int_distribution<>(0,1), std::default_random_engine());
    bool *B = new bool[n];

    for(int i = 0; i < n; ++i)
        B[i] = gen();

    // printf("Bitvector: ");
    // uint64_t i = n;
    // do
    //     putchar(B[--i] ? '1' : '0');
    // while(i > 0);
    // putchar('\n');

    bit_vector bits(B, n);
    rank_support r(&bits);

    // bits.print();


    // uint64_t idx = 10;
    // printf("rank1 (%llu) = %llu\n", (unsigned long long)idx, (unsigned long long)r.rank1(idx));


    uint64_t rank = 0, error = 0;
    for(uint64_t i = 0; i < n; ++i)
    {
        if(B[i])
            rank++;

        printf("Rank[%llu] = %llu vs %llu\n", (unsigned long long) i, (unsigned long long)rank,
                (unsigned long long)r.rank1(i));

        if(r.rank1(i) != rank)
            printf("Mismatch at idx %llu. Expected %llu, found %llu.\n",
                    (unsigned long long)i, (unsigned long long)rank,
                    (unsigned long long)r.rank1(i)), error++;
        // else
        //     printf("Rank[%llu] = %llu matched both\n", (unsigned long long) i, (unsigned long long)rank);
        
    }

    printf("Error count = %llu.\n", (unsigned long long)error);
}


int main()
{
    // unit_test_1();

    // unit_test_2();

    // rank_support_build_test();

    rank_support_build_test_2(15321);

    return 0;
}