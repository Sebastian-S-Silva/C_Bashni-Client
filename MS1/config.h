#ifndef CONFIG_H_
#define CONFIG_H_

typedef struct{
    char hostname[128];
    int pNumber;
    char gameKind[128];
    char game_ID[14];
    int spielernummer;
} confVars;

int getParam(char* confName, confVars* conf_ptr);


#endif
