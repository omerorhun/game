#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <stdint.h>
#include <string>
#include "constants.h"

#define B0(x)               (uint8_t)((x) & 0xFF)
#define B1(x)               (uint8_t)(((x)>>8) & 0xFF)
#define B2(x)               (uint8_t)(((x)>>16) & 0xFF)
#define B3(x)               (uint8_t)(((x)>>24) & 0xFF)
#define B4(x)               (uint8_t)(((x)>>32) & 0xFF)
#define B5(x)               (uint8_t)(((x)>>40) & 0xFF)
#define B6(x)               (uint8_t)(((x)>>48) & 0xFF)
#define B7(x)               (uint8_t)(((x)>>56) & 0xFF)

void print_hex(const char *header, char *buffer, uint16_t len);
uint16_t gen_crc16(const uint8_t *data, uint16_t size);
std::string read_file(const char *file_name);

#endif // _UTILITIES_H_