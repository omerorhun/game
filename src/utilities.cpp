
#include <stdio.h>
#include <ctype.h>

#include "utilities.h"

void print_hex(const char *header, char *buffer, uint16_t len) {
    
    printf("[%s]: (%d)\n", header, len);
    printf("---------------------------------------------------\n");
    
    // header line
    printf("%12s 00 10 20 30 40 50 60 70 80 90 A0 B0 C0 D0 E0 F0\n", " ");
    printf("%12s -----------------------------------------------\n", " ");
    
    bool is_new_line = true;
    for (uint16_t i = 0; i < len; i++) {
        
        if(is_new_line) {
            // header
            printf("0x%08x | ", i);
            is_new_line = false;
        }
        
        printf("%02x ", (uint8_t)buffer[i]);
        if (((i+1) % 0x10) == 0) {
            for (uint16_t j = i+1 - 0x10; j < i+1; j++) {
                if (isprint((int)buffer[j])) putchar(buffer[j]);
                else putchar('.');
            }
            putchar('\n');
            is_new_line = true;
        }
        
        if (i == len - 1) {
            uint16_t rem = 0x10 - (i+1) % 0x10;
            if (rem == 0x10)
                break;
            
            for (uint16_t j = 0; j < rem; j++) printf("00 ");
            for (uint16_t j = i - 0x10 + rem; j < i; j++) {
                if (isprint((int)buffer[j])) putchar(buffer[j]);
                else putchar('.');
            }
            for (uint16_t j = 0; j < rem; j++) putchar('.');
            putchar('\n');
        }
    }
    
    putchar('\n');
}


#define CRC16 0x8005

uint16_t gen_crc16(const uint8_t *data, uint16_t size) {
    uint16_t out = 0;
    int bits_read = 0, bit_flag;
    
    /* Sanity check: */
    if (data == NULL)
        return 0;
    
    while (size > 0) {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

        /* Increment bit counter: */
        bits_read++;
        if (bits_read > 7) {
            bits_read = 0;
            data++;
            size--;
        }
        
        /* Cycle check: */
        if (bit_flag)
            out ^= CRC16;
    }

    // item b) "push out" the last 16 bits
    int i;
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if (bit_flag)
            out ^= CRC16;
    }
    
    // item c) reverse the bits
    uint16_t crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>=1, j <<= 1) {
        if (i & out)
            crc |= j;
    }

    return crc;
}
