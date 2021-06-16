#ifndef READ_CONFIG
#define READ_CONFIG


typedef struct{
    char hostname[255];
    unsigned short int portnumber;
    char gamename[255];
    char game_ID[14]; // 14 weil \0 am ende 
    int  game_number; 
    void* pointer_to_shred_mem;
} configvars;



void remove_spaces_from_line(char* line);
configvars* read_gameparameters(char* filename, configvars* ptr);




#endif
