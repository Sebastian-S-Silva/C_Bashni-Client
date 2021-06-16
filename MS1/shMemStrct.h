#ifndef shMemStrct_H_
#define shMemStrct_H_

typedef struct{
    char gameName[101];
    int ownPlNum;
	int players;

	char pList[25][28];

    pid_t pidTh;
    pid_t pidCon;

    int thFlag;
} shareS;

#endif