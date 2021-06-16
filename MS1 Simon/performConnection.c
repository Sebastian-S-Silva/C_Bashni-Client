#include "bashniclient.h"


int clientSocket;
//char buffer[BUFFERSIZE];


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





int get_socket(configvars* param_pointer){
    struct addrinfo* serverAddr;

    //hints for getaddr info 
    struct addrinfo hints;
    hints.ai_family= AF_INET;
    hints.ai_socktype= SOCK_STREAM;
    hints.ai_flags= AI_ADDRCONFIG;
    hints.ai_protocol= IPPROTO_TCP;


    char *portstring = malloc(sizeof(char)*5); //get addrinfo expexts const char* as snd argumet not int
    snprintf(portstring,5,"%u", param_pointer->portnumber);
    int ret = getaddrinfo(param_pointer->hostname, portstring, &hints, &serverAddr);
    free(portstring);
    /*
    printf("creating socket..\n");
    struct hostent* hInf = gethostbyname(param_pointer->hostname);
    if( hInf == NULL){
        perror("could not resolve hostname: ");
        return EXIT_FAILURE;
    }*/
    if(ret != 0){
        fprintf(stderr,"(get_socket) getaddrinfo failed with exit code: %s\n", gai_strerror(ret));
        return -1;
    }
    /*
    struct sockaddr_in server_addr;
    memcpy(&server_addr.sin_addr, hInf->h_addr_list[0], hInf->h_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(param_pointer -> portnumber);
    */

    
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0){
        fprintf(stderr, "(get_socket) socket failed: %d\n", clientSocket);
        return -2;
    }

    ret = connect(clientSocket, serverAddr->ai_addr, serverAddr->ai_addrlen);
    freeaddrinfo(serverAddr);
    if(ret != 0){
        perror("(get_socket) connect() error: ");
        return -3;
    }
     
        
     
    return clientSocket;
}
/*
 *teilt str(base) in einzelne worte auf un speichert sie einzeln in target 
 *Rueckgabewert ist die anzahl Worte
 */    
