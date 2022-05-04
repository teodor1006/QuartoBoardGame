#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "connection.h"

int sock_fd;

int performConnection(char* hostname, char* port) {

	struct addrinfo hints, *serverInfo, *next;
	int s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	s = getaddrinfo(hostname, port, &hints, &serverInfo);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures
	   Try each address until we successfully connect
	   If socket() or connect() fails, we close the socket and try the next address*/

	for (next = serverInfo; next != NULL; next = next->ai_next) {

		sock_fd = socket(next->ai_family, next->ai_socktype, next->ai_protocol);
		if (sock_fd == -1) {
			printf("Socket creation failed");
			continue;
		}
		if (connect(sock_fd, next->ai_addr, next->ai_addrlen) != -1) {
			printf("Connection made!\n\n");
			break;
		}

		close(sock_fd);
	}

	// If there are no more addresses, connection wasnt made.

	if (next == NULL) {
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	/* Regardless of whether the connection was made or not,
	   the structure is no longer needed and released */

	freeaddrinfo(serverInfo);
	return 0;
}

int getSocketFd() {
	return sock_fd;
}

void disconnect() {
	if (close(sock_fd) < 0) {
		perror("error closing the socket");
	} else {
		printf("Successfully closed the socket\n");
	}
}
