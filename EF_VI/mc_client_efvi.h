#ifndef MC_CLIENT_EFVI_H_
#define MC_CLIENT_EFVI_H_

#include <etherfabric/vi.h>
#include <etherfabric/pd.h>
#include <etherfabric/memreg.h>
#include <etherfabric/efct_vi.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>

#include <poll.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <string>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>

typedef unsigned char uint8;   //8位无符号整数
typedef unsigned short uint16; //16位无符号整数
typedef unsigned int uint32;   //32位无符号整数

#define EV_POLL_BATCH_SIZE   16
#define REFILL_BATCH_SIZE    16
#define UDP_BASE_OFFSET      42
#define TCP_BASE_OFFSET      54

#define ETHTYPE_OFFSET                  12
#define DOT1Q_ETHTYPE_OFFSET            16
#define IP_ID_OFFSET                    18
#define DST_IP_OFFSET                   30
#define PAYLOAD_OFFSET                  42
#define IPV4_PACKET                     0x0008
#define DOT1Q_FRAME                     0x0081
#define DST_IP_FILTER                   0xefefef08      //239.239.239.8 is DST IP we want to read
#define BEST_QUOTE_MSG                  1
#define DEPTH_UPDATE_MSG                2

/* Hardware delivers at most ef_vi_receive_buffer_len() bytes to each
 * buffer (default 1792), and for best performance buffers should be
 * aligned on a 64-byte boundary.  Also, RX DMA will not cross a 4K
 * boundary.  The I/O address space may be discontiguous at 4K boundaries.
 * So easiest thing to do is to make buffers always be 2K in size.
 */
#define PKT_BUF_SIZE         2048

/* Align address where data is delivered onto EF_VI_DMA_ALIGN(64bytes) boundary,
 * because that gives best performance. sizeof(struct pkt_buf) is 32bytes, RX_DMA_OFF is 64
 */
#define RX_DMA_OFF           ROUND_UP(sizeof(struct pkt_buf), EF_VI_DMA_ALIGN)

//define number of order_flow entry
#define ORDER_FLOW_MAX  256


struct pkt_buf {
  /* I/O address corresponding to the start of this pkt_buf struct */
  ef_addr            efvi_addr;

  /* pointer to where received packets start */
  void*              rx_ptr;

  int                id;
  struct pkt_buf*    next;
};

struct resources {
  /* handle for accessing the driver */
  ef_driver_handle   dh;

  /* protection domain */
  struct ef_pd       pd;

  /* virtual interface (rxq + txq) */
  struct ef_vi       vi;
  int                rx_prefix_len;
  int                pktlen_offset;
  int                refill_level;
  int                refill_min;
  unsigned           batch_loops;

  /* registered memory for DMA */
  void*              pkt_bufs;
  int                pkt_bufs_n;
  struct ef_memreg   memreg;

  /* pool of free packet buffers (LIFO to minimise working set) */
  struct pkt_buf*    free_pkt_bufs;
  int                free_pkt_bufs_n;

  /* statistics */
  uint64_t           n_rx_pkts;
  uint64_t           n_rx_bytes;
  uint64_t           n_ht_events;
};


/**
 * @brief 组播接收客户端
 */
class mc_client_efvi
{
public:
    /**
     * @brief 构造函数
     *
     * @param mc_ip 组播组IP
     * @param mc_port 组播组端口号
     */
    mc_client_efvi(char *input_interface);

    /**
     * @brief 析构函数
     */
    ~mc_client_efvi();

    int init();
    void event_loop_low_latency();


/****** 报文处理函数 ******/
private:

    int poll_evq(struct resources* res);
    bool refill_rx_ring(struct resources* res);
    void pkt_buf_free(struct resources* res, struct pkt_buf* pkt_buf);
    struct pkt_buf* pkt_buf_from_id(struct resources* res, int pkt_buf_i);
    void handle_rx(struct resources* res, int pkt_buf_i, int len);
    void handle_rx_discard(struct resources* res, int pkt_buf_i, int len, int discard_type);
    void handle_rx_core(struct resources* res, const void* dma_ptr, const void* rx_ptr, int len);
    void handle_pkt(const void* pv, int len);

/****** 工具函数 ******/
private:
    char  	main_interface[10];	//record the interface which read marketdata
    int 	cfg_max_fill;
    int 	stop;

    uint16_t prev_ip_id;
    char packet_buf[2048];
    char output_data[256];

    struct resources* res;
    
};

#endif
