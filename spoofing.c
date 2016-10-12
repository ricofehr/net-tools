/*
* Simple program who send udp packet with ip spoofing
* @author Eric Fehr (ricofehr@nextdeploy.io, @github: ricofehr)
*/
#include <stdio.h>

#include <linux/ip.h>
#include <linux/udp.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int sockfd = 0;

void open_socket();
void send_datas(char *sendbuf, int len, struct sockaddr *target);
void init_target(struct sockaddr_in *target, struct in_addr in);
void init_iphdr(struct iphdr *ip, char *src, char *dest, int proto, int len);
void init_udphdr(struct udphdr *udp, int src, int dest, int len);

/**
 *	open_socket - Open a raw socket
 */
void open_socket()
{
	int on = 1;

	/* creates socket if needed */
	if (sockfd == 0) {
		sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
		if (sockfd < 0) {
			perror("Error on socket creation");
			exit(1);
		}


		/* socket options, tell the kernel we provide the IP structure */
        	if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
			perror("setsockopt() for IP_HDRINCL error");
			exit(1);
		}
	}
}

/*
*	send - Send datas on socket
*	@sendbuf: buffer with iphdr and icmphdr
*	@len: lenght of sendbuf
*	@target: destination host
*/
void send_datas(char *sendbuf, int len, struct sockaddr *target)
{
	int res;

	res = sendto(sockfd, sendbuf, len, 0,
	             target, sizeof(struct sockaddr_in));
	if (res <= 0)
		perror("Error send_ping");
	if (res != len)
		perror("Error send_ping (length sended != packet length");
}

/**
 *	init_target - Init target host struct
 *	@target: empty target struct
 *	@in: ip address of the destination
 */
void init_target(struct sockaddr_in *target, struct in_addr in)
{
	target->sin_family = AF_INET;
	target->sin_addr = in;
	target->sin_port = htons(0);
}

/**
 *	init_iphdr - Init ip header of the packet
 *	@ip: empty ip header
 *	@src; source address
 *	@dest: destination address
 *	@proto: transport protocol
 *	@len: size of the packet
 */
void init_iphdr(struct iphdr *ip, char *src, char *dest, int proto, int len)
{
	memset(ip, 0, sizeof(struct iphdr));
	ip->ihl = 0x5;
	ip->version = 0x4;
	ip->tos = 0x0;
	ip->tot_len = len;
	ip->id = htons(rand() % 255);
	ip->frag_off = 0x0;
	ip->ttl = 64;
	ip->protocol = proto;
	ip->check = 0x0;
	ip->saddr = inet_addr(src);
	ip->daddr = inet_addr(dest);
}

/**
 *	init_udphdr - Init udp header of the packet
 *	@udp: empty udp header
 *	@src; source port
 *	@dest: destination port
 *	@len: size of the udp part
 */

void init_udphdr(struct udphdr *udp, int src, int dest, int len)
{
	udp->source = htons(src);
	udp->dest = htons(dest);
	udp->len = htons(len);
	udp->check = 0;
	memset(udp + 8, 'a', len - 8);
}

int main(int argc, char **argv)
{
	unsigned char *packet;
	struct iphdr *ip;
	struct udphdr *udp;
	struct sockaddr_in target;
	struct in_addr in;
	int len = 60, port;

	if (argc != 4) {
		printf("usage: %s src dest port\n", argv[0]);
		return 1;
	}

	if (inet_aton(argv[2], &in) == 0) {
		perror("inet_aton");
		printf("%s isn't a valid address\n", argv[2]);
		return 1;
	}

	port = atoi(argv[3]);
	packet = (unsigned char *) malloc(len);
	init_target(&target, in);
	ip = (struct iphdr *) packet;
	init_iphdr(ip, argv[1], argv[2], IPPROTO_UDP, len);
	udp = (struct udphdr *)(packet + 20);
	init_udphdr(udp, 45512, port, 30);

	open_socket();
	send_datas(packet, 60, (struct sockaddr *)&target);
	free(packet);
	close(sockfd);
}
