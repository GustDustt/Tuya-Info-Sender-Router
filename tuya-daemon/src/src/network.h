#ifndef NETWORK_H
#define NETWORK_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE  
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>




typedef struct {
    char name[IFNAMSIZ];
    char ip[INET_ADDRSTRLEN];
    char netmask[INET_ADDRSTRLEN];
    unsigned long long rx_bytes;
    unsigned long long tx_bytes;
} NetworkData;


int get_net_bytes(const char *iface, unsigned long long *rx, unsigned long long *tx);
int get_network_data(NetworkData *data_arr, int max_interfaces);

#endif