#include "bashniclient.h"

/*
 * STRUCT CONTAINING READABLE PARAMETERS IN HEADERFILE config.h
 */



void remove_spaces_from_line(char* line){ //and tabs
    unsigned long int i = 0;
    if(line == NULL) printf("line is NULL!n");
    while(i < strlen(line)){
        if(line[i] == ' ' || line[i]== '\t' ){
            for(unsigned long int j = i; j<strlen(line); j++){
                line[j] = line[j+1];
            }
        }
        else i++;
    }
}

        


configvars* read_gameparameters(char *filename, configvars* ptr){
    /* datein oeffnen und zeile fuer zeile ainlesen bis eof
     * speicher fuer zeilen dynamisch und zeilen laenge dynamisch
     * dann alle leerzeichen entfernen 
     * ueberpruefen ob parameterName mit var in gameparameters struct uebereinstimmt
     * wert zuweisen 
     * alles andere ignorieren 
     * return struct
     */


    FILE *fp = fopen(filename, "r");
    if(fp == NULL){
        fprintf(stderr, "could not open file %s\n", filename);
        return NULL;
    }

    //read file line by line
    char chunk[128];
    size_t chunk_size = sizeof(chunk);
    size_t line_len = chunk_size;
    int current_line_index = 0;
    char **lines = (char**)malloc(sizeof(char*));
    if(lines==NULL){
        fprintf(stderr, "failed to allocate memory\n");
        return NULL;
    }
    lines[current_line_index] = malloc(chunk_size);
    if(lines[current_line_index] == NULL){
        fprintf(stderr, "allocation failure\n");
        
        return NULL;
    }


    lines[current_line_index][0] = '\0';

   
    while(fgets(chunk, chunk_size, fp) != NULL){

        size_t linebuffer_used = strlen(lines[current_line_index]);
        size_t chunk_used = strlen(chunk);

        if(line_len - linebuffer_used < chunk_used){
            line_len *=2;
            if((lines[current_line_index] = realloc(lines[current_line_index], line_len))== NULL){
                fprintf(stderr, "could not realloc memory");
                free(lines);
            }
        }

        strncat(lines[current_line_index], chunk, chunk_used);
        linebuffer_used += chunk_used;
        memset(chunk, 0, chunk_size);
        if(lines[current_line_index][linebuffer_used-1] == '\n') {
            current_line_index += 1;
            lines = realloc(lines, sizeof(char*)*(current_line_index+1));
            lines[current_line_index] = malloc(chunk_size);
            lines[current_line_index][0] = '\0';
        }
    }
    fclose(fp);

    /*remove spaces from lines 
    */
    for(int i = 0; i<current_line_index; i++){
        remove_spaces_from_line(lines[i]);
    }


    //printf("current_line_index is: %d\n", current_line_index);

    //printf("curren line indexis: %d\n", current_line_index);;



    //remove comments at end of line
    for(int i =0; i < current_line_index-1; i++){
        for(unsigned long int j=0;j<strlen(lines[i]); j++){

            if(lines[i][j] == '/' && lines[i][j+1] == '/'){
                lines[i][j] = '\n';
                lines[i][j+1] = '\0';
            }
        }
    }

    /*check if var name is in configvars and assign value
    */
    for(int i = 0; i < current_line_index; i++){
        char varname[128];
        char varvalue[128];
        bool name = true;
        //printf("line %d in config file is: %s\n", i, lines[i]);
        //printf("length of curren line is %lu\n", strlen(lines[i]));
        int istgleich_pos =0;
        for(unsigned long int j= 0; j < strlen(lines[i]); j++){
            if(lines[i][j] == '='){
                name = false;
                varname[j] = '\0';
                istgleich_pos = j+1;
                continue;
            }
            if(name){
                if(j >= 128) fprintf(stderr, "use shorter variable names\n");
                varname[j] = lines[i][j];
            }
            if(!name){
                if(j>= 128) fprintf(stderr, "value to long\n");
                varvalue[j-istgleich_pos] = lines[i][j];
                if(varvalue[j-istgleich_pos] == '\n') varvalue[j-istgleich_pos] = '\0';
             //  printf("j is: %lu and varvalue[%lu] is %c\n", j, j-istgleich_pos, varvalue[j-istgleich_pos]);
            }
        }
        //printf("varname is: %s varvalue is %s\n", varname, varvalue);
        if(strncmp(varname, "hostname", 8) == 0){
            strncpy(ptr -> hostname, varvalue, strlen(varvalue));
           // printf("read hostname: %s from config file\n", ptr->hostname);
        }
        else if(strncmp(varname, "portnumber", 10) == 0){
            ptr -> portnumber = atoi(varvalue);
            if(ptr -> portnumber == 0){
                fprintf(stderr, "portnumber is invalid\n");
            }
            //printf("read portnumber: %d from config file\n", ptr -> portnumber);
        }
        else if(strncmp(varname, "gamename", 8) == 0){
            strncpy(ptr -> gamename, varvalue, strlen(varvalue));
            //printf("read gamename from config file: %s\n", ptr->gamename);
        }
        else if(strncmp(varname, "game_ID", 7)==0){
            strncpy(ptr->game_ID, varvalue, strlen(varvalue));
           // printf("read game_id form config file: %s\n", ptr->game_ID);
        }
        else if(strncmp(varname, "game_number", 11)==0){
            ptr->game_number = atoi(varvalue);
            //printf("read game number from config file: %d\n",ptr->game_number);
        }
        else{
            //fprintf(stderr, "cant recognice config file entry ... skipping\n");
            continue;
        }

    }


    for(int i = 0; i<=current_line_index; i++){
        free(lines[i]);
    }
    free(lines);
    return ptr;
}

