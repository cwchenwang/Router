#include "lookuproute.h"
#include <arpa/inet.h>

int insert_route(unsigned long ip4prefix, unsigned int prefixlen, char *ifname,
        unsigned int ifindex, unsigned long nexthopaddr) {
    struct route* p = route_table;
    while(p -> next != NULL) {
        p = p -> next;
    }
    //insert at current place
    p -> prefixlen = prefixlen;
    p -> ip4prefix.s_addr = ip4prefix;

    p -> nexthop = (struct nexthop*)malloc(sizeof(struct nexthop));
    memset(p -> nexthop, 0, sizeof(struct nexthop));
    p -> nexthop -> ifname = strdup(ifname);
    p -> nexthop -> ifindex = ifindex;
    p -> nexthop -> nexthopaddr.s_addr = nexthopaddr;

    //allocate next pointer
    struct route* to_insert = (struct route*)malloc(sizeof(struct route));
    memset(to_insert, 0, sizeof(struct route));
    p -> next = to_insert;
    printf("insert route for prefix: %s, ",  inet_ntoa(p -> ip4prefix));
    printf("prefixlen: %d, nextaddr: %s at %s, %d\n",
           prefixlen, inet_ntoa(p->nexthop->nexthopaddr), ifname, ifindex);
    return 1; //insert success
}

int lookup_route(struct in_addr dstaddr,
                 struct nextaddr *nexthopinfo) {
    printf("looking for route...\n");
	struct route* p = route_table;
	struct route* answer = NULL;
	unsigned int max_prefixlen = 0;

    while(p != NULL) {
        unsigned int mask = ((unsigned int)-1) >>
                                               (32 - p -> prefixlen); //endian problem
        if((p -> ip4prefix.s_addr & mask) == (dstaddr.s_addr & mask)) {
//            printf("%s\n",
//                    inet_ntoa(p->nexthop->nexthopaddr));
            printf("outside: from %s\n", inet_ntoa(p->ip4prefix));
            printf("to - ifname: %s, nexthop: %s, prefixlen: %d\n",
                    p->nexthop->ifname, inet_ntoa(p->nexthop->nexthopaddr), p->prefixlen);
            printf("max:%d cur:%d\n", max_prefixlen, p->prefixlen);
            if(p -> prefixlen > max_prefixlen) {
                max_prefixlen = p -> prefixlen;
                answer = p;
                printf("check route - ifname: %s, nexthop: %s, prefixlen: %d\n",
                       p->nexthop->ifname, inet_ntoa(p->nexthop->nexthopaddr), p->prefixlen);

            }
        }
        p = p -> next;
    }
    if(answer == NULL) {
        printf("route not found\n");
        return -1;
    } else {
        nexthopinfo -> ifname = answer -> nexthop -> ifname;
        nexthopinfo -> ipv4addr = answer -> nexthop -> nexthopaddr;
        nexthopinfo -> prefixl = answer -> prefixlen;
        printf("find route - ifname: %s, nexthop: %s, prefixlen: %d\n",
               nexthopinfo->ifname, inet_ntoa(nexthopinfo->ipv4addr), nexthopinfo->prefixl);
        return 1;
    }
}

int delete_route(struct in_addr dstaddr,
                 unsigned int prefixlen) {
    printf("deleting route...\n");
    struct route *last = NULL;
    struct route *now = route_table;
    unsigned int mask = (((unsigned int)-1) >> (32 - prefixlen));
    while (now != NULL) {
//        printf("%x %x\n", now->ip4prefix.s_addr, dstaddr.s_addr);
        if ((now->ip4prefix.s_addr & mask) == (dstaddr.s_addr & mask)) {
            if (last != NULL)
                last -> next = now -> next;
            if (route_table == now)
                route_table = now->next;
            printf("delete %s\n", inet_ntoa(now->ip4prefix));
            struct route* tep = now;
            now = now -> next;
            free(tep);
        } else {
            last = now;
            now = now -> next;
        }
    }
    return 0;
}