#ifndef BOARD_REP
#define BOARD_REP

#include <stdlib.h>
#include <stdio.h>
#include "bashniclient.h"

/*Board is stored as a size 32 array of doubly linked lists
 * for faster move and jump generation bitboards are
 * generated from array
 */



//node struct for dubly linked list
//next pointer of last node is NULL 
//prev pointer of head is NULL
 struct node {
    char type;//b,w,B,W
    char  val; 
    struct node* next;
    struct node* prev; 

};
typedef struct node node_t;

node_t* pos_node_from_bf(uint32_t bf, node_t* node);
uint32_t bf_from_pos_node(node_t* node);
//DOUBLY LINKED LIST FUNCTIONS
node_t* get_head(node_t* some_node);
node_t* get_tail(node_t* some_node);
node_t* create_new_list(char type, char val);
node_t* append_new_to_list(char type, char val,  node_t* tail);
node_t* append_to_list(node_t* node, node_t* tail);
void create_board_from_piecelist(sharestruct* sharedmem, node_t* board);
void free_list(node_t* head);
void printlist(node_t* some_node);
int count_list_elements(node_t* head);
//DOUBLY LINKED LIST FUNCTIONS END





#endif
