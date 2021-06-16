#include "bashniclient.h"


int connector(configvars* param_pointer, sharestruct* sharedmem, int* fields){
    int returnval; 

    printf("(connector) Connection data:\n");
    printf("(connector) Hostname: %s\n",  param_pointer->hostname);
    printf("(connector) Portnumber %d\n",  param_pointer->portnumber);
    printf("(connector) Gamename: %s\n",  param_pointer->gamename);
    printf("(connector) Game-ID: %s\n",  param_pointer->game_ID);
    printf("(connector) Trying to connect to game server...\n");

    clientSocket = get_socket(param_pointer);
    if (clientSocket < 0){
        fprintf(stderr, "(connector) client socket failed!\n");
        return EXIT_FAILURE;
    }
    returnval = performConnection(&clientSocket, param_pointer, sharedmem);
    if(returnval == EXIT_FAILURE){
        fprintf(stderr, "(connector) performConnection failed\n");
    }

    returnval = read_from_server(sharedmem, &clientSocket, fields);
    if(returnval != EXIT_SUCCESS){
        fprintf(stderr, "(connector) read_from_server failed\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
