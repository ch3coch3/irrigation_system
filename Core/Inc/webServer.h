#include <inttypes.h>
#include <ip_arp_udp_tcp.h>

uint16_t _port;

void setup_server(uint8_t macAddress[], uint8_t ipAddress[], uint16_t port);
char* serviceRequest();		// returns a char* containing the requestString
								//        or NULL if no request to service
void print_text(char* text); 	// append the text to the response
void print_number(int value);  	// append the number to the response
void respond_single(); 			// write the final response
void respond_ack(); 			// write the final response
void respond_multiple(); 			// write the final response
void respond_end(); 			// write the final response
