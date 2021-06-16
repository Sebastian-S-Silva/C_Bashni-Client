#include"main.h"
#include"performConnection.h"
#include"config.h"
#include"connector.h"
#include"shMemStrct.h"
#include"shMem.h"
#include"signal.h"


int install_sock() {
	int sockI = socket(AF_INET, SOCK_STREAM, 0);
	if (sockI < 0) {
		printf("Error creating socket");
		return EXIT_FAILURE;
	}
	return sockI;
}

int recieveS(char* server_res, int sockC) {
	char *buffer = (char*)malloc(sizeof(char)*BUFFER);
    int bytes_recieved = 0;
    int tst = 0;
    memset(server_res, 0, BUFFER);
    while(true){
        memset(buffer, 0, BUFFER);
        if((tst = recv(sockC, buffer, 1, 0)) == -1){
        	perror("Failed to recieve servermessage correctly");
        	free(buffer);
        	return EXIT_FAILURE;
        }
        strncat(server_res, buffer, tst);
        bytes_recieved += tst;
        if(server_res[bytes_recieved-1] == '\n') break;
    }
    free(buffer);
	return EXIT_SUCCESS;
}

int splitStr(char* rawStr, char splitStr[][100], char* delim){
	int i = 0;
	char *ptr = strtok(rawStr, delim);
	while(ptr != NULL) {
		strcpy(splitStr[i], ptr);
		i++;
		ptr = strtok(NULL, delim);
	}
	return i;
}

int performConnection (int sockC, confVars* conf_ptr, int shMId, shareS* shMemP) {
	char *server_res = (char *)malloc(sizeof(char)*BUFFER);
	char *client_ans = (char *)malloc(sizeof(char)*BUFFER);
	char resSplit [10][100];
	struct hostent *hInf;
	int err;

	if((hInf = gethostbyname(HOSTNAME)) == NULL) {
		perror("gethostbyname() failed\n");
		free(server_res);
    	free(client_ans);
		return EXIT_FAILURE;
	}
	
	struct sockaddr_in server_addr;
	memcpy(&server_addr.sin_addr, hInf->h_addr_list[0], hInf->h_length);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(conf_ptr -> pNumber);
	server_addr.sin_addr.s_addr = *(long *)(hInf->h_addr_list[0]);

	if(connect(sockC, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0){
		perror("There was an error establishing a connection to the server\n");
		free(server_res);
    	free(client_ans);
		return EXIT_FAILURE;
	}else{
		printf("Successfully connected to server. Waiting for response...\n");
	}
	
	while (true) {
		recieveS(server_res, sockC);
		memset(client_ans, 0, BUFFER);
		for(int i = 0; i < 10; i++){
            memset(resSplit[i], 0, 100);
        }

		if(server_res[0] == '-'){
			printf("%s\n", server_res);
			printf("The server response is negative. The connection will be interrupted.\n");
			exit(0);
		} else if (server_res[2] == 'M' && server_res[3] == 'N' && server_res[4] == 'M' && server_res[5] == ' ') {
			strcpy(client_ans, "VERSION ");
			strcat(client_ans, VERSION);
			if ((err = send(sockC, client_ans, strlen(client_ans), 0)) == -1){
				perror("Failed to send version number to server. Retrying..\n");
			};

			printf("%s\n", server_res);
			printf("%s\n", client_ans);
		} else if(server_res[2] == 'C' && server_res[3] == 'l' && server_res[4] == 'i' && server_res[5] == 'e' && server_res[6] == 'n' && server_res[7] == 't' && server_res[8] == ' ') {
			strcpy(client_ans, "ID ");
			strcat(client_ans, conf_ptr -> game_ID);
			strcat(client_ans, "\n");
			if ((err = send(sockC, client_ans, strlen(client_ans), 0)) == -1){
				perror("Failed to send game-ID to server. Retrying..\n");
			};

			printf("%s\n", server_res);
			printf("%s\n", client_ans);
		} else if(server_res[10] == 'B' && server_res[11] == 'a' && server_res[12] == 's' && server_res[13] == 'h' && server_res[14] == 'n' && server_res[15] == 'i' && server_res[16] == '\n'){
			printf("%s\n", server_res);

			recieveS(server_res, sockC);
			printf("%s\n", server_res);
			splitStr(server_res, resSplit, " ");

			shMemP = (shareS *) atShm(shMId);
			strcpy(shMemP->gameName, resSplit[1]);
			shmdt(shMemP);

			strcpy(client_ans, "PLAYER\n");
			if ((err = send(sockC, client_ans, strlen(client_ans), 0)) == -1){
				perror("Failed to send 'Player'-ćommand to server. Retrying..\n");
			};
		} else if(server_res[2] == 'Y' && server_res[3] == 'O' && server_res[4] == 'U' && server_res[5] == ' ') {
			printf("%s\n", server_res);
			splitStr(server_res, resSplit, " ");
			
			shMemP = (shareS *) atShm(shMId);
			shMemP->ownPlNum = (atoi(resSplit[2]));

			while (true) {
				if(server_res[2] == 'T' && server_res[3] == 'O' && server_res[4] == 'T' && server_res[5] == 'A' && server_res[6] == 'L' && server_res[7] == ' ') {
					splitStr(server_res, resSplit, " ");
					shMemP->players = (atoi(resSplit[2]));
				} else if(server_res[2] == 'E' && server_res[5] == 'P' && server_res[6] == 'L' && server_res[11] == 'S'){
                    break;
				} else {
					recieveS(server_res, sockC);
					printf("%s\n", server_res);
				}
			}
			shmdt(shMemP);
			free(server_res);
    		free(client_ans);
			return EXIT_SUCCESS;
		} else {
			free(server_res);
            free(client_ans);
			return EXIT_FAILURE;
		}
	}
}