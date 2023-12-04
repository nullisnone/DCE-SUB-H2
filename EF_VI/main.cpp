#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

#include "mc_client_efvi.h"

char  INTERFACE[8] = "ens1f0";

void* MarketDataThread_EFVI(void *arg)
{
    mc_client_efvi client_efvi(INTERFACE);
    printf("EFVI Init\n");
    client_efvi.init();
    printf("Low Latency looping\n");
    client_efvi.event_loop_low_latency();

    return 0;
}

int main(int argc, char **argv)
{
    //Creating MarketData Reading Thread
    int md_ret = 0;
    pthread_t md_id;

    md_ret = pthread_create(&md_id, NULL, MarketDataThread_EFVI, NULL);
    if(md_ret) {
        printf("Create MarketData Thread_EFVI Error!");
        return -1;
    }

    while(true)
	continue;

    return 0;
}
