#include "bashniclient.h"
//#include "performConnection.h"
//#include "config.h"

char game_id[13];
int spielnummer;
configvars gameparameters;

void tooltip(){ //TODO update tooltip
    perror("to run program execute:\n ./sysprak-client -g <GAME-ID> -p <{1,2}>\n");
}

int main(int argc, char **argv){

    
    int ret=0;
    configvars* param_ptr = &gameparameters; // pointer to struct with parameters from config file


    //reading default config
    param_ptr = read_gameparameters("client.conf", param_ptr);
    if(param_ptr == NULL ){
        fprintf(stderr, "error while reading default config file!");
    }

    // command line parameters overwrite default, should they exist
    while((ret=getopt(argc, argv, "g:p:k:")) != -1){
        switch(ret){
            case 'g':
                param_ptr->game_ID[0] = '\0';
                if(strlen(optarg) != 13){
                    fprintf(stderr, "Game-ID muss 13 stellig sein\n");
                    return EXIT_FAILURE;
                }
                
                strncpy(param_ptr->game_ID,  optarg, 13); //fuer 13 stellige game id
                break;
            case 'p':
                printf("game number detected\n");
                //falls spielnummer in kommandozeilen param ueberschreibe default
                param_ptr->game_number = atoi(optarg);
                if (!(param_ptr->game_number ==1 || param_ptr->game_number==2)){
                        tooltip();//TODO change tooltip 
                        return EXIT_FAILURE;
                }
                break;
            case 'k':
                //eigenes config file ueberschreibt alles
                param_ptr = read_gameparameters(optarg, param_ptr);
                if (param_ptr == NULL){
                    printf("something went wrong trying fallback to default\n");
                    param_ptr = read_gameparameters("client.conf", param_ptr);
                }
                break;
            default:
                break;
        }

    }

//    printf("game_id: %s\n", gakme_id);
 //   printf("spielnummer: %i\n", spielnummer);
    if(strlen(param_ptr->game_ID) != 13){
        fprintf(stderr, "invalid game ID(13 characters)\n");
        return EXIT_FAILURE;
    }
    
    //create shared mem segment for sharestruct(haredmem struct.h
    sharestruct* sharedmem = create_shared_memory_segment(sizeof(sharestruct));
    strcpy(sharedmem->spielname, param_ptr->gamename);
    sharedmem->spielernumer = param_ptr->game_number;
    sharedmem->shared_mem_size = sizeof(sharestruct);

    
    /*pipe zur spielzug uebermittlung von thinker zu connector
     */
    int fields[2];
    if(pipe(fields) <0 ){
        perror("pipe error:");
        return EXIT_FAILURE;
    }


    
    /*programm in zwei prozesse spalten
     */
    int pid = fork();
    if(pid <0){
        fprintf(stderr, "fork failed\n");
        return EXIT_FAILURE;
    }
    if(pid == 0){
        printf("created connector process\n");

        close(fields[1]); //close write end of pipe

        //write connector pid to shared mem
        ret = lock_mem_segment(&sharedmem->connector_pid, sizeof(int));
        sharedmem->connector_pid = getpid();
        unlock_mem_segment(&sharedmem->connector_pid, sizeof(int));



        ret = connector(param_ptr, sharedmem, fields);
        if(ret != EXIT_SUCCESS){
            fprintf(stderr, "connector failed\n");
            close(fields[0]);
            return EXIT_FAILURE;
        }
        else{
            printf("connector finished sucessfully\n");
        }
        printf("connector is dyig\n");
        close(fields[0]);
        return EXIT_SUCCESS;
    }
    else{
        printf("created thinker process\n");

        close(fields[0]); // close read end of pipe

        int status;
        sleep(1);
        //read connector pid form shared mem (just a test)
        lock_mem_segment(&sharedmem->connector_pid, sizeof(int));
        printf("connector pid: %d\n", sharedmem->connector_pid);
        unlock_mem_segment(&sharedmem->connector_pid, sizeof(int));


        //write thinker pid to shared memory
        lock_mem_segment(&sharedmem->thinker_pid, sizeof(int));
        sharedmem->thinker_pid = getpid();
        unlock_mem_segment(&sharedmem->thinker_pid, sizeof(int));


        //read other players form shared mem
        printf("andere mitspieler(thinkerr):\n");

        //print playes form other process to test shared memory  
        lock_mem_segment(&sharedmem, sizeof(sharestruct));
        printf("anzahl spielr in shared mem: %d\n", sharedmem->anzahl_spieler);

        for(int i= 0; i<sharedmem->anzahl_spieler-1; i++){
            printf("playername %s, playerno %d ready: %d\n", sharedmem->player[i].name, sharedmem->player[i].spielernumer, sharedmem->player[i].ready_flag);
        }
        unlock_mem_segment(sharedmem, sizeof(sharestruct));
        printf("run_thinker\n");
        run_thinker(sharedmem, fields);

        

        //insert thinker
        wait(&status);//TODO error handeling
        printf("(main) child terminated with status: %d\n", status);
        munmap(&sharedmem, sizeof(sharestruct));// unmap shared memory after completion
        close(fields[1]);
    }
    

   
    




    
    return EXIT_SUCCESS;
    }
