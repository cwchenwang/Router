#ifndef __ARP__
#define __ARP__
#include <stdio.h>   
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <sys/ioctl.h>  
#include <net/if_arp.h>
#include <net/if.h>
#include <string.h> 

struct arpmac
{
    unsigned char * mac;
    unsigned int index;
};


//int arpGet(struct arpmac *dstmac, char *ifname, struct in_addr nexthopaddr);

//int srcMacGet(unsigned char*mac, char* ifname);
#endif 
