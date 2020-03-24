#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "Protocol.h"

using namespace std;

uint16_t gen_crc16(const uint8_t *data, uint16_t size);
void print_hex(char *buffer, uint16_t len);

Protocol::Protocol() {
    _length = 0;
    _data_length = 0;
    _buffer = NULL;
}

Protocol::Protocol(uint8_t *buffer) {
    // get buffer
    _buffer = buffer;
    
    // get length;
    _data_length = GET_LENGTH(_buffer);
    _length = _data_length + 5; // 1(header) + 2(length) + 2(crc)
    
    // check crc
    uint16_t crc = gen_crc16(&_buffer[REQUEST_CODE_POS], _data_length);
    if (crc != get_crc()) {
        throw ProtocolCrcException();
        return;
    }
}

Protocol::~Protocol() {
    if (_buffer != NULL)
        free(_buffer);
}

// TODO: add error codes
bool Protocol::receive_packet(int sock) {
    _buffer = (uint8_t *)malloc(sizeof(uint8_t) * RX_BUFFER_SIZE);
    if (_buffer == NULL)
        return false;
    
    if (recv(sock, _buffer, RX_BUFFER_SIZE, 0) == -1) {
        return false;
    }
    
    // get length;
    _data_length = GET_LENGTH(_buffer);
    _length = _data_length + 5; // 1(header) + 2(length) + 2(crc)
    return true;
}

bool Protocol::check_crc() {
    if (_buffer == NULL)
        return 0;
    
    uint16_t crc = gen_crc16(&_buffer[REQUEST_CODE_POS], _data_length);
    if (crc != get_crc()) {
        return false;
    }
    
    return true;
}

uint16_t Protocol::get_crc() {
    if (_buffer == NULL)
        return 0;
    
    return (uint16_t)(((_buffer[_length - 2] << 0) & 0xff) + \
                                ((_buffer[_length - 1] << 8) & 0xff00));
}

uint8_t Protocol::get_header() {
    if (_buffer == NULL)
        return 0;
    
    return _buffer[0];
}

uint16_t Protocol::get_length() {
    return _data_length - 1; // only data (-1 for req code)
}

void Protocol::free_buffer() {
    free(_buffer);
}

void Protocol::send_packet() {
    if (_buffer == NULL)
        return;
    
    print_hex((char *)_buffer, _length);
    free(_buffer);
}

bool Protocol::set_header(uint8_t header) {
    if (_buffer != NULL)
        return false;
    
    _buffer = (uint8_t *)malloc(sizeof(uint8_t) * 3);
    
    if (_buffer == NULL)
        return false;
    
    _length = 3;
    memset(_buffer, 0, _length);
    _buffer[0] = header;
    
    return true;
}

bool Protocol::set_request_code(uint8_t code) {
    if (_buffer == NULL)
        return false;
    
    uint8_t *temp = (uint8_t *)realloc(_buffer, _length + 1);
    
    if (temp == NULL)
        return false;
    
    _buffer = temp;
    _buffer[3] = code;
    
    _data_length++;
    _length++;
    
    return true;
}

uint8_t *Protocol::get_buffer() {
    return _buffer;
}

string Protocol::get_data() {
    if (_buffer == NULL)
        return string("");
    
    return string((char *)&_buffer[DATA_START_POS], _data_length - 1);
}

bool Protocol::add_data(string data) {
    if (_buffer == NULL)
        return false;
    
    uint8_t *tmp = (uint8_t *)realloc(_buffer, sizeof(char) * (_length + data.size()));
    if (tmp == NULL)
        return false;
    
    _buffer = tmp;
    uint8_t *ptr = &_buffer[_length];
    
    // set zero new allocated memory
    for (uint16_t i = 0; i < data.size(); i++) {
        *(ptr + i) = (uint8_t)NULL;
    }
    
    memccpy((char *)ptr, data.c_str(), sizeof(char), data.size());
    _length += data.size();
    _data_length += data.size();
    
    return true;
}

bool Protocol::add_data(uint8_t *data, uint16_t len) {
    if (_buffer == NULL)
        return false;
    
    uint8_t *tmp = (uint8_t *)realloc(_buffer, sizeof(char) * (_length + len));
    if (tmp == NULL)
        return false;
    
    _buffer = tmp;
    for (uint16_t i = 0; i < len; i++) {
        _buffer[_length + i] = data[i];
    }
    _data_length += len;
    _length += len;
}

bool Protocol::set_crc() {
    if (_buffer == NULL)
        return false;
    
    uint16_t crc16 = gen_crc16(&_buffer[REQUEST_CODE_POS], _data_length);
    uint8_t *tmp = (uint8_t *)realloc(_buffer, sizeof(char) * (_length + 2));

    if (tmp == NULL)
    return false;

    _buffer = tmp;
    uint8_t *ptr = &_buffer[_length];

    // set zero new allocated memory
    for (uint16_t i = 0; i < 2; i++) {
    *(ptr + i) = (uint8_t)NULL;
    }

    _buffer[_length++] = B0(crc16); // crc low
    _buffer[_length++] = B1(crc16); // crc high
    set_length();

    return true; 
}

void Protocol::set_length() {
    if (_buffer == NULL)
        return;
    
    _buffer[LENGTH_LOW_POS] = B0(_data_length);
    _buffer[LENGTH_HIGH_POS] = B1(_data_length);
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

void print_hex(char *buffer, uint16_t len) {
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
            for (uint16_t j = 0; j < rem; j++) printf("00 ");
            for (uint16_t j = i - 0x10 + rem; j < i; j++) {
                if (isprint((int)buffer[j])) putchar(buffer[j]);
                else putchar('.');
            }
            for (uint16_t j = 0; j < rem; j++) putchar('.');
            putchar('\n');
            putchar('\n');
        }
    }
}