/*
ETHER_28J60.cpp - Ethernet library
Copyright (c) 2010 Simon Monk.  All right reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "webServer.h"


/******************************************************************************
 * Definitions
 ******************************************************************************/
#define BUFFER_SIZE 1500
#define STR_BUFFER_SIZE 128
static uint8_t buf[BUFFER_SIZE+1];
static char strbuf[STR_BUFFER_SIZE+1];

uint16_t plen;


/******************************************************************************
 * Constructors
 ******************************************************************************/

/******************************************************************************
 * User API
 ******************************************************************************/

void setup_server(uint8_t macAddress[], uint8_t ipAddress[], uint16_t port)
{
	_port = port;
    enc28j60Init(macAddress);
    enc28j60clkout(2); // change clkout from 6.25MHz to 12.5MHz
    HAL_Delay(10);
	enc28j60PhyWrite(PHLCON,0x880);
	HAL_Delay(500);
	enc28j60PhyWrite(PHLCON,0x990);
	HAL_Delay(500);
	enc28j60PhyWrite(PHLCON,0x880);
	HAL_Delay(500);
	enc28j60PhyWrite(PHLCON,0x990);
	HAL_Delay(500);
    enc28j60PhyWrite(PHLCON,0x476);
    HAL_Delay(500);
    init_ip_arp_udp_tcp(macAddress, ipAddress, _port);
}


char* serviceRequest()
{
	uint16_t dat_p;
	plen = enc28j60PacketReceive(BUFFER_SIZE, buf);

	/*plen will ne unequal to zero if there is a valid packet (without crc error) */
	if(plen!=0)
	{
		// arp is broadcast if unknown but a host may also verify the mac address by sending it to a unicast address.
	    if (eth_type_is_arp_and_my_ip(buf, plen))
		{
	      make_arp_answer_from_request(buf);
	      return 0;
	    }
	    // check if ip packets are for us:
	    if (eth_type_is_ip_and_my_ip(buf, plen) == 0)
	 	{
	      return 0;
	    }
	    if (buf[IP_PROTO_P]==IP_PROTO_ICMP_V && buf[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V)
		{
	      make_echo_reply_from_request(buf, plen);
	      return 0;
	    }
	    // tcp port www start, compare only the lower byte
	    if (buf[IP_PROTO_P]==IP_PROTO_TCP_V&&buf[TCP_DST_PORT_H_P]==0&&buf[TCP_DST_PORT_L_P] == _port)
		{
	    	if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V)
			{
	         	make_tcp_synack_from_syn(buf); // make_tcp_synack_from_syn does already send the syn,ack
	         	return 0;
	      	}
	      	if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V)
			{
	        	init_len_info(buf); // init some data structures
	        	dat_p=get_tcp_data_pointer();
	        	if (dat_p==0)
				{ // we can possibly have no data, just ack:
	          		if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V)
					{
	            		make_tcp_ack_from_any(buf);
	          		}
	          		return 0;
	        	}
	        	//Verfica se n???o recebeu GET
	        	if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0)
				{
	          		// head, post and other methods for possible status codes see:
	            	// http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
	            	plen=fill_tcp_data(buf,0,"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>");
					plen=fill_tcp_data(buf,plen,"<h1>A</h1>");
					respond_single();
	        	}
	        	//Verifica se recebeu GET + request action
	 			if (strncmp("/",(char *)&(buf[dat_p+4]),1)==0) // was "/ " and 2
				{
					// Copy the request action before we overwrite it with the response
					int i = 0;
					while (buf[dat_p+5+i] != ' ' && i < STR_BUFFER_SIZE)
					{
						strbuf[i] = buf[dat_p+5+i];
						i++;
					}
					strbuf[i] = '\0';
					//plen=fill_tcp_data(buf,0,"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
					//plen=fill_tcp_data(buf,0,"HTTP/1.0 200 OK\r\nContent-Type: application/octet-stream\r\n\r\n");
					plen = 0;
					return (char*)strbuf;
	         	}
	      }
		}
	}
}


void print_text(char* text)
{
	int j = 0;
  	while (text[j])
	{
    	buf[TCP_CHECKSUM_L_P+3+plen]=text[j++];
    	plen++;
  	}
}

void print_number(int number)
{
  char tempString[9];
  sprintf(tempString, "%d",number);
  print_text(tempString);
}

void respond_single()
{
	make_tcp_ack_from_any(buf); // send ack for http get
	make_tcp_ack_with_data_single(buf,plen); // send data
	plen = 0;
}

void respond_multiple() 			// write the final response
{
	make_tcp_ack_with_data_multiple(buf,plen); // send data
	plen = 0;
}
void respond_end() 			// write the final response
{
	make_tcp_ack_with_data_end(buf,plen); // send data
	plen = 0;
}

void respond_ack() 			// write the final response
{
	make_tcp_ack_from_any(buf); // send ack for http get
}

