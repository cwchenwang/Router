#include "checksum.h"

int check_sum(unsigned short *iphd, int len,
        unsigned short checksum) {

    int new_checksum = 0;
    int i;
    for(i = 0; i < len; i++) {
        new_checksum += iphd[i];
    }
    new_checksum = (new_checksum & 0xFFFF) + (new_checksum >> 16);
    if(new_checksum == 0xFFFF) return 1;
    else return 0;
}

unsigned short count_check_sum(unsigned short *iphd) {
    struct _iphdr *iphdr = (struct _iphdr*)iphd;
    iphdr -> checksum += 1;
    iphdr -> ttl -= 1;
}
