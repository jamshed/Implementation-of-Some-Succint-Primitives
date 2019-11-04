#include<cmath>
#include<iostream>

#include "bit_vector.h"


class rank_support
{
private:
    bit_vector *B;  //  Bitvector on which the rank-support data-structure is built upon.
    bit_vector R_s; // The superblocks bitvector.
    bit_vector R_b; // The blocks bitvector.

    uint64_t bitCount;  // Number of bits in the bitvector B.

    // All the following sizes are strongly based on the assumption that the maximum
    // length of the input bitvector B is 2^64; on the grounds that it is not ossible
    // to address addresses of length more than 64 bits at modern architectures.

    uint16_t supBlkLen;         // (log^2 n) / 2
    uint8_t supBlkWrdSz;        // log n
    uint64_t supBlkCnt;         // 2n / (log^2 n)

    uint8_t blkLen;             // (log n) / 2
    uint16_t blkWrdSz;          // (2 log log n) - 1
    uint8_t blkCntPerSupBlk;    // log n


    inline void set_superblock_value(uint64_t idx, uint64_t val);
    inline void set_block_value(uint64_t supBlkIdx, uint8_t blkIdx, uint64_t val);
    void build();
    void dump_metadata();


public:
    rank_support(bit_vector *b);

    uint64_t rank1(uint64_t idx);
    uint64_t rank0(uint64_t idx);
    uint64_t overhead();
};



rank_support::rank_support(bit_vector *b)
{
    B = b;
    bitCount = B -> get_len();

    std::cout << "bitcount = " << bitCount << "\n";

    supBlkLen = ceil(pow(log2(bitCount), 2) / 2);
    supBlkWrdSz = ceil(log2(bitCount));
    supBlkCnt = ceil(double(bitCount) / supBlkLen);

    blkLen = ceil(log2(bitCount) / 2);
    blkWrdSz = ceil(log2(supBlkLen));
    blkCntPerSupBlk = ceil(double(supBlkLen) / blkLen);


    R_s.set_len(supBlkCnt * supBlkWrdSz);
    R_b.set_len((supBlkCnt * blkCntPerSupBlk) * blkWrdSz);

    build();
    dump_metadata();
}



void rank_support::dump_metadata()
{
    std::cout << "Superblock length = " << supBlkLen << "\n";
    std::cout << "Superblock word size = " << (uint16_t)supBlkWrdSz << "\n";
    std::cout << "Superblock count = " << supBlkCnt << "\n";
    
    std::cout << "===================================\n";

    std::cout << "Block length = " << (uint16_t)blkLen << "\n";
    std::cout << "Block word size = " << (uint16_t)blkWrdSz << "\n";
    std::cout << "Block cout per superblock = " << (uint16_t)blkCntPerSupBlk << "\n";
}



void rank_support::build()
{
    uint64_t supBlkVal = 0;
    for(uint64_t i = 0; i < supBlkCnt; ++i)
    {
        set_superblock_value(i, supBlkVal);

        // printf("R_s[%d] = %d\n", (int)i, (int)supBlkVal);

        uint64_t blkVal = 0;
        for(uint8_t j = 0; j < blkCntPerSupBlk; ++j)
        {
            set_block_value(i, j, blkVal);

            // printf("R_b[%d][%d] = %d\n", (int)i, (int)j, (int)blkVal);
            
            uint64_t bitOffset = i * supBlkLen + j * blkLen;
            // printf("bitOffset = %d\n", (int)bitOffset);

            for(uint8_t k = 0; k < blkLen && (bitOffset + k) < (i + 1) * supBlkLen && (bitOffset + k) < bitCount; ++k)
            {
                // printf("bit_%d = %d\n", (int)(bitOffset + k), B -> get_bit(bitOffset + k));

                if(B -> get_bit(bitOffset + k))
                    blkVal++, supBlkVal++;
            }
        }
    }
}



void rank_support::set_superblock_value(uint64_t idx, uint64_t val)
{
    R_s.set_int(idx * supBlkWrdSz, supBlkWrdSz, val);
}



void rank_support::set_block_value(uint64_t supBlkIdx, uint8_t blkIdx, uint64_t val)
{
    R_b.set_int((supBlkIdx * blkCntPerSupBlk + blkIdx) * blkWrdSz, blkWrdSz, val);
}


uint64_t rank_support::rank1(uint64_t idx)
{
    uint64_t supBlk = idx / supBlkLen;
    uint8_t blk = (idx % supBlkLen) / blkLen;
    uint8_t inBlkBit = (idx - (supBlk * supBlkLen + (uint64_t)blk * blkLen));
    

    uint64_t supBlkVal = R_s.get_int(supBlk * supBlkWrdSz, supBlkWrdSz);
    uint64_t blkVal = R_b.get_int((supBlk * blkCntPerSupBlk + blk) * blkWrdSz, blkWrdSz);

    uint64_t inBlkVal = B -> get_int(supBlk * supBlkLen + (uint64_t)blk * blkLen, inBlkBit + 1);
    inBlkVal = __builtin_popcount(inBlkVal);

    // printf("R_b = ");
    // R_b.print();


    // printf("supBlk = %d\n", (int)supBlk);
    // printf("supBlkVal = %d\n", (int)R_s.get_int(supBlk * supBlkWrdSz, supBlkWrdSz));

    // printf("blk = %d\n", (int)blk);
    // printf("Block starts at %d\n", int((supBlk * blkCntPerSupBlk + blk) * blkWrdSz));
    // printf("blkVal = %d\n", (int)R_b.get_int((supBlk * blkCntPerSupBlk + blk) * blkWrdSz, blkWrdSz));

    return  supBlkVal + blkVal + inBlkVal;

    // uint64_t supBlkVal = R_s.get_int(idx / supBlkLen, supBlkWrdSz);
    // uint64_t blkVal = R_b.get_int((idx % supBlkLen) / blkLen, blkWrdSz);
    // uint64_t inBlkVal = B -> get_int()
}
