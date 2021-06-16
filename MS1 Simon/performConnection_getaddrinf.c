#include "bashniclient.h"
#include "performConnection.h"

#include <time.h>// for millisecond sleep

int clientSocket;
char buffer[BUFFERSIZE];


/*sleep_milliseconds(long milliseconds); wie sleep nur ganuer 
 */
int sleep_milliseconds(long mseconds){
    struct timespec sleeptime;
    if(mseconds < 0){
        fprintf(stderr, "mseconds has to be positive");
        return -1;
    }
    sleeptime.tv_sec = mseconds / 1000;
    sleeptime.tv_nsec = (mseconds % 1000) * 1000000;
    nanosleep(&sleeptime, NULL);
    return 0;
}





int get_socket(){
    struct addrinfo* serverAddr;

    //hints for getaddr info 
    struct addrinfo hints;
    hints.ai_family= AF_INET;
    hints.ai_socktype= SOCK_STREAM;
    hints.ai_flags= AI_ADDRCONFIG;
    hints.ai_protocol= IPPROTO_TCP;
    char *portstring = malloc(sizeof(char)*5); //get addrinfo expexts const char* as snd argumet not int
    snprintf(portstring,5,"%u", PORTNUMBER);
    int ret = getaddrinfo(HOSTNAME, portstring, &hints, &serverAddr);
    free(portstring);
    if(ret != 0){
        fprintf(stderr,"getaddrinfo failed with exit code: %s\n", gai_strerror(ret));
        return -1;
    } 
    //freeaddrinfo(&hints);//free memory allocated by hints struct 
    
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0){
        fprintf(stderr, "socket failed: %d\n", clientSocket);
        return -2;
    }

    ret = connect(clientSocket, serverAddr->ai_addr, serverAddr->ai_addrlen);
    if(ret != 0){
        perror("connect() error: ");
        return -3;
    }
     
        
     
    return clientSocket;
}
/*
 *teilt str(base) in einzelne worte auf un speichert sie einzeln in target 
 *Rueckgabewert ist die anzahl Worte
 */    
int getWords(char *base, char target[][20])
{
	int n=0,i,j=0;
	
	for(i=0;true;i++)
	{
		if(base[i]!=' '){
			target[n][j++]=base[i];
		}
		else{
			target[n][j++]='\0';//insert NULL
			n++;
			j=0;
		}
		if(base[i]=='\0')
		    break;
	}
	return n;
	
}
int recieve_until_newline(char* servermessage, int* sock){
    char *buffer = (char*)malloc(sizeof(char)*BUFFERSIZE);
    int bytes_recieved =0;
    int retval = 0;
    memset(servermessage, 0, BUFFERSIZE);
    while(true){
        memset(buffer, 0, BUFFERSIZE);
        retval=recv(*sock, buffer, 1, 0);
        strncat(servermessage, buffer, retval);
        bytes_recieved+=retval;
        if(servermessage[bytes_recieved-1] == '\n') break;
    }
    free(buffer);
    return retval;
}
        
