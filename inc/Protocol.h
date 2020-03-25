#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <string>

#define B0(x)               ((x) & 0xFF)
#define B1(x)               (((x)>>8) & 0xFF)
#define B2(x)               (((x)>>16) & 0xFF)
#define B3(x)               (((x)>>24) & 0xFF)

#define RX_BUFFER_SIZE 1024

#define PROTOCOL_HEADER 0x01

#define LENGTH_LOW_POS 1
#define LENGTH_HIGH_POS 2
#define REQUEST_CODE_POS 3
#define DATA_START_POS 4

#define GET_LENGTH(p) (uint16_t)(((p[1] << 0) & 0xff) + \
                                ((p[2] << 8) & 0xff00))

#define GET_REQUEST_CODE(p) p[3]

uint16_t gen_crc16(const uint8_t *data, uint16_t size);
void print_hex(char *header, char *buffer, uint16_t len);

struct ProtocolCrcException : public std::exception
{
	const char * what () const throw ()
    {
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