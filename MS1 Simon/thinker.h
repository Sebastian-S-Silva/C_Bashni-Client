#ifndef THINKER
#define THINKER

#include "sharedmemstruct.h"
#include <ctype.h>
#include <stdint.h>


/*
 * Macros for bitfields
 */

#define SET_BIT(BF, N) BF |= ((uint32_t)0x1 <<N) 
#define CLR_BIT(BF, N) BF &= ~((uint32_t)0x1  <<N)
#define IS_SET(BF, N) ((BF >> N) & 0x1)

int run_thinker(sharestruct* sharedmem, int* fields);

#define MAX_MOVE_LEN 48 //increase to allow for multijumps
typedef struct{
    uint64_t from;
    uint64_t to;
    char move_chess_not[MAX_MOVE_LEN];
}move;//stores 2 bitfields one with pawn that moves and one with destination



#endif
