#ifndef PERFORMCONNECTION
#define PERFORMCONNECTION

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>
#include "config.h"
#define BUFFERSIZE 512
#include "sharedmemstruct.h"


extern int clientSocket;                        

int get_socket();
int performConnection(int* sock, configvars* param_pointer, sharestruct* sharedmem); 
int recieve_until_newline(char* servermessage, int* sock);
int getWords(char *base, char target[][100]);
int sleep_milliseconds(long mseconds);


#endif
