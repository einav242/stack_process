#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/mman.h>
#include"malloc.hpp"
#include"stack.hpp"
#define MAX_LIMIT 1024
#define BUFSIZE 1024
#define PORT "5013"
#define BACKLOG 10
#define SIZE 1024

struct flock lock;
bool stop=false;

void sigchld_handler(int s)
{
	(void)s;

	int saved_errno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0)
		;

	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void func(int sock, int *size, Node**top)
{
	char data[1024];
	int check;
	while (1)
	{
		if ((check = recv(sock, data, 1024, 0)) == -1)
		{
			perror("recv");
			exit(1);
		}
		data[strlen(data)] = '\0';
		if (strcmp(data, "EXIT") == 0)
		{
			lock.l_type = F_UNLCK;
			fcntl(sock, F_SETLKW, &lock);
			break;
		}
		else if (strncmp(data, "PUSH", 4) == 0)
		{
			lock.l_type = F_WRLCK;
			fcntl(sock, F_SETLKW, &lock);
			int i = 0;
			int k = 5;
			char value[strlen(data)];
			while (i < strlen(data))
			{
				value[i] = data[k];
				i++;
				k++;
			}
			for (int i = 0; i < strlen(value); i++)
			{
				(*top)[*size].data[i] = value[i];
			}
			*size = *size + 1;
			lock.l_type = F_UNLCK;
			fcntl(sock, F_SETLKW, &lock);
		}
		else if (strncmp(data, "POP", 3) == 0)
		{
			lock.l_type = F_WRLCK;
			fcntl(sock, F_SETLKW, &lock);
			if (*size==0)
			{
				printf("\nEMPTY STACK\n");
			}
			else
			{
				strcpy((*top)[*size].data,"");
				*size = *size - 1;
			}
			lock.l_type = F_UNLCK;
			fcntl(sock, F_SETLKW, &lock);
		}
		else if (strncmp(data, "TOP", 3) == 0)
		{
			lock.l_type = F_WRLCK;
			fcntl(sock, F_SETLKW, &lock);
			if (*size==0)
			{
				char buffer[1024] = "Empty stack";
				if (send(sock, buffer, 1024, 0) == -1)
				{
					perror("send");
					exit(0);
				}
			}
			else
			{
				char *d =(*top)[*size-1].data;
				char buffer[1024];
				memset(buffer, 0, 1024);
				for (int i = 0; i < strlen(d); i++)
				{
					buffer[i] = d[i];
				}
				buffer[strlen(d)] = '\0';
				if (send(sock, buffer, 1024, 0) == -1)
				{
					perror("send");
					exit(0);
				}
			}
			lock.l_type = F_UNLCK;
			fcntl(sock, F_SETLKW, &lock);
		}
		lock.l_type = F_UNLCK;
		fcntl(sock, F_SETLKW, &lock);
	}
}

void sig_handler(int sig)
{
	exit(0);
}

int main()
{
	printf("server:to exit press ctrl+c or ctrl+z\n");
	signal(SIGINT, sig_handler);
	signal(SIGTSTP, sig_handler);
	int *size = (int *)new_malloc(sizeof(int));
	if(size==NULL)
	{
		perror("malloc");
	}
	*size = 0;
	memset(&lock, 0, sizeof(lock));
	int sockfd, new_fd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1)
		{
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
					   sizeof(int)) == -1)
		{
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);

	if (p == NULL)
	{
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("sigaction");
		exit(1);
	}
	Node* top =(Node*)new_malloc(sizeof(Node)*1000);
	if(top==NULL)
	{
		perror("malloc");
	}
	memset(top->data,0,1024);
	top->next=top+sizeof(Node);
	while (1)
	{
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd < 0)
		{
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family,
				  get_in_addr((struct sockaddr *)&their_addr),
				  s, sizeof s);
		printf("server: got connection from %s\n", s);
		if (!fork())
		{
			func(new_fd, size, &top);
		}
	}
	close(new_fd);

	return 0;
}