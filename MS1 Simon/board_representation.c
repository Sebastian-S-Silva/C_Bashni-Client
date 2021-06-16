#include "board_representation.h"

char trans_table[32][2] = { "B8", "D8", "F8", "H8", 
                           "A7", "C7", "E7", "G7", 
                            "B6", "D6", "F6", "H6", 
                           "A5", "C5", "E5", "G5", 
                            "B4", "D4", "F4", "H4", 
                           "A3", "C3", "E3", "G3", 
                            "B2", "D2", "F2", "H2", 
                           "A1", "C1", "E1","G1" };


/*creates chess notation from bitfield and stores it in a node_t
 * e.g. A8 := node_t->type = A node_t->val = 8A
 * only one bit should be set
 */ 
node_t* pos_node_from_bf(uint32_t bf, node_t* node){
    char type = 0;
    char val = 0;
    for(int i = 0; i<32 ; i++){
        if(IS_SET(bf, i)){
            type = trans_table[i][0];
            val = trans_table[i][1];
            break;
        }
    }
    node->type = type;
    node->val = val;
//    printf("(pos_node_from_bf) type is: %c, val is %c\n", node->type, node->val);
    return node;
}

// creates bitfield from chess notation node
uint32_t bf_from_pos_node(node_t* node){
    uint32_t bf =0;
    for(int i = 0; i < 32 ; i++){
        if((node->type == trans_table[i][0]) && (node->val == trans_table[i][1])){
                SET_BIT(bf, i);
                break;
                }
    }
    if(bf == 0) fprintf(stderr, "(bf_from_pos_node) bitfield is ZERO!!\n");
    return bf;
}

node_t* deepcopy_board(node_t* original_board, node_t* copy){
    for(int i =0; i<32 ; i++){
        if((&original_board[i] == NULL) | (&copy[i] == NULL)){
            fprintf(stderr, "(deepcopy_board) invalid input expecting two node_t[32] arrays\n");
            return NULL;
        }
        node_t* orig_head = get_head(&original_board[i]);
        copy[i] = *create_new_list(orig_head->type, orig_head->val);
        node_t* cpy_head = &copy[i];
        while(orig_head != NULL){
            orig_head = orig_head->next;
            if(orig_head == NULL) break;
            cpy_head = append_new_to_list(orig_head->type, orig_head->val, cpy_head);
        }

    }
return copy;
}









/*
 * 
 * DOUBLY LINKED LIST FUNCTIONS
 *
 */


node_t* create_new_list(char type, char val){
    node_t *new_node = malloc(sizeof(node_t));
    new_node->next = NULL;
    new_node->prev = NULL;
    new_node->type = type;
    new_node->val  = val;
    return new_node;
}


node_t* get_head(node_t* some_node){
    node_t* temp = some_node;
    if(temp == NULL){
        fprintf(stderr, "(get head) got null pointer\n");
        return NULL;
    }
    while(temp->prev != NULL){
        temp = temp->prev;
    }
    return temp;
}

node_t* get_tail(node_t* some_node){
    node_t* temp = some_node;
    if(temp == NULL){
        fprintf(stderr, "(get_tail) got NULL pointer!\n");
        return NULL;
    }
    while(temp->next != NULL){
        temp = temp->next;
    }
    return temp;
}

// created and appends node to list
// PUSH
node_t* append_new_to_list(char type,char val,  node_t* tail){
    if(tail->type == ' '){
        tail->type = type;
        return tail;
    }
    node_t* new_node =malloc(sizeof(node_t));
    new_node->type = type;
    tail->next = new_node;
    new_node->val = val;
    new_node->prev = tail;
    new_node->next = NULL;
    return new_node;
}

//appends node to list
//returns pointer to new tail (node
node_t* append_to_list(node_t* node, node_t* tail){
    tail->next = node;
    node->prev = tail;
    
    return node;
}


// cuts tail and retruns pointer
// POP
node_t* pop_tail(node_t* tail){
    node_t* temp = tail->prev;
    temp->next = NULL;
    tail->prev = NULL;
    return tail;
}





void free_list(node_t* head){
    node_t* temp = head;
    node_t* next = head->next;
    while(temp != NULL){
        free(temp);
        temp = next;
        next = temp->next;
    }
}



/*print doubly linked list form head to tail
 */
void printlist(node_t *some_node){
    node_t* temp = get_head(some_node);
    while(temp != NULL){
        printf("%c - ", temp->type);
        temp = temp->next;
    }
    printf("\n");
}

/*
 *
 * DOUBLY LINKED LIST FUNCTIONS END
 *
 */

// creates board from piecelist board should be node_t arr[32]
void create_board_from_piecelist(sharestruct* sharedmem, node_t* board){
    lock_mem_segment(sharedmem, sizeof(sharestruct));
    for(int i = 0; i<64; i++){
        for(int j = 0; j < 32; j++){
            if((sharedmem->piecelist[i][2] == trans_table[j][0]) && (sharedmem->piecelist[i][3] == trans_table[j][1])) {
                node_t* temp = get_tail(&board[j]);
                node_t* new_node = append_new_to_list(sharedmem->piecelist[i][0],0,  temp);
                printf("%d ", j);
                printlist(new_node);
            }
        }
    }
}





int count_list_elements(node_t* head){
    int i = 0;
    node_t *temp = head;
    while(temp != NULL){
        i++;
        temp = temp->next;
    }
    return i;
}






