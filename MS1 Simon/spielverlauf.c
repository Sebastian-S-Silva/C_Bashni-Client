#include "bashniclient.h"



/* send_signal(sharedmem) sends SIGUSR1 to parent(thinker) and sets sent_signal flag to true
 *
 */
int send_signal(sharestruct* sharedmem){
    lock_mem_segment(&sharedmem, sizeof(sharestruct));
    printf("(send_signal) sending signal\n");
    sharedmem->sent_signal = true;
    printf("(send_signa) send_signal is now %d\n", sharedmem->sent_signal);
    unlock_mem_segment(&sharedmem, sizeof(sharestruct));
    int ret = kill(getppid(), SIGUSR1);
    if(ret <0 ){
        perror("(send_signal) failed to send signal to parent: ");
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;


}
int send_sigterm(sharestruct* sharedmem){
    lock_mem_segment(&sharedmem, sizeof(sharestruct));
    sharedmem->sent_signal = true;
    unlock_mem_segment(&sharedmem, sizeof(sharestruct));
    int ret = kill(getppid(), SIGTERM);
    if(ret<0){
        perror("failed to send SIGTERM to parent\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}



/*
 * wait handler sends OKWAIT to server
 * return value is return value of send, which is 7(bytes sent) on success
 */

int wait_handler(int* socket){
    printf("(wait_handler)\n");
    int ret = send(*socket, "OKWAIT\n", 7, 0);
    return ret; 
}

/*gameover_handler handles gameover sequence
 * on sucess it returns EXIT_SUCCESS otherwise EXIT_FAILURE or integer
 *
 *
 */

    
int gameover_handler(sharestruct* sharedmem, int* socket){
    printf("gameover_handler\n");
    int ret;
    char* servermessage = malloc(sizeof(char)*BUFFERSIZE);
    if(servermessage == NULL){
        fprintf(stderr, "could not malloc (gameover_handler)\n");
        return EXIT_FAILURE;
    }
    char split_buffer[10][100];
    while(1){
        ret = recieve_until_newline(servermessage, socket);
        if(ret < 0){
            fprintf(stderr, "could not recieve message (gameover_handler)\n");
            free(servermessage);
            return EXIT_FAILURE;
        }
        ret = getWords(servermessage, split_buffer);
        //add error handeling
        if(split_buffer[0][0] == '-'){
            fprintf(stderr, "serever error (gameover_handler): %s\n", split_buffer[1]);
            free(servermessage);
            return EXIT_FAILURE;
        }
        else if (strncmp(split_buffer[1], "PIECESLIST", 10) ==0){
            int piececounter =0;
            while(1){

                ret = recieve_until_newline(servermessage, socket);
                if(ret < 0){
                    fprintf(stderr, "could not recieve message (gameover_handler)\n");
                    free(servermessage);
                    return EXIT_FAILURE;
                }
                ret = getWords(servermessage, split_buffer);
                //add error 
                //

                if(split_buffer[0][0] == '-'){
                    fprintf(stderr, "serever error (gameover_handler): %s\n", split_buffer[1]);
                    free(servermessage);
                    return EXIT_FAILURE;

                }
                if(strncmp(split_buffer[1], "ENDPIECESLIST", 13)==0){
                    //send signal to parent
                    //if(send_signal(sharedmem)==EXIT_FAILURE) return EXIT_FAILURE;
                    break;
                }
                lock_mem_segment(&sharedmem, sizeof(sharestruct));
                strncpy(sharedmem->piecelist[piececounter], split_buffer[1], piecelistmsglength);
                printf("added %s to piecelist\n", sharedmem->piecelist[piececounter]); 
                unlock_mem_segment(&sharedmem, sizeof(sharestruct));
                piececounter++;

            }
        }
        else if(strncmp(split_buffer[1], "PLAYER0WON", 10)==0){
            if(strncmp(split_buffer[2], "Yes", 3)==0){
                printf("Player 0 Won\n");
                //break;
            }
            
        }
        else if(strncmp(split_buffer[1], "PLAYER1WON", 10)==0){
            if(strncmp(split_buffer[2], "Yes", 3)==0){
                printf("Player 1 Won\n");
                //break;
            }
        }
        else if(strncmp(split_buffer[1], "QUIT", 4)==0){
            printf("sending sigterm");
            if(send_sigterm(sharedmem)==EXIT_FAILURE) return EXIT_FAILURE;
            free(servermessage);
            return EXIT_SUCCESS;
        }
    }
    





            return ret; 
}


/*move handler handles move signal vorm server
 * it recieves peices list and stores ot in shared mem
 * after ENDPIECESLIST, THINKING is sent to server
 * returns EXIT_SUCCESS on sucess and EXIT_FAILURE or other integer value (in case of send error)  on failure
 */
int move_handler(sharestruct* sharedmem, int* socket){
    printf("(move_handler) now running\n");
    int ret;
    char* servermessage = malloc(sizeof(char)*BUFFERSIZE);
    char split_buffer[10][100];
    ret = recieve_until_newline(servermessage, socket);
    if(ret < 0){
        perror("recieve error(move_handler): ");
        free(servermessage);
        return ret;
    }
    ret = getWords(servermessage, split_buffer);
    
    if(split_buffer[0][0] == '-'){
        fprintf(stderr, "(move_handler) server error: %s\n", split_buffer[1]);
    }
   // printf("%s", servermessage);
    if(strncmp(split_buffer[1], "MOVE", 4)==0){
            move_handler(sharedmem, socket);
            }
    else if(strncmp(split_buffer[1], "PIECESLIST", 10)==0){//recieve pieces until ENDPIECES
        printf("(move handler) got piecelist!\n");
        int piececounter = 0;
        while(1){

            ret = recieve_until_newline(servermessage, socket);
            //printf("(move_handler) server sent: %s\n", servermessage);

            if(ret < 0){ perror("recieve error(move_handler): "); 
                free(servermessage);
                return ret;
            }
            ret =getWords(servermessage, split_buffer);
            if(split_buffer[0][0] == '-'){
                fprintf(stderr, "(move_handler) server error: %s\n", split_buffer[1]);
                free(servermessage);
                return EXIT_FAILURE;
            }
            else if(strncmp(split_buffer[1], "ENDPIECESLIST", 13) ==0){
                //send signal to thinker
                if(send_signal(sharedmem)==EXIT_FAILURE) {
                    printf("(move_handler) send_signal failed\n"); 
                    return EXIT_FAILURE;
                }

                //after thinker is done send play to server


                ret=send(*socket, "THINKING\n", 9, 0);
                if(ret < 0){
                    perror("send error move_handler: ");
                    free(servermessage);
                    return ret;
                }
                ret = recieve_until_newline(servermessage, socket);
                if(ret<0){
                    perror("(move_handler) recv error");
                    return ret;
                }
                printf("(move_handler) print okthink: %s\n", servermessage);

                break;
            } 
            else{
                lock_mem_segment(&sharedmem, sizeof(sharestruct));
                strncpy(sharedmem->piecelist[piececounter], split_buffer[1],piecelistmsglength );
                //printf("(move_handler) added %s to piecelist[%d]\n", sharedmem->piecelist[piececounter], piececounter);
                unlock_mem_segment(&sharedmem, sizeof(sharestruct));
                piececounter++;
            }
            }



        }
    else if (strncmp(split_buffer[1], "WAIT", 4)==0){
      wait_handler(socket);
      free(servermessage);
      return EXIT_SUCCESS;

    }
    else if(strncmp(split_buffer[1], "GAMEOVER", 8) == 0){
        printf("calling gameover_handler\n");
        if( gameover_handler(sharedmem, socket) != EXIT_SUCCESS){
            printf("gameover_handler failed\n");
        }
        
        free(servermessage);
        return EXIT_SUCCESS;
    }
    
    else{
        fprintf(stderr, "(move_handle) recieved random garbage:\n%s", servermessage);
        //garbage
        //
    
    }


    
    free(servermessage);
    return EXIT_SUCCESS;
}






/*main game loop handles MOVE WAIT or GAMEOVER sequences
 */

int read_from_server(sharestruct* sharedmem, int* socket, int* fields){

    char* servermessage = malloc(sizeof(char)*BUFFERSIZE);
    int ret;
    char split_buffer[10][100];


    /*Structs for select
     */

    struct timeval select_timeout;
    select_timeout.tv_sec = 10;
    select_timeout.tv_usec = 100;

    fd_set select_fps;
    FD_ZERO(&select_fps);
    FD_SET(*socket, &select_fps);
    FD_SET(*fields, &select_fps);





    while(1){

        fd_set ready_fps = select_fps;
        /*ADD SELECT HERE AND TO ASYNCHRONOUSLY READ FORM PIPE AND SOCKET
        */
        ret = select(FD_SETSIZE, &ready_fps, NULL, NULL, &select_timeout); 
        if(ret < 0){
            perror("(run_thinker) select error:");
            return EXIT_FAILURE;
        }
        else if (ret == 0){
            fprintf(stderr, "(run_thinker) TIMEOUT!\n");
            return EXIT_FAILURE;
        }
        else{
            if(FD_ISSET(*socket, &ready_fps)){
                printf("socket is ready to read\n");

                ret = recieve_until_newline(servermessage, socket);//function in performconnection.c
                if(ret<0){
                    perror("(read_from_serve) recieve error: ");
                    free(servermessage);
                    return EXIT_FAILURE;
                }
                ret =getWords(servermessage, split_buffer);//function in performconnection.c
                if(split_buffer[0][0] == '-'){
                    fprintf(stderr, "(read_from_server) server error: %s\n", servermessage);
                    free(servermessage);
                    return EXIT_FAILURE;
                }
                else{
                    if(strncmp(split_buffer[1], "GAMEOVER", 7) == 0){
                        ret = gameover_handler(sharedmem, socket);
                        if(ret!=EXIT_SUCCESS){
                            free(servermessage);
                            fprintf(stderr, "(read_from_server) gameover_handler failed\n");
                            return EXIT_FAILURE;
                        }
                    }
                    else if(strncmp(split_buffer[1], "MOVE", 4) == 0){
                        ret=move_handler(sharedmem, socket);
                        if(ret!=EXIT_SUCCESS){
                            free(servermessage);
                            fprintf(stderr, "(read_from_server) move_handler failed\n");
                            return EXIT_FAILURE;
                        }

                    }
                    else if(strncmp(split_buffer[1], "WAIT", 4) == 0){
                        ret=wait_handler(socket);
                        if(ret!=7){
                            free(servermessage);
                            fprintf(stderr, "(read_from_server)wait_handler failed\n");
                            return EXIT_FAILURE;
                        }
                    }
                    else{
                        printf("%s\n", servermessage);
                    }
                }
            }
            if(FD_ISSET(*fields, &ready_fps)){
                /*read move form pipe and send to server
                */

                printf("pipe is ready to read\n");
                char move[MAX_MOVE_LEN];
                char sendbuffer[BUFFERSIZE] = "sdasdasd";
                //memset(move, 0, MAX_MOVE_LEN);
                ret =read(fields[0], &move, MAX_MOVE_LEN);
                if(ret <0){
                   perror("(read_from_server) pipe read error:");
                   return EXIT_FAILURE;
                }
                sleep(1);
                memset(sendbuffer, 0, BUFFERSIZE); 
                strncpy(sendbuffer, "PLAY ", 6);
                strcat(sendbuffer, move);

                //sometimes random garbage is at tehe end of a move this is to remove it
                for(unsigned long long i =0; i < strlen(sendbuffer); i++){
                    if(sendbuffer[i] == '\n'){
                        sendbuffer[i+1] = '\0';
                        break;
                    }
                }
             

                printf("(read_from_server) Got move form thinker: %s\n", move);
                printf("sending to server: %s", sendbuffer);
                if(send(*socket, sendbuffer, strlen(sendbuffer), 0) < 0){
                    perror("(read_from_server) error while sending move:");
                    return EXIT_FAILURE;
                }

            }
        }
    }
    printf("connector loop broke\n");
    free(servermessage);
    return EXIT_SUCCESS;
} 
