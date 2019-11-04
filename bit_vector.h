#include<cstdint>
#include<cstdio>


class bit_vector
{
private:
    const static int UNIT_WIDTH = 8;
    const static uint64_t ALL_ONES_UNIT = 0xFF;

    uint64_t len;
    unsigned char *B;

public:
    // bit_vector() { B = new unsigned char; }

    bit_vector(uint64_t len);

    bit_vector(bool *bits, uint64_t len);

    ~bit_vector()
    {
        delete B;
    }


    inline uint64_t get_len() { return len; }
    inline bool get_bit(uint64_t idx);
    inline void set_bit(uint64_t idx);
    inline void reset_bit(uint64_t idx);
    void set_int(uint64_t idx, uint64_t len, uint64_t val);
    uint64_t get_int(uint64_t idx, uint64_t len);
    void print();
};



bit_vector::bit_vector(bool *bits, uint64_t len)
{
    this -> len = len;
    B = new unsigned char[(len + UNIT_WIDTH - 1) / UNIT_WIDTH];

    for(uint64_t i = 0; i < len; ++i)
        bits[i] ? set_bit(i) : reset_bit(i);
}



bit_vector::bit_vector(uint64_t len)
{
    this -> len = len;
    B = new unsigned char[(len + UNIT_WIDTH - 1) / UNIT_WIDTH]();
}



bool bit_vector::get_bit(uint64_t idx)
{
    return (B[idx / UNIT_WIDTH]) & (1u << (idx % UNIT_WIDTH));
}



void bit_vector::set_bit(uint64_t idx)
{
    B[idx / UNIT_WIDTH] |= (1u << (idx % UNIT_WIDTH));
}



void bit_vector::reset_bit(uint64_t idx)
{
    B[idx / UNIT_WIDTH] &= ~(1u << (idx % UNIT_WIDTH));
}



void bit_vector::set_int(uint64_t idx, uint64_t len, uint64_t val)
{
    while(len && idx % UNIT_WIDTH)
    {
        (val & 1) ? set_bit(idx) : reset_bit(idx);
        idx++, len--, val >>= 1;
    }

    while(len >= UNIT_WIDTH)
    {
        B[idx / UNIT_WIDTH] = (val & ALL_ONES_UNIT);
        idx += UNIT_WIDTH, len -= UNIT_WIDTH, val >>= UNIT_WIDTH;
    }

    while(len)
    {
        (val & 1) ? set_bit(idx) : reset_bit(idx);
        idx++, len--, val >>= 1;
    }
}



uint64_t bit_vector::get_int(uint64_t idx, uint64_t len)
{
    uint64_t val = 0, i = 0;

    while(i < len && idx % UNIT_WIDTH)
    {
        if(get_bit(idx))
            val |= (1 << i);

        idx++, i++;
    }

    while((len - i) >= UNIT_WIDTH)
    {
        val |= (B[idx / UNIT_WIDTH] << i);
        idx += UNIT_WIDTH, i += UNIT_WIDTH;
    }

    while(i < len)
    {
        if(get_bit(idx))
            val |= (1 << i);

        idx++, i++;
    }

    return val;
}



void bit_vector::print()
{
    uint64_t i = len;
    do
    {
        i--;
        putchar(get_bit(i) ? '1' : '0');
    }
    while(i > 0);

    putchar('\n');
}