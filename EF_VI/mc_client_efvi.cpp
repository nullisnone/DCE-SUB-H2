#include "mc_client_efvi.h"
#include "utils.h"
#include "normal_udp.h"

int packet_handler_full(char *input_data, char *output_data);

mc_client_efvi::mc_client_efvi(char *input_interface)
{
  cfg_max_fill = -1;
  stop = 1;
  prev_ip_id = 0;
  memset(packet_buf, 0x00, 2048);
  memset(output_data, 0x00, 256);

  memcpy(main_interface, input_interface, 8);
  printf("main_interface:%s\n", main_interface);
}

mc_client_efvi::~mc_client_efvi()
{
}

struct pkt_buf* mc_client_efvi::pkt_buf_from_id(struct resources* res, int pkt_buf_i)
{
  assert((unsigned) pkt_buf_i < (unsigned) res->pkt_bufs_n);
  return (struct pkt_buf*) ((char*) res->pkt_bufs + (size_t) pkt_buf_i * PKT_BUF_SIZE);
}

void mc_client_efvi::pkt_buf_free(struct resources* res, struct pkt_buf* pkt_buf)
{
  pkt_buf->next = res->free_pkt_bufs;
  res->free_pkt_bufs = pkt_buf;
  ++(res->free_pkt_bufs_n);
}

