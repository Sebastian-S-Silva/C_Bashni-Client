#ifndef MAIN_H_
#define MAIN_H_

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h> 

#define GAMEKINDNAME "Bashni"
#define PORTNUMBER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"
#define VERSION "2.0\n"

extern char game_ID[13];
extern int spielernummer;

extern int sockC;

#endif