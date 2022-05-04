#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>


#include "communication.h"
#include "connection.h"
#include "config.h"


char buff[BUFFSIZE] = {0};
char helpBuff[1000] = {0};
char c[1];

char* readSock(int sock_fd) {
	int bytes_read = -1;
	printf("Reading..\n");
	if ((strlen(buff)) > 0) {
		memset(buff, '\0', sizeof(buff));
	}
	while (*c != '\n' || bytes_read == -1) {
		if ((bytes_read = read(sock_fd, c, 1)) == -1) {
			perror("Read failed");
			exit(EXIT_FAILURE);
		}

		strncat(buff, c, 1);
	}

	if (strncmp(buff, "+ FI", 4) == 0) {
		strcat(helpBuff, buff);
		for (int line = 0; line < 4; line++) {
			strcat(helpBuff, readSock(sock_fd));
		}
		strcpy(buff, helpBuff);
		memset(helpBuff, '\0', sizeof(helpBuff));
	}

	if (strncmp("+", buff, 1) == 0) {
		printf("Server says:\n%s\n", buff);

	} else if (strncmp("- ", buff, 2) == 0) {
		printf("\033[1;31m");
		printf("Server says:\n%s\n", buff);
		printf("\033[0m");

	} else {
		printf("Something went wrong with readSock\n");
		exit(EXIT_FAILURE);
	}

	return buff;
}

char* writeSock(int sock_fd, char* str) {

	ssize_t bytes_sent = 0;
	size_t remaining_bytes = strlen(str) * sizeof(char);

	printf("Writing..\n");
	while (remaining_bytes) {
		if ((bytes_sent = write(sock_fd, str, remaining_bytes)) == -1) {
			perror("write failed");
			exit(EXIT_FAILURE);
		} else printf("Message sent: %s\n", str);

		remaining_bytes -= bytes_sent;
	}

	return buff;
}

char* getVersion() {
	printf("Getting Version..\nBuffer is: %s\n", buff);
	char version[13] = "VERSION ";
	for (size_t i = 0; i < strlen(buff); i++) {
		if (buff[i] == 'v' && isdigit(buff[i + 1])) {
			int n = i + 1;
			while (isspace(buff[n]) == 0) {
				strncat(version, &buff[n], 1);
				n++;
			}
			strcat(version, "\n");
			break;
		}
	}
	if (strlen(version) <= 8) {
		printf("Reading the server version failed\n");
		exit(EXIT_FAILURE);
	} else {
		if (strlen(buff) != 0) memset(buff, '\0', sizeof(buff));
		strcat(buff, version);
	}
	printf("Returning buffer: %s\n", buff);
	return buff;
}

char* sendMove(int sock_fd, char *spielzug) {
	char send_text[BUFFSIZE];
	snprintf(send_text, 15, "PLAY %s\n", spielzug);
	writeSock(sock_fd, send_text);
	printf("Done: %s\n", spielzug);

	return buff;
}

