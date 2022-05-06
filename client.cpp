#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
// #include "stack.hpp"


#include <arpa/inet.h>

#define PORT "5013" 

#define MAXDATASIZE 100 
#define ip "127.0.0.1"


void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main()
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(ip, PORT, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1)
		{
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			  s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo);

	char data[1024];
	char buffer[1024];
	int choice;
	while (1)
	{
		memset(data, 0, strlen(data));
		memset(buffer, 0, strlen(buffer));
		printf("PUSH\n POP\n TOP\n EXIT\n");
		printf("\nEnter your choice : ");
		fgets(data, 1024, stdin);
		if (send(sockfd, data, strlen(data) + 1, 0) == -1)
		{
			perror("send");
		}
		if (strncmp(data, "EXIT", 4) == 0)
		{
			break;
		}
		if (strncmp(data, "TOP", 3) == 0)
		{
			printf("OUTPUT:");
			read(sockfd, buffer, 1024);
			printf("%s\n",buffer);
		}
	}

	close(sockfd);

	return 0;
}