#include <stdio.h>

bool bIsBigEndian() {
    union uEndianTester {
        unsigned int m_int;
        unsigned char m_byte[4];
    };

    uEndianTester stTester;
    stTester.m_int = 0x0a0b0c0d;

    if (stTester.m_byte[0] == 0x0a) {
        return true;
    }
    return false;
}

int main()
{
    if (bIsBigEndian())
    {
        printf("Big Endian\n");
    }
    else
    {
        printf("Little Endian\n");
    }
    return 0;
}