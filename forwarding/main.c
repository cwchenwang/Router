//#include "analyseip.h"
#include "checksum.h"
#include "lookuproute.h"
#include "arpfind.h"
#include "sendetherip.h"
#include "recvroute.h"
#include <pthread.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/ether.h>

#define IP_HEADER_LEN sizeof(struct ip)
#define ETHER_HEADER_LEN sizeof(struct ether_header)

void *thr_fn(void* arg) {
	int st = 0;
	struct selfroute *selfrt;
	selfrt = (struct selfroute*)malloc(sizeof(struct selfroute));
	memset(selfrt, 0, sizeof(struct selfroute));

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //set ip transmission
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(800);

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	int connfd;

	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	listen(listenfd, 10); //the second params is the line length
	printf("listen at %d\n", listenfd);

	//add-24 delete-25
	while(1) {
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
		printf("connfd=%d\n", connfd);
		int n = recv(connfd, selfrt, sizeof(struct route), 0);
		if(n < 0) {
			printf("receive error\n");
		} else {
			printf("receive a route %d\n", n);
		}

		if(selfrt -> cmdnum == 24) { //insert
			char idx_name[10];
			if_indextoname(selfrt -> ifindex, idx_name);
			printf("%s\n", inet_ntoa(selfrt->nexthop));
			insert_route(selfrt -> prefix.s_addr, selfrt -> prefixlen,
					idx_name, selfrt -> ifindex, selfrt -> nexthop.s_addr);

			struct route* p = route_table;

			while(p != NULL && p -> ip4prefix.s_addr != 0) {
					//printf("%s\n",
					//	   inet_ntoa(p->nexthop->nexthopaddr));
				printf("from %s ", inet_ntoa(p->ip4prefix));
				printf("to - ifname: %s, nexthop: %s, prefixlen: %d\n",
						   p->nexthop->ifname, inet_ntoa(p->nexthop->nexthopaddr), p->prefixlen);
				p = p -> next;
			}
		} else if(selfrt -> cmdnum == 25) { //delete
			delete_route(selfrt -> prefix, selfrt -> prefixlen);
		}
		close(connfd);
	}
	close(listenfd);
}

int main() {
	char skbuf[1530];
	char data[1480];
	int recvfd, datalen;
	int recvlen;		
	struct ip *ip_recvpkt;
	pthread_t tid;
	ip_recvpkt = (struct ip*)malloc(sizeof(struct ip));

	//创建raw socket套接字
	if((recvfd=socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP)))==-1) {
		printf("recvfd() error\n");
		return -1;
	}	

	//路由表初始化
	route_table=(struct route*)malloc(sizeof(struct route));
	if(route_table==NULL) {
			printf("malloc error!!\n");
			return -1;
	}
	memset(route_table, 0, sizeof(struct route));

 	//调用添加函数insert_route往路由表里添加直连路由
