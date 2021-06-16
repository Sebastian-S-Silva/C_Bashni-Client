#include"main.h"
#include"performConnection.h"
#include"config.h"
#include"connector.h"
#include"shMemStrct.h"
#include"shMem.h"
#include"signal.h"

int tst;
int End = 0;
char confName [512];

confVars gameConf;
confVars* conf_ptr = &gameConf;
int fd[2];
shareS* shMemP;
int shMId;

void thThink(int sig) {
    printf("Caught signal to calculate the next move! Sig: %d\n", sig);
    shMemP = (shareS *) atShm(shMId);
    char p_write[6];
    strcpy(p_write, "C3:D4");
    if ((write (fd[1], p_write, 6*(sizeof(char)))) == -1) {
    	printf("Failed to send move to Connector.");
    }

    shMemP->thFlag = 0;
    shmdt(shMemP);
}

void thGo(int sig) {
    printf("Caught signal to finish the client! Sig: %d\n", sig);
	End = 1;
}

int main(int argc, char* argv[]) {

	strcpy(confName, "client.conf");

	while((tst=getopt(argc, argv, "g:p:c:")) != -1){
        switch(tst){
        	case 'g':
                if(strlen(optarg) != 13){
                    perror("Invalid Game-ID length (13 digits)\nor player number.\nPlease try again in the form of: './sysprak-client -g <Game-ID> -p <{1,2}>'\n");
                    exit(0);
                }
                
                strncpy(conf_ptr -> game_ID, optarg, 13);
                break;
            case 'p':
                conf_ptr -> spielernummer = atoi(optarg);
                if (!(conf_ptr -> spielernummer == 1 || conf_ptr -> spielernummer == 2)){
                	perror("Invalid Game-ID length (13 digits)\nor player number.\nPlease try again in the form of: './sysprak-client -g <Game-ID> -p <{1,2}>'\n");
					exit(0);
                }
                break;
            case 'c':
            	if(optarg != NULL){
            		memset(confName, 0, sizeof(confName));
            		strcpy(confName, optarg);
            		strcat(confName, ".conf");
            	} else {
            	}
            	break;
            default:
                break;
        }

    }

	printf("Ihre Game-ID: %s\n", conf_ptr -> game_ID);
	printf("Ihre Spielernummer: %i\n", conf_ptr -> spielernummer);

	if ((tst = getParam(confName, conf_ptr)) == -1){
		printf("The game associated with this config file is not Bashni! Exiting..\n");
		return EXIT_FAILURE;
	} else {
		printf("Successfully read config file!\n");
	}

	shMId = crShm(sizeof(shMemP));

    if (pipe (fd) < 0) {
      perror ("Failed to open pipe!");
    } else {
        printf("Successfully opened pipe!\n");
    }
    
	int pId = fork();
    if(pId > 0){
		signal(SIGUSR1, thThink);
        signal(SIGUSR2, thGo);
        printf("Thinker ready!\n");

        close(fd[0]);

        while(End != 1) {
        	
        }

        shMemP = (shareS *) atShm(shMId);
        printf("From Thinker: %s\n", shMemP->gameName);
        printf("From Thinker: %i\n", shMemP->ownPlNum);
        printf("From Thinker: %i\n", shMemP->players);
        printf("From Thinker: %s\n", shMemP->pList[0]);
        printf("From Thinker: %s\n", shMemP->pList[23]);
        printf("From Thinker: %i\n", shMemP->pidCon);
        shmdt(shMemP);

    	close(fd[1]);

        printf("Thinker shutting down..\n");
    }else if(pId == 0){
        printf("Connector ready!\n");

        close(fd[1]);

        shMemP = (shareS *) atShm(shMId);
        shMemP->pidCon = getpid();
        shMemP->pidTh = getppid();
        shMemP->thFlag = 0;
        int thId = getppid();
        shmdt(shMemP);

        int sockC = clSock();
        tst = connectCl(conf_ptr, shMId, shMemP, sockC);
        if(tst != 0){
            fprintf(stderr, "Problem connecting to gameserver. Exiting..\n");
            close(fd[0]);
            kill(thId, SIGUSR2);
            return EXIT_FAILURE;
        }

        if ((tst = gameRoutine(fd, shMId, shMemP, sockC)) != 0) {
        	printf("Something caused the game to not run correctly. Disconnecting..\n");
        	close(fd[0]);
        	kill(thId, SIGUSR2);
        	return EXIT_FAILURE;
        }
        printf("Connector shutting down..\n");
        close(fd[0]);
        kill(thId, SIGUSR2);
        return EXIT_SUCCESS;
    } else {
		fprintf(stderr, "Failed to fork processes!\n");
        return EXIT_FAILURE;
    }

    shmctl(shMId, IPC_RMID, NULL);
	printf("Game finished successfully! Thanks for playing!\n");
	return EXIT_SUCCESS;
}

//valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./sysprak-client -g 214jqckslp7sq -p 1