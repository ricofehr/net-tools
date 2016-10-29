/*
* Simple ping program
* @author Eric Fehr (ricofehr@nextdeploy.io, @github: ricofehr)
*/

#include <stdio.h>

#include <linux/ip.h>
#include <linux/icmp.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define BUFSIZE 1500
int sockfd = 0;
uint16_t loop = 1;
uint16_t succ = 0;
uint16_t fail = 0;

void open_socket();
void init_iphdr(struct iphdr *ip, char *dest, int len);
void init_icmphdr(struct icmphdr *icmp, int dlen);
void init_target(struct sockaddr_in *target, struct in_addr in);
uint16_t ping_cheksum(uint16_t *addr, int len);
void ping_send(char *sendbuf, int len, struct sockaddr *target);
void ping_recv(char *dest);

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

/**
 *	init_iphdr - Init ip header of the packet
 *	@ip: empty ip header
 *	@dest: destination address
 *	@len: size of the packet
 */
void init_iphdr(struct iphdr *ip, char *dest, int len)
{
	memset(ip, 0, sizeof(struct iphdr));
	ip->version = 4;
	ip->ihl = 5;
	ip->tos = 0;
	ip->tot_len = htons(len);
	ip->id = htons(rand() % 255);
	ip->ttl = 64;
	ip->check = 0;
	ip->protocol = IPPROTO_ICMP;
	ip->daddr = inet_addr(dest);
}

/**
 *	init_icmphdr - Init icmp header
 *	@icmp: empty icmp header
 *	@dlen: size of data field
 */
void init_icmphdr(struct icmphdr *icmp, int dlen)
{
	memset(icmp, 0, sizeof(struct icmphdr));
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = getpid();

	memset(icmp + sizeof(struct icmphdr), rand() % 255, dlen);
	icmp->checksum = ping_cheksum((uint16_t *)icmp, sizeof(struct icmphdr) + dlen);
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
*	ping_cheksum - Compute checksum
*	@addr: address pointer of targetted data
*	@len: length of data
*
*	Compute a checksum for len bytes of data targeted by addr
*/
uint16_t ping_cheksum(uint16_t *addr, int len)
{
	/* register => keep value into cpu registers */
	register int sum = 0;
	uint16_t answer = 0;
	register uint16_t *w = addr;
	register int nleft = len;

	/*
	* sum: 32 bits accumulator
	* w: 16 bit words to add sequential
	*/
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

        /* Add odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *) (&answer) = *(u_char *) w;
		sum += answer;
	}

        /* add back carry outs from top 16 bits to low 16 bits */
	/* add hi 16 to low 16 */
	sum = (sum >> 16) + (sum & 0xffff);
	/* add carry */
	sum += (sum >> 16);
        /* truncate to 16 bits */
        answer = ~sum;
	return (answer);
}

/*
*	ping_send - Creates socket and send an ECHO request
*	@sendbuf: buffer with iphdr and icmphdr
*	@len: lenght of sendbuf
*	@target: destination host
*/
void ping_send(char *sendbuf, int len, struct sockaddr *target)
{
	int res;

	res = sendto(sockfd, sendbuf, len, 0,
	             target, sizeof(struct sockaddr_in));
	if (res <= 0)
		perror("Error send_ping");
	if (res != len)
		perror("Error send_ping (length sended != packet length");
}

/*
*	recv_ping - Try to get an ECHO_REPLY response
*	@dest: destination address
*/
void ping_recv(char *dest)
{
	uint8_t *buf;
	char addrbuf[128];
	struct iovec iov;
	struct iphdr *iphdr;
	struct icmphdr *icmphdr;
	char recvbuf[BUFSIZE];
	char controlbuf[BUFSIZE];
	struct msghdr msg;
	fd_set read_set;
	int rc;
	struct timeval timeout = {1, 0};

	memset(&read_set, 0, sizeof read_set);
        FD_SET(sockfd, &read_set);

        /* wait for a reply with a timeout */
        rc = select(sockfd + 1, &read_set, NULL, NULL, &timeout);
        if (rc == 0) {
		printf("No reply icmp_seq=%d Destination Host Unreachable\n", loop);
		return;
        } else if (rc < 0) {
		perror("Select");
		exit(1);
        }

	iov.iov_base = recvbuf;
	iov.iov_len = sizeof(recvbuf);
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = addrbuf;
	msg.msg_namelen = sizeof(addrbuf);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = controlbuf;
	msg.msg_controllen = sizeof(controlbuf);
	recvmsg(sockfd, &msg, 0);
	buf = msg.msg_iov->iov_base;
	iphdr = (struct iphdr*)buf;
	icmphdr = (struct icmphdr*)(buf + (iphdr->ihl*4));
	if (icmphdr->type == ICMP_ECHOREPLY) {
		printf("%d bytes from %s: icmp_seq=%d ttl=%d\n",
		        (int)sizeof(icmphdr), dest, loop, (int)iphdr->ttl);
		++succ;
	}

	if (icmphdr->type == ICMP_DEST_UNREACH) {
		printf("From 0.0.0.0 icmp_seq=%d Destination Host Unreachable\n", loop);
		++fail;
	}
}

/*
*	ping_end - stop signals handler
*/
static void ping_end(int sig, siginfo_t *siginfo, void *context)
{
	printf("\n--- ping statistics ---\n");
	printf("%d packets transmitted, %d packets received, %d packet loss\n", loop, succ, fail);
	exit(0);
}

int main(int argc, char **argv)
{
	char sendbuf[BUFSIZE];
	struct iphdr *ip = (struct iphdr *)sendbuf;
	struct icmphdr *icmp = (struct icmphdr *)(ip + 1);

	struct sockaddr_in target;
	struct in_addr in;
	int dlen = 56, pck_len;
	struct sigaction act;

 	/* init sigaction object */
 	memset (&act, '\0', sizeof(act));

	/* Use the sa_sigaction field */
	act.sa_sigaction = &ping_end;

	/* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field */
	act.sa_flags = SA_SIGINFO;
	if (sigaction(SIGINT, &act, NULL) < 0) {
		perror ("sigaction SIGINT failed to handled");
		return 1;
	}

	if (sigaction(SIGTSTP, &act, NULL) < 0) {
		perror ("sigaction SIGTSTP failed to handled");
		return 1;
	}
	
	if (argc != 2) {
		printf("usage: %s dest\n", argv[0]);
		return 1;
	}

	if (inet_aton(argv[1], &in) == 0) {
		perror("inet_aton");
		printf("%s isn't a valid address\n", argv[1]);
		return 1;
	}

	pck_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + dlen;
	init_target(&target, in);
	init_iphdr(ip, argv[1], dlen);
	init_icmphdr(icmp, dlen);

	open_socket();

	printf("PING %s (%s) %d(%d) bytes of data.\n", argv[1], argv[1], dlen, pck_len);

	for (;;) {
		ping_send(sendbuf, pck_len, (struct sockaddr *)&target);
		ping_recv(argv[1]);
		sleep(1);
		loop++;
	}

	if (sockfd != 0)
		close(sockfd);
}
