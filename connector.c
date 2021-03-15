#include"main.h"
#include"performConnection.h"
#include"config.h"
#include"connector.h"
#include"shMemStrct.h"
#include"shMem.h"

int clSock(){
    int sockC = install_sock();
    if(sockC == EXIT_FAILURE){
        fprintf(stderr, "Failed to install sock!");
    } else {
        printf("Successfully installed sock!\n");
    }

    return sockC;
}
int connectCl(confVars* ptr, int shMId, shareS* shMemP, int sockC) {
	int retVal;

    if((retVal = performConnection(sockC, ptr, shMId, shMemP)) == EXIT_FAILURE){
    		printf("Failed to establish connection!\n");
    		return EXIT_FAILURE;
    	}else{
    		printf("Successfully ended the prolog phase!\n\n");
    	}

    return EXIT_SUCCESS;
}

int gameRoutine(int* fd, int shMId, shareS* shMemP, int sockC) {
	char *server_com = (char *)malloc(sizeof(char)*BUFFER);
	char *connector_ans = (char *)malloc(sizeof(char)*BUFFER);

	char server_com_split [2][100];
	int err, pC_read; 
	char p_buffer[BUFFER];

	while (true) {
		memset(connector_ans, 0, BUFFER);
		recieveS(server_com, sockC);
		printf("%s\n", server_com);
            
		if (server_com[0] == '-') {
			printf("%s\n", server_com);
			free(server_com);
			free(connector_ans);
			return EXIT_FAILURE;

		} else if(strcmp(server_com, "+ WAIT\n") == 0) {
			strcpy(connector_ans, "OKWAIT\n");
			if ((err = send(sockC, connector_ans, strlen(connector_ans), 0)) == -1){
				perror("Failed to send answer.\n");
				free(server_com);
				free(connector_ans);
				return EXIT_FAILURE;
			}
			printf("Waiting..\n");

		} else if(server_com[2] == 'M' && server_com[3] == 'O' && server_com[4] == 'V' && server_com[5] == 'E' && server_com[6] == ' ') {
			shMemP = (shareS *) atShm(shMId);
			int c = 0;
			recieveS(server_com, sockC);
			printf("%s\n", server_com);
			while(true) {
				recieveS(server_com, sockC);
				printf("%s\n", server_com);
				if(strcmp(server_com, "+ ENDPIECESLIST\n") != 0){
					memset(shMemP->pList[c], 0, (28*sizeof(char)));
					splitStr(server_com, server_com_split, " ");
					strcpy(shMemP->pList[c], server_com_split[1]);
					c++;
				} else {
					strcpy(connector_ans, "THINKING\n");
					printf("%s\n", connector_ans);
					if ((err = send(sockC, connector_ans, strlen(connector_ans), 0)) == -1){
						perror("Failed to send answer. Retrying..\n");
					} else {
						break;
					}	
				}
			}
		} else if(strcmp(server_com, "+ OKTHINK\n") == 0) {
			shMemP->thFlag = 1;
			int pidTH = shMemP->pidTh;
			shmdt(shMemP);
			kill(pidTH, SIGUSR1);
			printf("Waking up Th\n");
			if((pC_read = read (fd[0], p_buffer, BUFFER)) == -1) {
				printf("Thinker read failed!\n");
				free(server_com);
				free(connector_ans);
				return EXIT_FAILURE;
			}
			strcpy(connector_ans, "PLAY ");
			strcat(connector_ans, p_buffer);
			strcat(connector_ans, "\n");
			printf("%s\n", connector_ans);

			if ((err = send(sockC, connector_ans, strlen(connector_ans), 0)) == -1){
				perror("Failed to send answer.\n");
				free(server_com);
				free(connector_ans);
				return EXIT_FAILURE;
			}
			memset(p_buffer, 0, BUFFER);
		} else if(strcmp(server_com, "+ GAMEOVER\n") == 0) {
			shMemP = (shareS *) atShm(shMId);
			int c = 0;
			recieveS(server_com, sockC);
			printf("%s\n", server_com);
			while(true){
				recieveS(server_com, sockC);
				printf("%s\n", server_com);
				if(strcmp(server_com, "+ QUIT\n") != 0){
					memset(shMemP->pList[c], 0, (28*sizeof(char)));
					splitStr(server_com, server_com_split, " ");
					strcpy(shMemP->pList[c], server_com_split[1]);
					c++;
				} else {
					printf("Spiel verloren, schade Schokolade. Endspielstand folgt: \n");
					break;
				}
			} 
			shmdt(shMemP);
		} else if (strcmp(server_com, "+ MOVEOK\n") == 0)  {
			break;
		} else {
			printf("Waiting for server answer..\n\n");
		}
	}


	free(server_com);
	free(connector_ans);
	return EXIT_SUCCESS;
}