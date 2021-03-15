#ifndef PERFORMCONNECTION_H_
#define PERFORMCONNECTION_H_

#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdbool.h>

#include"config.h"
#include"shMemStrct.h"

#define BUFFER 512


extern int sockI;

                       

int install_sock();
int splitStr(char* rawStr, char splitStr[][100], char* delim);
int performConnection(int sockC, confVars* conf_ptr, int shMId, shareS* shMemP);

int recieveS(char* server_res, int sockC);

#endif