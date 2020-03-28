#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <stdint.h>

#define B0(x)               (uint8_t)((x) & 0xFF)
#define B1(x)               (uint8_t)(((x)>>8) & 0xFF)
#define B2(x)               (uint8_t)(((x)>>16) & 0xFF)
#define B3(x)               (uint8_t)(((x)>>24) & 0xFF)

void print_hex(const char *header, char *buffer, uint16_t len);
uint16_t gen_crc16(const uint8_t *data, uint16_t size);

#endif // _UTILITIES_H_