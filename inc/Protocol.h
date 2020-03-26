#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <string>

#define B0(x)               ((x) & 0xFF)
#define B1(x)               (((x)>>8) & 0xFF)
#define B2(x)               (((x)>>16) & 0xFF)
#define B3(x)               (((x)>>24) & 0xFF)

#define RX_BUFFER_SIZE 1024
#define TOKEN_SIZE 142

#define PROTOCOL_HEADER 0x01

#define PKT_LENGTH_LOW        1
#define PKT_LENGTH_HIGH       2
#define PKT_REQUEST_CODE      3
#define PKT_TOKEN             4
#define PKT_IN_DATA_START   (PKT_TOKEN + TOKEN_SIZE)
#define PKT_OUT_DATA_START    4

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
  
  bool set_header(uint8_t header);
  bool set_request_code(uint8_t code);
  bool add_data(std::string data);
  bool add_data(uint8_t *data, uint16_t len);
  bool set_crc();
  void send_packet(int sock);
  bool receive_packet(int sock);
  bool check_crc();
  bool check_token(std::string key);
  
  std::string get_data();
  uint8_t *get_buffer();
  uint8_t get_header();
  uint8_t get_request_code();
  uint16_t get_crc();
  uint16_t get_length();
  void free_buffer();
  
  private:
  uint8_t *_buffer;
  uint16_t _length; // whole buffer size
  uint16_t _data_length; // sum of data + requestcode(1 byte)
  void set_length();
};

#endif // _PROTOCOL_H_