#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <stdint.h>

void print_hex(char *header, char *buffer, uint16_t len);
uint16_t gen_crc16(const uint8_t *data, uint16_t size);

#endif // _UTILITIES_H_