//	insert_route(inet_addr("192.168.1.2"), 24, "eth12",
//			if_nametoindex("eth12"), inet_addr("192.168.3.2"));
//	insert_route(inet_addr("192.168.3.2."), 24, "eth13",
//			if_nametoindex("eth13"), inet_addr("192.168.1.2"));

	//创建线程去接收路由信息
	int pd;
	pd = pthread_create(&tid, NULL, thr_fn, NULL);

	while(1) {
		//接收ip数据包模块
		recvlen = recv(recvfd,skbuf,sizeof(skbuf),0);
		if(recvlen > 0) {
			ip_recvpkt = (struct ip *)(skbuf+ETHER_HEADER_LEN);
//			if(ip_recvpkt->ip_dst.s_addr==inet_addr("192.168.4.2") ||
//				ip_recvpkt->ip_dst.s_addr==inet_addr("127.0.0.1"))
//				continue;
			printf("Receive packet from %s", inet_ntoa(ip_recvpkt->ip_src));
			printf(" to %s\n", inet_ntoa(ip_recvpkt->ip_dst));
			//192.168.1.10是测试服务器的IP，现在测试服务器IP是192.168.1.10到192.168.1.80.
			//使用不同的测试服务器要进行修改对应的IP。然后再编译。
			//192.168.6.2是测试时候ping的目的地址。与静态路由相对应。
// 			if(ip_recvpkt->ip_src.s_addr == inet_addr("192.168.1.1")
// 			    && ip_recvpkt->ip_dst.s_addr == inet_addr("192.168.6.2"))
//			{
				//分析打印ip数据包的源和目的ip地址
			//	analyseIP(ip_recvpkt);

			printf("current route table----------\n");
			struct route* p = route_table;

			while(p != NULL && p -> ip4prefix.s_addr != 0) {
				//printf("%s\n",
				//	   inet_ntoa(p->nexthop->nexthopaddr));
				printf("from %s ", inet_ntoa(p->ip4prefix));
				printf("to - ifname: %s, nexthop: %s, prefixlen: %d\n",
					   p->nexthop->ifname, inet_ntoa(p->nexthop->nexthopaddr), p->prefixlen);
				p = p -> next;
			}

				printf("get ping ip package ");
				printf("from %s\n", inet_ntoa(ip_recvpkt->ip_src));
				int s;
				memset(data, 0, 1480);
				for(s = 0; s < 1480; s++) {
					data[s] = skbuf[s+34];
				}

				// 校验计算模块
				struct _iphdr *iphead;
				int c = 0;

				iphead=(struct _iphdr *)malloc(sizeof(struct _iphdr));
				memcpy(iphead, ip_recvpkt, 20);

//				printf("origin checksum:%d\n", iphead->checksum);
//				printf("origin ttl:%d\n", iphead->ttl);

				//调用校验函数check_sum，成功返回1
				c = check_sum(iphead, 10, ip_recvpkt->ip_sum);

				if(c == 1) {
					printf("checksum is ok!!\n");
				} else {
					printf("checksum is error !!\n");
					continue;
				}

				//调用计算校验和函数count_check_sum，返回新的校验和
				count_check_sum(iphead);


				//查找路由表，获取下一跳ip地址和出接口模块
				struct nextaddr *nexthopinfo;
				nexthopinfo = (struct nextaddr *)malloc(sizeof(struct nextaddr));
				memset(nexthopinfo, 0, sizeof(struct nextaddr));

				//调用查找路由函数lookup_route，获取下一跳ip地址和出接口
				if(lookup_route(ip_recvpkt -> ip_dst, nexthopinfo) == -1)
					continue;

				//arp find
				struct arpmac *dstmac;
				dstmac = (struct arpmac*)malloc(sizeof(struct arpmac));
				memset(dstmac, 0, sizeof(struct arpmac));

				//调用arpGet获取下一跳的mac地
				printf("%s\n", inet_ntoa(nexthopinfo->ipv4addr));
				unsigned char mac[6];
				if(nexthopinfo -> ipv4addr.s_addr == inet_addr("0.0.0.0")) {
					printf("looking for mac address for subnet\n");

					if(arpGet(skbuf, nexthopinfo->ifname, ip_recvpkt->ip_dst) < 0)
						continue;
				}
				else {
					if(arpGet(skbuf, nexthopinfo->ifname, nexthopinfo->ipv4addr) < 0)
						continue;
				}

				//get source mac
				if(macGet(skbuf + 6, nexthopinfo->ifname) < 0)
					continue;

				int i;
				for(i=0; i < 12; i++) {
					printf("%02x ", (unsigned char) skbuf[i]);
				}
				printf("\n");

				memcpy(skbuf+ETHER_HEADER_LEN, iphead, 20);

				memcpy(iphead, skbuf+ETHER_HEADER_LEN, 20);

//				printf("current checksum:%d\n", iphead->checksum);
//				printf("current ttl:%d\n", iphead->ttl);
				printf("%d\n", sizeof(struct ether_header));
				struct ether_header* eth_header =
						(struct ether_header*)malloc(sizeof(struct ether_header));
				eth_header = (struct ether_header*)skbuf;
				eth_header -> ether_type = htons(ETHERTYPE_IP);

				printf("new header:");
				for(i = 0; i < 6; i++) {
					printf("%02x ", eth_header->ether_dhost[i]);
				}

				for(i = 0; i < 6; i++) {
					printf("%02x ", eth_header->ether_shost[i]);
				}
				printf("\n");

				//send ether icmp
				//调用ip_transmit函数   填充数据包，通过原始套接字从查表得到的出接口(比如网卡2)将数据包发送出去
				//将获取到的下一跳接口信息存储到存储接口信息的结构体ifreq里，通过ioctl获取出接口的mac地址作为数据包的源mac地址
				//封装数据包：
				//<1>.根据获取到的信息填充以太网数据包头，以太网包头主要需要源mac地址、目的mac地址、以太网类型eth_header->ether_type = htons(ETHERTYPE_IP);
				//<2>.再填充ip数据包头，对其进行校验处理；
				//<3>.然后再填充接收到的ip数据包剩余数据部分，然后通过raw socket发送出去
				int sendfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);

				struct sockaddr_ll sadr_ll;

				sadr_ll.sll_ifindex = if_nametoindex(nexthopinfo->ifname);
				sadr_ll.sll_halen = ETH_ALEN;
				memcpy(sadr_ll.sll_addr, skbuf, ETH_ALEN);

				printf("send interface: %s(%d)\n", nexthopinfo->ifname,if_nametoindex(nexthopinfo->ifname));
				printf("send to MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
					   sadr_ll.sll_addr[0], sadr_ll.sll_addr[1], sadr_ll.sll_addr[2],
					   sadr_ll.sll_addr[3], sadr_ll.sll_addr[4], sadr_ll.sll_addr[5]);
				if (sendto(sendfd, skbuf, recvlen, 0, (const struct sockaddr *)&sadr_ll,
						sizeof(struct sockaddr_ll)) < 0) {
					printf("send error\n");
				} else {
					printf("send succeed.\n");
				}
				close(sendfd);
				printf("---------------------------\n");
			}
		}
//	}
	close(recvfd);	
	return 0;
}