int performConnection(int* sock){ //prologphase
    int ret = 0; 

    char *servermessage =(char*)malloc(sizeof(char)*BUFFERSIZE);
    char *sendbuffer = (char*)malloc(sizeof(char)*BUFFERSIZE);
    char split_buffer[10][20]; // max anz woerter == 10 im prolog protokoll
    while(true){

        /*alle buffer leeren dmit strcat von vorne anfangt*/
        memset(sendbuffer, 0, BUFFERSIZE);
        memset (servermessage, 0, BUFFERSIZE);
        for(int i = 0; i < 10; i++){
            memset(split_buffer[i], 0, 20);
        }


        ret=recieve_until_newline(servermessage, sock);    
        if(ret < 0){
            perror("recv error: ");
            close(*sock);
            return EXIT_FAILURE;
        }


       // printf("server: %s\n", servermessage);
        //servermessage in einzelne woerter aufteilen 
        getWords(servermessage, split_buffer);

        //wenn erstes wort == '-' dann fehelermeldung ausgeben
        if(strncmp(split_buffer[0], "-", 1) == 0){
            fprintf(stderr, "server error: %s\n", servermessage);//fehlermeldung);
            break;//oder vlt auch return EXIT_FAILURE

        }
        else if(strncmp(split_buffer[0], "+", 1) == 0){
            //den rest ausgeben und nach protokolldefinition andtworten TODO pretty output

            if(strncmp(split_buffer[1], "MNM", 3) ==0){

                //MNM Gameserver <version> accepting connection
                printf("Connected to Gameserver(%s)\n", split_buffer[3]);
                printf("sending Client version...\n");
                strncpy(sendbuffer , "VERSION ", 9);
                strcat(sendbuffer,VERSION); 
                sleep(1);//notwendig da der server sonst zu langsam ist und die nachricht nicht bekommt 
                ret = send(*sock, sendbuffer, strlen(sendbuffer), 0);
        //        printf("sent: %s\n", sendbuffer);
                if(ret<0){
                    perror("send error: ");
                    return EXIT_FAILURE;
                }
                continue;
            }
            else if(strncmp(split_buffer[1], "Client", 6)==0){
                //client version accepted pls send id
                printf("Client version accepted by Server\nsending Game-ID: %s\n\n", game_id);
                strncpy(sendbuffer , "ID ", 4);
                strncat(sendbuffer, game_id, 13);
                strcat(sendbuffer, "\n");
                sleep_milliseconds(100);
                ret = send(*sock, sendbuffer, strlen(sendbuffer), 0);
                if(ret<0){
                    perror("send error: ");
                    return EXIT_FAILURE;
                }

   //             printf("sent: %s\n", sendbuffer);

            }
            else if(strncmp(split_buffer[1], "PLAYING", 7) ==0){
                //da Game-Name arbitraer aber immer nach PLAYING ist in diesem else-if blokc
                printf("Playing %s", split_buffer[2]);
                ret = recieve_until_newline(servermessage, sock);
                if(ret < 0){
                    fprintf(stderr, "recieve error");
                    return EXIT_FAILURE;
                }
                getWords(servermessage, split_buffer);
                if(strncmp(split_buffer[0], "-", 1) == 0){
                    fprintf(stderr, "server error: %s\n", servermessage);
                    return EXIT_FAILURE;
                }
                else{
                    printf("Game-Name: %s", split_buffer[1]);
                }
                

                strncpy(sendbuffer, "PLAYER\n", 8);//optional anz spieler
                ret= send(*sock, sendbuffer, strlen(sendbuffer), 0);
    //            printf("sent: %s\n", sendbuffer);
                if(ret<0){
                    perror("PLAYER send error: ");
                }
            }

            else if(strncmp(split_buffer[1], "YOU", 3) == 0){
                int Mitspielernummer = atoi(split_buffer[2]);//mitspielerzahl evtl in header datei falls die variable ausserhalb dieser funktion gebraucht wird
                char* Mitspielername = split_buffer[3]; //same here
                //da unbenutzte vars zu warnungen fuehren werden sie hier noch auf stdout ausgegeben
                printf("Mitspielernummer: %d\nMitspielername: %s\n", Mitspielernummer, Mitspielername);
            }
            else if(strncmp(split_buffer[1], "TOTAL", 5) ==0){
                int mitspielerzahl = atoi(split_buffer[2]);//auch hier variable global definiern falls man sie noch braucht
                printf("Mitspielerzahl: %d\n", mitspielerzahl);
                printf("Andere Mitspieler:\n");
                while(true){//empfangen und auf stdout ausgeben bis ENDPLAYERS empfangen wird

                    ret = recieve_until_newline(servermessage, sock);
                    if(ret < 0){
                        perror("recv error: ");
                        return EXIT_FAILURE;
                    }
                    for(int i =0; i < 10; i++){
                        memset(split_buffer[i], 0, 20);
                    }
                    getWords(servermessage, split_buffer);

                    if(strncmp(split_buffer[0], "-", 1) == 0){
                        fprintf(stderr, "server error: %s\n",servermessage);
                        break;
                    }

                    if(strncmp(split_buffer[1], "ENDPLAYERS", 10) ==0){
                        printf("end of prolog!\n");
                        free(sendbuffer);
                        free(servermessage);
                        return EXIT_SUCCESS;
                    }
                    printf("Mitspielername: %s Mitspielernummer: %s Bereit: %s\n", split_buffer[2], split_buffer[1], split_buffer[3]);
                    //printf("Server: %s\n", servermessage);

                }

            }

        }
        else{
            fprintf(stderr, "something weird happened");
            close(*sock);
            return EXIT_FAILURE;
        }






    }
    free(sendbuffer);
    //free(buffer);
    free(servermessage);

    return EXIT_SUCCESS;
}
