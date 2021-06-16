#ifndef SPIELVERLAUF
#define SPIELVERLAUF
#include "sharedmemstruct.h"
#include <signal.h>
#include <sys/select.h>

int read_from_server(sharestruct* sharedmem, int* socket, int* fields);







#endif