int getWords(char *base, char target[][100]){
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
    char *buffer = (char*)malloc(sizeof(char)*2);
    int bytes_recieved =0;
    int retval = 0;
    memset(servermessage, 0, BUFFERSIZE);
    servermessage[0] = '\0';
    while(true){
        memset(buffer, 0, 2);
        retval=recv(*sock, buffer, 1, 0);
        if(retval<0) break;
        strncat(servermessage, buffer, retval);
        bytes_recieved+=retval;
        if(buffer[0] == '\n') break;
        //if(bytes_recieved == 0) break;
        //if(servermessage[bytes_recieved-1] == NULL) break;
        //if(servermessage[bytes_recieved-1] == '\n') break;
    }
    free(buffer);
    return retval;
}
/*
int recieve_until_newline(char* receiveBuffer, int* socket) {
    memset(receiveBuffer, 0, sizeof(char)*BUFFERSIZE);
    printf("%s", receiveBuffer);
    int ret;
    char buffer[1] = "0";

    while (buffer[0] != '\n') {  // receive Message
        ret = recv(*socket, buffer, 1 , 0 );
        strncat(receiveBuffer, buffer, 1);
    }
    printf("S: %s", receiveBuffer);
    buffer[0] = 0;
    return ret;
}
*/

        
int performConnection(int* sock, configvars* param_pointer, sharestruct* sharedmem){ //prologphase
    int ret = 0; 

    char *servermessage =(char*)malloc(sizeof(char)*BUFFERSIZE);
    char *sendbuffer = (char*)malloc(sizeof(char)*BUFFERSIZE);
    char split_buffer[10][100]; // max anz woerter == 10 im prolog protokoll
    while(true){

        /*alle buffer leeren dmit strcat von vorne anfangt*/
        memset(sendbuffer, 0, BUFFERSIZE);
        memset (servermessage, 0, BUFFERSIZE);
        for(int i = 0; i < 10; i++){
            memset(split_buffer[i], 0, 20);
        }


        ret=recieve_until_newline(servermessage, sock);    
        if(ret < 0){
            perror("(performConnection) recv error: ");
            close(*sock);
            return EXIT_FAILURE;
        }


       // printf("server: %s\n", servermessage);
        //servermessage in einzelne woerter aufteilen 
        getWords(servermessage, split_buffer);

        //wenn erstes wort == '-' dann fehelermeldung ausgeben
        if(strncmp(split_buffer[0], "-", 1) == 0){
            fprintf(stderr, "(performConnection) server error: %s\n", servermessage);//fehlermeldung);
            break;//oder vlt auch return EXIT_FAILURE

        }
        else if(strncmp(split_buffer[0], "+", 1) == 0){
            //den rest ausgeben und nach protokolldefinition andtworten TODO pretty output

            if(strncmp(split_buffer[1], "MNM", 3) ==0){

                //MNM Gameserver <version> accepting connection
                printf("(performConnection) Connected to Gameserver(%s)\n", split_buffer[3]);
                printf("(performConnection) sending Client version...\n");
                strncpy(sendbuffer , "VERSION ", 9);
                strcat(sendbuffer,VERSION); 
                sleep(1);//notwendig da der server sonst zu langsam ist und die nachricht nicht bekommt 
                ret = send(*sock, sendbuffer, strlen(sendbuffer), 0);
        //        printf("sent: %s\n", sendbuffer);
                if(ret<0){
                    perror("(performConnection) send error: ");
                    return EXIT_FAILURE;
                }
                continue;
            }
            else if(strncmp(split_buffer[1], "Client", 6)==0){
                //client version accepted pls send id
                printf("(performConnection) Client version accepted by Server\nsending Game-ID: %s\n\n",param_pointer->game_ID);
                strncpy(sendbuffer , "ID ", 4);
                strncat(sendbuffer, param_pointer->game_ID, 13);
                strcat(sendbuffer, "\n");
                sleep_milliseconds(100);
                ret = send(*sock, sendbuffer, strlen(sendbuffer), 0);
                if(ret<0){
                    perror("(performConnection) send error: ");
                    return EXIT_FAILURE;
                }

   //             printf("sent: %s\n", sendbuffer);

            }
            else if(strncmp(split_buffer[1], "PLAYING", 7) ==0){
                //da Game-Name arbitraer aber immer nach PLAYING ist in diesem else-if blokc
                printf("(performConnection) Playing %s", split_buffer[2]);

                if(strncmp(split_buffer[2], "Bashni", 6)!=0){
                    fprintf(stderr, "(performConnection) Game not bashni... quitting\n");
                    return EXIT_FAILURE;
                }
                ret = recieve_until_newline(servermessage, sock);
                if(ret < 0){
                    fprintf(stderr, "(performConnection) recieve error");
                    return EXIT_FAILURE;
                }
                getWords(servermessage, split_buffer);
                if(strncmp(split_buffer[0], "-", 1) == 0){
                    fprintf(stderr, "(performConnection) server error: %s\n", servermessage);
                    return EXIT_FAILURE;
                }
                else{
                    printf("(performConnection) Game-Name: %s", split_buffer[1]);
                }
                printf("servermessage: %s\n", servermessage);
                

                strncpy(sendbuffer, "PLAYER\n", 8);//optional anz spieler
                ret= send(*sock, sendbuffer, strlen(sendbuffer), 0);
    //            printf("sent: %s\n", sendbuffer);
                if(ret<0){
                    perror("(performConnection) PLAYER send error: ");
                }
            }

            else if(strncmp(split_buffer[1], "YOU", 3) == 0){
                lock_mem_segment(sharedmem, sizeof(sharestruct));
                sharedmem->spielernumer = atoi(split_buffer[2]);//mitspielerzahl evtl in header datei falls die variable ausserhalb dieser funktion gebraucht wird
                char* Mitspielername = split_buffer[3]; //same here
                //da unbenutzte vars zu warnungen fuehren werden sie hier noch auf stdout ausgegeben
                printf("(performConnection) Mitspielernummer: %d\nMitspielername: %s\n", sharedmem->spielernumer, Mitspielername);
                unlock_mem_segment(sharedmem, sizeof(sharestruct));
            }
            else if(strncmp(split_buffer[1], "TOTAL", 5) ==0){
                int mitspielerzahl = atoi(split_buffer[2]);//auch hier variable global definiern falls man sie noch braucht
                printf("(performConnection) Mitspielerzahl: %d\n", mitspielerzahl);
                lock_mem_segment(&sharedmem->anzahl_spieler, sizeof(int));
                sharedmem->anzahl_spieler = mitspielerzahl;
                unlock_mem_segment(&sharedmem->anzahl_spieler, sizeof(int));
                printf("(performConnection) Andere Mitspieler:\n");


                int playerno =0; 


                while(true){//empfangen und auf stdout ausgeben bis ENDPLAYERS empfangen wird

                    ret = recieve_until_newline(servermessage, sock);
                    if(ret < 0){
                        perror("(performConnection) recv error: ");
                        return EXIT_FAILURE;
                    }
                    for(int i =0; i < 10; i++){
                        memset(split_buffer[i], 0, 20);
                    }
                    getWords(servermessage, split_buffer);

                    if(strncmp(split_buffer[0], "-", 1) == 0){
                        fprintf(stderr, "(performConnection) server error: %s\n",servermessage);
                        break;
                    }

                    if(strncmp(split_buffer[1], "ENDPLAYERS", 10) ==0){
                        printf("(performConnection) end of prolog!\n");
                        free(sendbuffer);
                        free(servermessage);
                        return EXIT_SUCCESS;
                    }
                    printf("(performConnection) Mitspielername: %s Mitspielernummer: %s Bereit: %s\n", split_buffer[2], split_buffer[1], split_buffer[3]);
                    //printf("Server: %s\n", servermessage);



                    lock_mem_segment(&sharedmem, sizeof(sharestruct));
                    strcpy(sharedmem->player[playerno].name, split_buffer[2]);
                    sharedmem->player[playerno].spielernumer = atoi(split_buffer[1]);
                    sharedmem->player[playerno].ready_flag = atoi(split_buffer[3]);

                    playerno +=1;
                    unlock_mem_segment(&sharedmem, sizeof(sharestruct));;




                }

            }

        }
        else{
            fprintf(stderr, "(performConnection) something weird happened");
            close(*sock);
            return EXIT_FAILURE;
        }






    }
    free(sendbuffer);
    //free(buffer);
    free(servermessage);

    return EXIT_SUCCESS;
}
