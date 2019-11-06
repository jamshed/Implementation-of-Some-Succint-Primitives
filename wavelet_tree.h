#include<iostream>
#include<fstream>
#include<cstdio>
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


    wavelet_tree(uint8_t l, uint8_t r, uint64_t len, uint8_t wrdSz);

    void build(std::string &text, std::map<char, uint8_t> &charMap);
    void build(uint8_t l, uint8_t r);
    void serialize(std::ofstream &output, std::string &text, std::map<char, uint8_t> &charMap);
    void serialize_wavelet_tree(std::ofstream &output);


public:
    wavelet_tree() {}
    wavelet_tree(std::string &inputFile, std::string &outputFile);

    uint64_t rank(uint64_t idx);
    uint64_t select(uint8_t ch, uint64_t rank);

    void deserialize(std::string &waveletFile, std::string &text, std::map<char, uint8_t> &charMap);
    void deserialize_wavelet_tree(std::ifstream &input);

    static void access_queries(std::string &wtFileName, std::string &accessIndices);
    static void rank_queries(std::string &wtFileName, std::string &queryIndices);
    static void select_queries(std::string &wtFileName, std::string &queryIndices);
};



wavelet_tree::wavelet_tree(std::string &inputFile, std::string &outputFile)
{
    std::ifstream input(inputFile);
    std::string text;


    // Read in the text.

    std::getline(input, text);
    input.close();


    // Map the arbitrary alphabet to a [0, sigma) range.
    // Maintaining the lexicographical order (ASCII) here for ease of analysis and debug.
    // Any arbitrary ordering should suffice in practice.

    uint8_t distinctChar = 0;
    std::map<char, uint8_t> charMap;

    for(auto p = text.begin(); p != text.end(); ++p)
        if(charMap.find(*p) == charMap.end())
            charMap[*p] = 0;

    for(auto p = charMap.begin(); p != charMap.end(); ++p)
        p -> second = distinctChar++;


    // Build the wavelet tree.
    build(text, charMap);


    // Seralize the text, the character mapping, and the wavelet tree.

    std::ofstream output;
    output.open(outputFile.c_str(), std::ios::binary | std::ios::out);

    serialize(output, text, charMap);

    output.close();


    std::cout << "Size of the alphabet the tree is constructed over: " << (unsigned)distinctChar << "\n";
    std::cout << "Number of characters in the input string: " << text.length() << "\n";
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
        return; // No need to build the bitvector, and can be left as is (all zeroes based on the initialization)

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



void wavelet_tree::serialize(std::ofstream &output, std::string &text, std::map<char, uint8_t> &charMap)
{
    // Serialize the text length, followed by the text itself.
    // (Required for future access(idx) operations).
    uint64_t textLen = text.length();

    output.write((const char *)&textLen, sizeof(textLen));
    output.write(text.c_str(), text.length());


    // Serialize the hash map size, followed by the hass map itself.
    // (Required for future select(ch, rank) operations).
    uint64_t mapSize = charMap.size();
    
    output.write((const char *)&mapSize, sizeof(mapSize));
    for(auto p = charMap.begin(); p != charMap.end(); ++p)
    {
        output.write(&p -> first, sizeof(p -> first));
        output.write((const char *)&p -> second, sizeof(p -> second));
    }


    // Serialize the wavelet tree.
    serialize_wavelet_tree(output);
}



void wavelet_tree::serialize_wavelet_tree(std::ofstream &output)
{
    

    // Serialize the character range.

    output.write((const char *)&left, sizeof(left));
    output.write((const char *)&right, sizeof(right));

    // Serialize the bitvector.
    B.serialize(output);

    // Serialize the word size in words bitvector.
    output.write((const char *)&wrdSz, sizeof(wrdSz));

    // Serialize the words bitvector.
    words.serialize(output);

    
    // Serialize the rank_support.
    r.serialize(output);

    // No serialization required for the select_support.

    // Recursively serialize the left and right wavelet trees, if existent.
    if(left < right)
    {
        wt_l -> serialize_wavelet_tree(output);
        wt_r -> serialize_wavelet_tree(output);
    }
}



void wavelet_tree::deserialize(std::string &waveletFile, std::string &text, std::map<char, uint8_t> &charMap)
{
    std::ifstream input;
    input.open(waveletFile.c_str(), std::ios::binary | std::ios::in);

    // std::cout << "Deserializing\n";

    uint64_t textLen;
    input.read((char *)&textLen, sizeof(textLen));
    // std::cout << "text len = " << textLen << "\n";

    char *s = new char[textLen + 1]();
    input.read(s, textLen);
    text = std::string(s);
    // std::cout << "text: " << s << "\n";


    uint64_t mapSize;
    input.read((char *)&mapSize, sizeof(mapSize));
    // std::cout << "Map size = " << mapSize << "\n";


    for(uint64_t i = 0; i < mapSize; ++i)
    {
        char ch;
        uint8_t val;

        input.read(&ch, sizeof(ch));
        input.read((char *)&val, sizeof(val));

        charMap[ch] = val;

        // std::cout << "Map " << ch << " " << (unsigned)val << "\n";
    }


    deserialize_wavelet_tree(input);

    input.close();

    // std::cout << "Deserialization completed.\n";
}



void wavelet_tree::deserialize_wavelet_tree(std::ifstream &input)
{
    // Deserialize the character range.

    input.read((char *)&left, sizeof(left));
    input.read((char *)&right, sizeof(right));

    // std::cout << "Left = " << (unsigned)left << ", Right = " << (unsigned)right << "\n";

    // Deserialize the bitvector.

    B.deserialize(input);

    // std::cout << "Bitvector length = " << B.get_len() << "\n";
    // B.print();

    // Deserialize the word size.

    input.read((char *)&wrdSz, sizeof(wrdSz));

    // std::cout << "Word size = " << (unsigned)wrdSz << "\n";

    // Deserialize the words bitvector.
    
    words.deserialize(input);

    // std::cout << "Words bitvector length = " << words.get_len() << "\n";
    // words.print();

    // Deserialize the rank support, and pass the underlying bitvector B to it.

    r.deserialize(&B, input);

    // Pass the deserialized rank support to the select support.

    s.build(&r);


    // Recursively desrialize the left and the right wavelet subtrees, if exist.

    wt_l = new wavelet_tree();
    wt_r = new wavelet_tree();

    if(left < right)
    {
        wt_l -> deserialize_wavelet_tree(input);
        wt_r -> deserialize_wavelet_tree(input);
    }
}



void wavelet_tree::access_queries(std::string &wtFileName, std::string &accessIndices)
{
    std::string text;
    std::map<char, uint8_t> charMap;

    wavelet_tree wt;
    wt.deserialize(wtFileName, text, charMap);

    
    std::ifstream input(accessIndices);
    uint64_t idx;

    while(input >> idx)
        std::cout << text[idx] << "\n";
}



void wavelet_tree::rank_queries(std::string &wtFileName, std::string &queryIndices)
{
    std::string text;
    std::map<char, uint8_t> charMap;

    wavelet_tree wt;
    wt.deserialize(wtFileName, text, charMap);

    
    std::ifstream input(queryIndices);
    char ch;
    uint64_t idx;

    while(input >> ch >> idx)
        std::cout << wt.rank(idx) << "\n";
}



void wavelet_tree::select_queries(std::string &wtFileName, std::string &queryIndices)
{
    std::string text;
    std::map<char, uint8_t> charMap;

    wavelet_tree wt;
    wt.deserialize(wtFileName, text, charMap);

    
    std::ifstream input(queryIndices);
    char ch;
    uint64_t rank;

    while(input >> ch >> rank)
        std::cout << wt.select(charMap[ch], rank) << "\n";
}
