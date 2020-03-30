#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <string>

#define RX_BUFFER_SIZE 1024
#define TOKEN_SIZE 130

#define PROTOCOL_HEADER 0x01

#define PKT_HEADER            0
#define PKT_LENGTH_LOW        1
#define PKT_LENGTH_HIGH       2
#define PKT_REQUEST_CODE      3
#define PKT_DATA_START        4
#define PKT_TOKEN             PKT_DATA_START

#define GET_LENGTH(p) (uint16_t)(((p[PKT_LENGTH_LOW] << 0) & 0xff) + \
                                ((p[PKT_LENGTH_HIGH] << 8) & 0xff00))

#define GET_REQUEST_CODE(p) p[3]

uint16_t gen_crc16(const uint8_t *data, uint16_t size);
void print_hex(char *header, char *buffer, uint16_t len);

struct ProtocolCrcException : public std::exception {
	const char * what () const throw () {
    	return "Crc values doesn't match!";
    }
};

class Protocol {
  public:
  Protocol();
  Protocol(uint8_t *buffer);
  ~Protocol();
  
  uint8_t get_header();
  uint8_t get_request_code();
  uint8_t get_ack(); // definition will be added
  uint16_t get_crc();
  uint16_t get_length();
  uint8_t *get_buffer();
  std::string get_data();
  
  bool set_header(uint8_t header);
  bool set_request_code(uint8_t code);
  bool set_ack(uint8_t ack);
  bool add_data(std::string data);
  bool add_data(uint8_t *data, uint16_t len);
  bool set_crc();
  
  bool check_crc();
  bool check_token(std::string key, int *p_uid);
  
  void send_packet(int sock);
  bool receive_packet(int sock);
  
  void free_buffer();
  
  private:
  uint8_t *_buffer;
  uint16_t _length; // whole buffer size
  uint16_t _data_length; // sum of data + requestcode(1 byte)
  void set_length();
};

#endif // _PROTOCOL_H_