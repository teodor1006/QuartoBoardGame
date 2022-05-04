#define BUFFSIZE 512

#ifndef communication
#define communication

char* readSock(int sock_fd);
char* writeSock(int sock_fd, char* str);
char* getVersion();
char* sendMove(int sock_fd,char *spielzug);
#endif
