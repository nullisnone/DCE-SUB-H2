#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/fifo_tx.h>
#include <exanic/util.h>

#include "normal_udp.h"

#define ETHTYPE_OFFSET                  12
#define DOT1Q_ETHTYPE_OFFSET            16
#define IP_ID_OFFSET  			18
#define DST_IP_OFFSET 			30
#define PAYLOAD_OFFSET			42
#define IPV4_PACKET                     0x0008
#define DOT1Q_FRAME                     0x0081
#define DST_IP_FILTER 			0xefefef08	//239.239.239.8 is DST IP we want to read
#define BEST_QUOTE_MSG			1
#define DEPTH_UPDATE_MSG		2

#ifdef __cplusplus
extern "C" {
#endif
int packet_handler_full(char *input_data, char *output_data);
#ifdef __cplusplus
}
#endif

int main(int argc, char *argv[])
{
    char *nic_name;
    uint8_t  nic_port = 0;
    int keep_running = 1;

    int size = 0;
    int result = 0;
    uint16_t prev_ip_id = 0;
    char packet_buf[2048];
    char output_data[256];

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s exanic[0-N]:[0,1]\n", argv[0]);
        return -1;
    }

    if(strchr(argv[1], ':') != NULL)
    {
	nic_name = strtok(argv[1], ":");
	nic_port = atoi(strtok(NULL, ":"));

        printf("nic_name:%s, nic_port:%d \n", nic_name, nic_port);
    }	
    else
    {
        fprintf(stderr, "Usage: %s exanic[0-N]:[0,1]\n", argv[0]);
        return -1;
    }
		
    /* acquire exanic device handle */
    exanic_t *nic = exanic_acquire_handle(nic_name);
    if (!nic)
    {
        fprintf(stderr, "exanic_acquire_handle: %s\n", exanic_get_last_error());
        return -1;
    }

    /* fpga upload data to port1, acquire rx buffer to receive data */
    exanic_rx_t *rx = exanic_acquire_rx_buffer(nic, nic_port, 0);
    if (!rx)
    {
        fprintf(stderr, "exanic_acquire_rx_buffer: %s\n", exanic_get_last_error());
        return -1;
    }

    while (keep_running)
    {
        size = exanic_receive_frame(rx, packet_buf, sizeof(packet_buf), 0);

	if(size > 64)
	{

        	uint16_t *ethtype = (uint16_t *)(packet_buf + ETHTYPE_OFFSET);
        	uint16_t *dot1q_ethtype = (uint16_t *)(packet_buf + DOT1Q_ETHTYPE_OFFSET);
		int dot1q_offset = 0;

        	if(*ethtype == IPV4_PACKET)
                	dot1q_offset = 0;
        	else if(*ethtype==DOT1Q_FRAME && *dot1q_ethtype==IPV4_PACKET)
                	dot1q_offset = 4;
        	else
			continue;

        	if(ntohl(*((uint32_t *)(packet_buf + dot1q_offset + DST_IP_OFFSET))) != DST_IP_FILTER)
			continue;
		
		uint16_t last_ip_id = ntohs(*((uint16_t *)(packet_buf + dot1q_offset + IP_ID_OFFSET)));
		if((uint16_t)(prev_ip_id+0x0001) != last_ip_id)
		{
			printf("<WARN> IP_ID sequence number gap previous/latest: %d/%d \n", prev_ip_id, last_ip_id);
		}
		prev_ip_id = last_ip_id;

		memset(output_data, 0x00, 256);
		result = packet_handler_full(packet_buf, output_data);

		if(result == BEST_QUOTE_MSG)
		{
			normal_best_quote *best = (normal_best_quote *)output_data;
			if(best->bid_price>0xffff)  best->bid_price = 0;
			if(best->ask_price>0xffff)  best->ask_price = 0;
			if(best->last_price>0xffff) best->last_price = 0;

                	printf("%s, %8s, best, %d_%.1f x %.1f_%d, %d/%.2f/%d, %.1f/%d\n",
                		best->contract_id, best->gen_time,
                		best->bid_qty, best->bid_price, best->ask_price, best->ask_qty,
                		best->match_tot_qty, best->turnover, best->open_interest, 
				best->last_price, best->last_match_qty);
		}
		else if(result == DEPTH_UPDATE_MSG)
		{
			normal_depth_update *deep = (normal_depth_update *)output_data;
			if(deep->bid1_price>0xffff)  deep->bid1_price = 0;
			if(deep->bid2_price>0xffff)  deep->bid2_price = 0;
			if(deep->bid3_price>0xffff)  deep->bid3_price = 0;
			if(deep->bid4_price>0xffff)  deep->bid4_price = 0;
			if(deep->bid5_price>0xffff)  deep->bid5_price = 0;
			if(deep->ask1_price>0xffff)  deep->ask1_price = 0;
			if(deep->ask2_price>0xffff)  deep->ask2_price = 0;
			if(deep->ask3_price>0xffff)  deep->ask3_price = 0;
			if(deep->ask4_price>0xffff)  deep->ask4_price = 0;
			if(deep->ask5_price>0xffff)  deep->ask5_price = 0;

                	printf("%s, %8s, deep, %d_%.1f|%d_%.1f|%d_%.1f|%d_%.1f|%d_%.1f x %.1f_%d|%.1f_%d|%.1f_%d|%.1f_%d|%.1f_%d\n",
                                deep->contract_id, deep->gen_time,
                                deep->bid5_qty, deep->bid5_price,
                                deep->bid4_qty, deep->bid4_price,
                                deep->bid3_qty, deep->bid3_price,
                                deep->bid2_qty, deep->bid2_price,
                                deep->bid1_qty, deep->bid1_price,
                                deep->ask1_price, deep->ask1_qty,
                                deep->ask2_price, deep->ask2_qty,
                                deep->ask3_price, deep->ask3_qty,
                                deep->ask4_price, deep->ask4_qty,
                                deep->ask5_price, deep->ask5_qty);
		}
		else
		{
			printf("<ERROR> packet_decode_err: %d\n", size);
			continue;
		}
	}
    }

    return 0;
}