void mc_client_efvi::handle_pkt(const void* pv, int len)
{
  unsigned char* p = (unsigned char*) pv;
  uint16_t *eth_proto;
  uint8_t  *ip_header_len; 
  uint8_t  *ip_proto;
  uint32_t *dst_ip;

  eth_proto = (uint16_t *)(p + 6);
  ip_header_len = (uint8_t *)(p + 8);
  ip_proto = (uint8_t *)(p + 17);
  dst_ip = (uint32_t *)(p + 24);
  if(len<60 ||*eth_proto!=0x0008 || *ip_header_len!=0x45 || *ip_proto!=0x11 || *dst_ip!=0x08efefef)
  {
  	//printf("NOT_IP: eth_proto:%04x, ip_header_len:%02x\n", *eth_proto, *ip_header_len);
	return;
  }

  int size = len - 36;
  char *packet_buf = (char *)(p - 6);

                uint16_t *ethtype = (uint16_t *)(packet_buf + ETHTYPE_OFFSET);
                uint16_t *dot1q_ethtype = (uint16_t *)(packet_buf + DOT1Q_ETHTYPE_OFFSET);
                int dot1q_offset = 0;

                if(*ethtype == IPV4_PACKET)
                        dot1q_offset = 0;
                else if(*ethtype==DOT1Q_FRAME && *dot1q_ethtype==IPV4_PACKET)
                        dot1q_offset = 4;
                else
                        return;

                if(ntohl(*((uint32_t *)(packet_buf + dot1q_offset + DST_IP_OFFSET))) != DST_IP_FILTER)
                        return;

                uint16_t last_ip_id = ntohs(*((uint16_t *)(packet_buf + dot1q_offset + IP_ID_OFFSET)));
                if((uint16_t)(prev_ip_id+0x0001) != last_ip_id)
                {
                        printf("<WARN> IP_ID sequence number gap previous/latest: %d/%d \n", prev_ip_id, last_ip_id);
                }
                prev_ip_id = last_ip_id;

                memset(output_data, 0x00, 256);
                int result = packet_handler_full(packet_buf, output_data);

                if(result == BEST_QUOTE_MSG)
                {
                        normal_best_quote *best = (normal_best_quote *)output_data;
                        if(best->bid_price>0xffff)  best->bid_price = 0;
                        if(best->ask_price>0xffff)  best->ask_price = 0;
                        if(best->last_price>0xffff) best->last_price = 0;

                        printf("%5d, %s, %12s, best, %d_%.1f x %.1f_%d, %d/%d, %.1f/%d\n",
                                last_ip_id, best->contract_id, best->gen_time,
                                best->bid_qty, best->bid_price, best->ask_price, best->ask_qty,
                                best->match_tot_qty, best->open_interest,
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

                        printf("%5d, %s, %12s, deep, %d_%.1f|%d_%.1f|%d_%.1f|%d_%.1f|%d_%.1f x %.1f_%d|%.1f_%d|%.1f_%d|%.1f_%d|%.1f_%d\n",
                                last_ip_id, deep->contract_id, deep->gen_time,
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
                }
}

void mc_client_efvi::handle_rx_core(struct resources* res, const void* dma_ptr, const void* rx_ptr, int len)
{
  /* Do something useful with packet contents here! */
  handle_pkt(rx_ptr, len);

  res->n_rx_pkts += 1;
  res->n_rx_bytes += len;
}

void mc_client_efvi::handle_rx(struct resources* res, int pkt_buf_i, int len)
{
  struct pkt_buf* pkt_buf;

  LOGV("PKT: received pkt=%d len=%d\n", pkt_buf_i, len);

  pkt_buf = pkt_buf_from_id(res, pkt_buf_i);
  handle_rx_core(res, (char*)pkt_buf + RX_DMA_OFF, pkt_buf->rx_ptr, len);
  pkt_buf_free(res, pkt_buf);
}

void mc_client_efvi::handle_rx_discard(struct resources* res, int pkt_buf_i, int len, int discard_type)
{
  struct pkt_buf* pkt_buf;

  LOGE("ERROR: discard type=%d\n", discard_type);

  pkt_buf = pkt_buf_from_id(res, pkt_buf_i);
  pkt_buf_free(res, pkt_buf);
}


bool mc_client_efvi::refill_rx_ring(struct resources* res)
{
  struct pkt_buf* pkt_buf;
  int i;

  if( ef_vi_receive_fill_level(&res->vi) > res->refill_level ||
      res->free_pkt_bufs_n < REFILL_BATCH_SIZE )
    return false;	/* means RX_RING is empty and ready to receive, or there is no free buffer space left to refill RX_RING */

  do {
    for( i = 0; i < REFILL_BATCH_SIZE; ++i ) {
      pkt_buf = res->free_pkt_bufs;
      res->free_pkt_bufs = res->free_pkt_bufs->next;
      --(res->free_pkt_bufs_n);
      ef_vi_receive_init(&res->vi, pkt_buf->efvi_addr + RX_DMA_OFF, pkt_buf->id);
    }
  } while( ef_vi_receive_fill_level(&res->vi) < res->refill_min &&
           res->free_pkt_bufs_n >= REFILL_BATCH_SIZE );

  ef_vi_receive_push(&res->vi);

  return true;
}


int mc_client_efvi::poll_evq(struct resources* res)
{
  ef_event evs[EV_POLL_BATCH_SIZE];
  int i;

  int n_ev = ef_eventq_poll(&res->vi, evs, EV_POLL_BATCH_SIZE);

  for( i = 0; i < n_ev; ++i ) {
    switch( EF_EVENT_TYPE(evs[i]) ) {
    case EF_EVENT_TYPE_RX:
      /* This code does not handle scattered jumbos. */
      TEST( EF_EVENT_RX_SOP(evs[i]) && ! EF_EVENT_RX_CONT(evs[i]) );
      handle_rx(res, EF_EVENT_RX_RQ_ID(evs[i]), EF_EVENT_RX_BYTES(evs[i]) - res->rx_prefix_len);
      break;
    case EF_EVENT_TYPE_RX_DISCARD:
      handle_rx_discard(res, EF_EVENT_RX_DISCARD_RQ_ID(evs[i]), EF_EVENT_RX_DISCARD_BYTES(evs[i]) - res->rx_prefix_len, EF_EVENT_RX_DISCARD_TYPE(evs[i]));
      break;
    default:
      LOGE("ERROR: unexpected event type=%d\n", (int) EF_EVENT_TYPE(evs[i]));
      break;
    }
  }

  return n_ev;
}

void mc_client_efvi::event_loop_low_latency()
{
  while( stop ) {
    refill_rx_ring(res);
    poll_evq(res);
  }
}

int mc_client_efvi::init()
{
  TEST((res = (struct resources*)calloc(1, sizeof(*res))) != NULL);

  /* Open driver and allocate a VI. */
  TRY(ef_driver_open(&res->dh));
  TRY(ef_pd_alloc_by_name(&res->pd, res->dh, main_interface, EF_PD_DEFAULT));
  TRY(ef_vi_alloc_from_pd(&res->vi, res->dh, &res->pd, res->dh, -1, -1, 0, NULL, -1, EF_VI_FLAGS_DEFAULT));

  res->rx_prefix_len = ef_vi_receive_prefix_len(&res->vi);
  cfg_max_fill = ef_vi_receive_capacity(&res->vi) - 15;

  LOGI("rxq_size=%d\n", ef_vi_receive_capacity(&res->vi));
  LOGI("max_fill=%d\n", cfg_max_fill);
  LOGI("evq_size=%d\n", ef_eventq_capacity(&res->vi));
  LOGI("rx_prefix_len=%d\n", res->rx_prefix_len);

  /* Allocate memory for DMA transfers. Try mmap() with MAP_HUGETLB to get huge
   * pages. If that fails, fall back to posix_memalign() and hope that we do
   * get them. */
  res->pkt_bufs_n = cfg_max_fill;
  size_t alloc_size = res->pkt_bufs_n * PKT_BUF_SIZE;

  /* huge_page_size is 2097152 for this system, so it is double of alloc_size */
  printf("alloc_size:%d, huge_page_size:%d\n", (int)alloc_size, (int)huge_page_size);

  alloc_size = ROUND_UP(alloc_size, huge_page_size);
  res->pkt_bufs = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

  if( res->pkt_bufs == MAP_FAILED ) {
    LOGW("mmap() failed. Are huge pages configured?\n");

    /* Allocate huge-page-aligned memory to give best chance of allocating
     * transparent huge-pages.
     */
    TEST(posix_memalign(&res->pkt_bufs, huge_page_size, alloc_size) == 0);
  }

  int i;
  for( i = 0; i < res->pkt_bufs_n; ++i ) {
    struct pkt_buf* pkt_buf = pkt_buf_from_id(res, i);
    pkt_buf->rx_ptr = (char*) pkt_buf + RX_DMA_OFF + res->rx_prefix_len;
    pkt_buf->id = i;
    pkt_buf_free(res, pkt_buf);
  }

  /* Register the memory so that the adapter can access it. */
  TRY(ef_memreg_alloc(&res->memreg, res->dh, &res->pd, res->dh, res->pkt_bufs, alloc_size));
  for( i = 0; i < res->pkt_bufs_n; ++i ) {
    struct pkt_buf* pkt_buf = pkt_buf_from_id(res, i);
    pkt_buf->efvi_addr = ef_memreg_dma_addr(&res->memreg, i * PKT_BUF_SIZE);
  }

  /* Fill the RX ring. */
  res->refill_level = cfg_max_fill - REFILL_BATCH_SIZE;
  res->refill_min = cfg_max_fill / 2;
  while( ef_vi_receive_fill_level(&res->vi) <= res->refill_level )
    refill_rx_ring(res);

  /* Add filters so that adapter will send packets to this VI. */
  ef_filter_spec filter_spec;
  if( filter_parse(&filter_spec, "sniff", NULL) != 0 ) {
      LOGE("ERROR: Bad filter spec\n");
      exit(1);
  }
  TRY(ef_vi_filter_add(&res->vi, res->dh, &filter_spec, NULL));
 
  //pthread_mutex_init(&printf_mutex, NULL);

  //get current time to store into global_value
  time_t pre_time_min = 0;
  struct tm *pre_time_min_p;
  time(&pre_time_min);
  pre_time_min_p = localtime(&pre_time_min);
  
  printf("Ready to receive, time: %04d:%02d:%02d_%02d:%02d:%02d \n",
        pre_time_min_p->tm_year + 1900,
        pre_time_min_p->tm_mon,
        pre_time_min_p->tm_mday,
        pre_time_min_p->tm_hour,
        pre_time_min_p->tm_min,
        pre_time_min_p->tm_sec);

  return 0;
}
