#include "arpfind.h"

int arpGet(unsigned char *mac, char *ifname,
        struct in_addr nexthopaddr) {
    printf("in arpget..\n");
    struct arpreq arp_req;
    struct sockaddr_in *sin;

    sin = (struct sockaddr_in *)&(arp_req.arp_pa);

    memset(&arp_req, 0, sizeof(arp_req));
    sin -> sin_family = AF_INET;
    sin -> sin_addr = nexthopaddr;

    strcpy(arp_req.arp_dev, ifname);

    int sfd = socket(AF_INET, SOCK_DGRAM, 0);

    int ret = 0;
    ret = ioctl(sfd, SIOCGARP, &arp_req);
    if (ret < 0) {
        printf("Get ARP entry failed : %s\n");
        return -1;
    }

    if (arp_req.arp_flags & ATF_COM) {
        memcpy(mac, (unsigned char*)arp_req.arp_ha.sa_data, 6);
        printf("dst mac addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
        printf("dst mac addr not in the ARP cache.\n");
    }
    close(sfd);
    return 0;  
}

int macGet(unsigned char *mac, char *ifname) {
    printf("in mac get...\n");
    if(ifname == NULL) {
        printf("error! find mac addr for null interface\n");
        return -1;
    }
    struct ifreq ifr;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == 0) {
//        memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
        memcpy(mac, (unsigned char*)ifr.ifr_hwaddr.sa_data, 6);
        printf("mac address for %s: %02x:%02x:%02x:%02x:%02x:%02x\n",
              ifname, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
        printf("mac address for %s not found\n", ifname);
        return -1;
    }
    close(sockfd);

    return 0;
}
                                                                                                            
                                                                                                              
