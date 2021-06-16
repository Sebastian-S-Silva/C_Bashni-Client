#ifndef BASHNICLIENT
#define BASHNICLIENT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <getopt.h> 
#include <stdbool.h>
#include <string.h>
#include "config.h"
#include "performConnection.h"
#include "connector.h"
#include <sys/types.h>
#include <sys/wait.h>
#include "sharedmemstruct.h"
#include <errno.h>
#include "sharedmem.h"
#include "spielverlauf.h"
#include "thinker.h"

//constants
//
//deprecated, now using client.conf file
#define GAMEKINDNAME "Bashni"
#define PORTNUMBER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"




#define VERSION "2.0\n" //muss 2.x sein

extern char game_id[13];//deprecated 
extern int spielnummer;//deprecated
extern configvars gameparameters; // configvars struct in config.h


#endif
