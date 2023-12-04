
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "normal_udp.h"

#define EXAMPLE_PORT  			888
#define EXAMPLE_GROUP 			"239.239.239.8"
#define BEST_QUOTE_MSG                  1
#define DEPTH_UPDATE_MSG                2

#ifdef __cplusplus
extern "C" {
#endif

int packet_handler_payload(char *input_data, char *output_data, int payload_size);

#ifdef __cplusplus
}
#endif

int main(int argc, char *argv[])
{
   struct sockaddr_in addr;
   int addrlen, sock, cnt;
   struct ip_mreq mreq;
   char message[2048];
   char output_data[256];
   int result = 0;

   /* set up socket */
   sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) {
     perror("socket");
     return -1;
   }

   memset(&addr, 0x00, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   addr.sin_port = htons(EXAMPLE_PORT);
   addrlen = sizeof(addr);

      /* receive */
      if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {        
         perror("bind");
	 return -1; 
      }    
      mreq.imr_multiaddr.s_addr = inet_addr(EXAMPLE_GROUP);         
      mreq.imr_interface.s_addr = htonl(INADDR_ANY);         
      if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
	 perror("setsockopt mreq");
	 return -1;
      }         

      while (1) {
 	 	cnt = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *) &addr, (socklen_t *)&addrlen);
	 	if (cnt < 0) {
	    		perror("recvfrom");
	    		return -1;
	 	}
		else if (cnt<20 && cnt>144) {
	    		printf("<ERR> abnormal payload size:%d\n", cnt);
			break;
		}	

      		memset(output_data, 0x00, 256);
      		result = packet_handler_payload(message, output_data, cnt);

      		if(result == BEST_QUOTE_MSG)
      		{
      		        normal_best_quote *best = (normal_best_quote *)output_data;
      		        if(best->bid_price>0xffff)  best->bid_price = 0;
      		        if(best->ask_price>0xffff)  best->ask_price = 0;
      		        if(best->last_price>0xffff) best->last_price = 0;

      		        printf("%s, %s, best, %d_%.1f x %.1f_%d, %d/%.2f/%d, %.1f/%d\n",
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

      		        printf("%s, %s, deep, %d_%.1f|%d_%.1f|%d_%.1f|%d_%.1f|%d_%.1f x %.1f_%d|%.1f_%d|%.1f_%d|%.1f_%d|%.1f_%d\n",
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
      		        printf("<ERROR> packet_decode_err: %d\n", cnt);
      		        continue;
      		}
      }

      return 0;
}
