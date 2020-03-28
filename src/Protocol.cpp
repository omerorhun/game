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
#include "Jwt.h"
#include "utilities.h"

using namespace std;

Protocol::Protocol() {
    _length = 0;
    _data_length = 0;
    _buffer = NULL;
    _state = PKT_ST_EMPTY;
}

Protocol::Protocol(uint8_t *buffer) {
    // get buffer
    _buffer = buffer;
    
    // get length;
    _data_length = GET_LENGTH(_buffer);
    _length = _data_length + 5; // 1(header) + 2(length) + 2(crc)
    
    // check crc
    uint16_t crc = gen_crc16(&_buffer[PKT_REQUEST_CODE], _data_length);
    if (crc != get_crc()) {
        throw ProtocolCrcException();
        return;
    }
}

Protocol::~Protocol() {
    if (_buffer != NULL) {
        free(_buffer);
        _buffer = NULL;
    }   
}

// TODO: add error codes
bool Protocol::receive_packet(int sock) {
    _buffer = (uint8_t *)malloc(sizeof(uint8_t) * RX_BUFFER_SIZE);
    if (_buffer == NULL)
        return false;
    
    memset(_buffer, 0, RX_BUFFER_SIZE);
    
    if (recv(sock, _buffer, RX_BUFFER_SIZE, 0) == -1) {
        return false;
    }
    
    if (strlen((char *)_buffer) == 0)
        return false;
    
    // get length;
    _data_length = GET_LENGTH(_buffer);
    _length = _data_length + 5; // 1(header) + 2(length) + 2(crc)
    
    const char header[] = "rx packet";
    print_hex(header, (char *)_buffer, _length);
    
    return true;
}

bool Protocol::check_crc() {
    if (_buffer == NULL)
        return 0;
    
    uint16_t crc = gen_crc16(&_buffer[PKT_REQUEST_CODE], _data_length);
    if (crc != get_crc())
        return false;
    
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
    if (_buffer != NULL) {
        free(_buffer);
        _buffer = NULL;
    }
}

void Protocol::send_packet(int sock) {
    if (_buffer == NULL)
        return;
    
    const char header[] = "tx packet";
    print_hex(header, (char *)_buffer, _length);
    send(sock, _buffer, _length, 0);
}

bool Protocol::set_header(uint8_t header) {
    if (_buffer != NULL)
        return false;
    
    _buffer = (uint8_t *)malloc(sizeof(uint8_t) * 4); // 5: hdr+len+code
    if (_buffer == NULL)
        return false;
    
    memset(_buffer, 0, _length);
    _buffer[PKT_HEADER] = header;
    _length = 4;
    _data_length++; // empty request code, will be filled by set_request_code()
    
    return true;
}

#if 0
bool Protocol::set_ack(uint8_t ack) {
    if (_buffer == NULL)
        return false;
    
    _buffer[PKT_ACK] = ack;
    _data_length++;
    _length++;
    
    return true;
}
#endif

uint8_t Protocol::get_request_code() {
    return _buffer[PKT_REQUEST_CODE];
}

bool Protocol::set_request_code(uint8_t code) {
    if (_buffer == NULL)
        return false;
    
    _buffer[PKT_REQUEST_CODE] = code;
    
    return true;
}

uint8_t *Protocol::get_buffer() {
    return _buffer;
}

bool Protocol::check_token(string key) {
    string indata((char *)&_buffer[PKT_TOKEN]);
    print_hex((const char *)"constrctr token", (char *)indata.c_str(), indata.size());
    Jwt tkn(indata, key);
    
    if (!tkn.verify())
        return false;
    
    return true;
}

string Protocol::get_data() {
    if (_buffer == NULL)
        return string("");
    
    return string((char *)&_buffer[PKT_DATA_START], _data_length - 1);
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
    
    return true;
}

bool Protocol::set_crc() {
    if (_buffer == NULL)
        return false;
    
    uint16_t crc16 = gen_crc16(&_buffer[PKT_REQUEST_CODE], _data_length);
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
    
    _buffer[PKT_LENGTH_LOW] = B0(_data_length);
    _buffer[PKT_LENGTH_HIGH] = B1(_data_length);
}
