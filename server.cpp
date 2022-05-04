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
#include "stack.hpp"
// include mmap header
#include <sys/mman.h>
#define MAX_LIMIT 1024
#define BUFSIZE 1024
#define PORT "5013"
#define BACKLOG 10



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

void func(int sock, int *size)
{
	char data[1024];
	char size_message[1024];
	int check;
	struct flock lock;
	memset(&lock, 0, sizeof(lock));

	while (1)
	{
		if ((check = recv(sock, data, 1024, 0)) == -1)
		{
			perror("recv");
			exit(1);
		}
		data[strlen(data)] = '\0';
		lock.l_type = F_WRLCK;
		fcntl(sock, F_SETLKW, &lock);
		if (strcmp(data, "EXIT") == 0)
		{
			lock.l_type = F_UNLCK;
			fcntl(sock, F_SETLKW, &lock);
			break;
		}
		else if (strncmp(data, "PUSH", 4) == 0)
		{
			int i = 0;
			int k = 5;
			char value[strlen(data)];
			while (i < strlen(data))
			{
				value[i] = data[k];
				i++;
				k++;
			}
			push(value);
			*size = *size + 1;
		}
		else if (strncmp(data, "POP", 3) == 0)
		{
			pop();
			*size = *size - 1;
		}
		else if (strncmp(data, "TOP", 3) == 0)
		{
			char *d = Top();
			if (d == NULL)
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
				char buffer[1024];
				memset(buffer, 0, strlen(buffer));
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
		}
		else if (strncmp(data, "size", 4) == 0)
		{
			bzero(data, sizeof(data));
			strcat(data, "DEBUG:");
			sprintf(size_message, "%d", *size);
			strncat(data, size_message, 1024);
			printf("DEBUG: size call %d\n", *size);
			send(sock, data, sizeof(data), 0);
		}
		else if (strncmp(data, "DISPLAY", 7) == 0)
		{
			// display();
		}
		lock.l_type = F_UNLCK;
		fcntl(sock, F_SETLKW, &lock);
	}
}

void sig_handler(int sig)
{
	printf("\nbye bye\n");
	exit(0);
}

int main()
{
	printf("server:to exit press ctrl+c or ctrl+z\n");
	signal(SIGINT, sig_handler);
	signal(SIGTSTP, sig_handler);
	int *size = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*size = 0;
	int sockfd,new_fd;
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
			close(sockfd);
			func(new_fd, size);
		}
	}
	close(new_fd);

	return 0;
}