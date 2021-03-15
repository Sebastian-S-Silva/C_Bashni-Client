#include"main.h"
#include"performConnection.h"
#include"config.h"
#include"connector.h"
#include"shMemStrct.h"
#include"shMem.h"

void removeSpace(char* splitPart){
    long unsigned int i = 0;
    if(splitPart == NULL) printf("String is empty!\n");
    while(i < strlen(splitPart)){
        if(splitPart[i] == ' ' || splitPart[i] == '\t' || splitPart[i] == '\n'){
            for(long unsigned int j = i; j<strlen(splitPart); j++){
                splitPart[j] = splitPart[j+1];
            }
        }
        else i++;
    }
}

int getParam(char* confName, confVars* conf_ptr){
	FILE *fp = fopen(confName, "r");
	if(fp == NULL){
		perror("Failed to open configuration file");
	}

	
	char holder[100];
	char split[3][100];
	char varName[100];
	char varValue[100];

	char* stat;

	while(true){
		stat = fgets(holder, 101, fp);
		if(stat != NULL){
			removeSpace(holder);
			splitStr(holder, split, "=");
			strcpy(varName, split[0]);
			strcpy(varValue, split[1]);

			if (strcmp(varName, "hostname") == 0){
				strcpy(conf_ptr->hostname, varValue);
			}else if (strcmp(varName, "portnumber") == 0){
				conf_ptr->pNumber = atoi(varValue);
			}else if (strcmp(varName, "gamekindname") == 0){
				if(strcmp(varValue, "Bashni") != 0){
					return -1;
				}
				strcpy(conf_ptr->gameKind, varValue);
			}else{
				printf("The variable '%s' doesn't have a corresponding place in the gameparameter struct.\n", holder);
			}

			memset(holder, 0, 100*sizeof(char));
			for(int i = 0; i < 3; i++){
	            memset(split[i], 0, 100);
	        }
	        memset(varName, 0, 100*sizeof(char));
	        memset(varValue, 0, 100*sizeof(char));
    	}else{
    		if(ferror(fp)){
    			perror("Error reading config file. Exiting.");
    			break;
    		}
    		break;
    	}
	}
	
	fclose(fp);

	return 1;
}