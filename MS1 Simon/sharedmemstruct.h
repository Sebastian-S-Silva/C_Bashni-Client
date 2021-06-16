#ifndef SHAREDMEMSTRUCT
#define SHAREDMEMSTRUCT


#include <unistd.h>
#include <stdbool.h>


/*
 * Maximale spielerzahl =64 damit sharedmem vor fork mit fixer groesse reserviert werden kann
 */
#define max_players 64

typedef struct{
    int spielernumer;
    char name[32];
    int ready_flag;
}spieler; // array mit mitspelern erst nach performconnection bekannt shared mem nach fork implementeier

#define piecelistmsglength 6 //4 chars for piecelocation + \n\0
typedef struct {
    char spielname[64];
    size_t shared_mem_size;
    unsigned int spielernumer;
    int anzahl_spieler;
    pid_t connector_pid;
    pid_t thinker_pid;
    spieler player[max_players];
    char piecelist[64][piecelistmsglength]; // durch geignetere struktur ersetzen
    bool sent_signal;
} sharestruct; //fixe gruesse: speicher kann vor fork reserviert werden


#endif
