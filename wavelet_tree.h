#include<iostream>
#include<fstream>
#include<string>
#include<map>
#include<unordered_map>
#include<cmath>
#include<algorithm>

#include "select_support.h"


class wavelet_tree
{
private:
    uint8_t left;       // Left limit of the alphabet.
    uint8_t right;      // Right limit of the alphabet.
    bit_vector B;       // Bitvector at the root.
    uint8_t wrdSz;      // Bit-length for each text character.
    bit_vector words;   // Text characters in this tree, in exact order.
    wavelet_tree *wt_l; // Left subtree.
    wavelet_tree *wt_r; // Right subtree.
    rank_support r;     // Rank support for the bitvector B.
    select_support s;   // Select support on rank support r.


    void build(std::string &text, std::map<char, uint8_t> &charMap);
    void build(uint8_t l, uint8_t r);


public:
    wavelet_tree() {}
    wavelet_tree(uint8_t l, uint8_t r, uint64_t len, uint8_t wrdSz);
    wavelet_tree(std::string &inputFile, std::string &outputFile);

    char access(std::string wtFileName, std::string acessIndices);
    uint64_t rank(uint64_t idx);
    uint64_t select(uint8_t ch, uint64_t rank);
};



wavelet_tree::wavelet_tree(std::string &inputFile, std::string &outputFile)
{
    std::ifstream inpFile(inputFile);
    std::string text;


    // Read in the text.

    std::getline(inpFile, text);
    inpFile.close();


    // Map the arbitrary alphabet to a [0, sigma) range.

    uint8_t distinctChar = 0;
    std::map<char, uint8_t> charMap;

    for(auto p = text.begin(); p != text.end(); ++p)
        if(charMap.find(*p) == charMap.end())
            charMap[*p] = 0;

    for(auto p = charMap.begin(); p != charMap.end(); ++p)
        p -> second = distinctChar++;


    // Serialize the text as is, to disk;
    // Serialize this hash map into the same file, with metadata.


    // Build the wavelet tree.
    build(text, charMap);


    // Serialize the wavelet tree to the same file.
}



wavelet_tree::wavelet_tree(uint8_t l, uint8_t r, uint64_t len, uint8_t wordSize):
    left(l),
    right(r),
    B(len),
    wrdSz(wordSize),
    words(len * wordSize)
{
}



void wavelet_tree::build(std::string &text, std::map<char, uint8_t> &charMap)
{
    left = 0, right = charMap.size() - 1;

    B.set_len(text.length());

    wrdSz = ceil(log2(charMap.size()));
    words.set_len(text.length() * wrdSz);

    for(uint64_t i = 0; i < text.length(); ++i)
        words.set_int(i * wrdSz, wrdSz, charMap[text[i]]);

    build(0, charMap.size() - 1);
}



void wavelet_tree::build(uint8_t l, uint8_t r)
{
    // printf("build(%d, %d). text len = %d\n", (int)l, (int)r, (int)B.get_len());

    if(l == r)
        return;

    uint8_t mid = (l + r) / 2;
    uint64_t countL = 0, countR = 0, wordsVecSz = words.get_len();

    for(uint64_t i = 0; i < wordsVecSz; i += wrdSz)
    {
        uint8_t wrd = words.get_int(i, wrdSz);
        if(wrd <= mid)
            countL++;
        else
            countR++;
    }


    wt_l = new wavelet_tree(l, mid, countL, wrdSz);
    wt_r = new wavelet_tree(mid + 1, r, countR, wrdSz);

    
    countL = 0, countR = 0;
    for(uint64_t i = 0; i < wordsVecSz; i += wrdSz)
    {
        uint8_t wrd = words.get_int(i, wrdSz);
        if(wrd <= mid)
        {
            B.reset_bit(i / wrdSz);
            wt_l -> words.set_int(countL * wrdSz, wrdSz, wrd);
            countL++;
        }
        else
        {
            B.set_bit(i / wrdSz);
            wt_r -> words.set_int(countR * wrdSz, wrdSz, wrd);
            countR++;
        }
    }

    
    (this -> r).build(&B);
    s.build(&(this -> r));


    wt_l -> build(l, mid);
    wt_r -> build(mid + 1, r);
}



uint64_t wavelet_tree::rank(uint64_t idx)
{
    if(left == right)
        return idx <= B.get_len() ? idx + 1 : std::numeric_limits<uint64_t>::max();

    
    bool bit = B.get_bit(idx);

    if(!bit)
    {
        uint64_t rankVal = r.rank0(idx);
        return wt_l -> rank(rankVal - 1);
    }
    

    uint64_t rankVal = r.rank1(idx);
    return wt_r -> rank(rankVal - 1);
}



uint64_t wavelet_tree::select(uint8_t ch, uint64_t rank)
{
    if(left == right)
        return rank <= B.get_len() ? rank - 1 : std::numeric_limits<uint64_t>::max();

    if(ch <= (left + right) / 2)
    {
        uint64_t nxtLvlIdx = wt_l -> select(ch, rank);
        uint64_t currLvlIdx = s.select0(nxtLvlIdx + 1);

        return currLvlIdx;        
    }

    uint64_t nxtLvlIdx = wt_r -> select(ch, rank);
    uint64_t currLvlIdx = s.select1(nxtLvlIdx + 1);

    return currLvlIdx;
}
