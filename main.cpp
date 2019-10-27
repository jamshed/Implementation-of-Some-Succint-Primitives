#include<cstdio>
#include<random>
#include<functional>
#include "bit_vector.h"

using namespace std;

void unit_test_1()
{
    puts("Unit test");


    uint64_t len = 1;

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

    bits.set_int(0, 3, 0);
    bits.set_int(3, 3, 1);
    bits.set_int(6, 3, 2);
    bits.set_int(9, 3, 3);

    bits.print();

    for(int i = 0; i < 12; i += 3)
        printf("get_int(%d, %d) = %llu\n", i, 3, (unsigned long long)bits.get_int(i, 3));

    for(int i = 1; i < 12; i += 3)
        printf("get_int(%d, %d) = %llu\n", i, 3, (unsigned long long)bits.get_int(i, 3));
}

int main()
{
    unit_test_1();

    unit_test_2();

    return 0;
}