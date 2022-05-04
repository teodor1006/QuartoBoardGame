#ifndef connection
#define connection

int performConnection(char* hostname, char* port);
int getSocketFd();
void disconnect();

#endif
