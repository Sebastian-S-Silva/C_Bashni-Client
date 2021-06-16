#include "bashniclient.h"
#include "board_representation.h"



int got_signal = 0;
node_t board[32];
node_t moves[32];
void init_moves(){
    for(int i = 0 ; i< 32; i++){
        if(moves[i].next != NULL){
            node_t* temp = moves[i].next;
            while(temp != NULL){
                node_t* next_node = temp->next;
                free(temp);
                temp = next_node;
            }
        }

        moves[i].next = NULL;
        moves[i].prev = NULL;
        moves[i].val = 0;
        moves[i].type =0;
    }
}
bool jumped_tower = false;

//some bitfields
uint32_t white_on_top =0;
uint32_t black_on_top =0;
uint32_t kings = 0;
uint32_t towers = 0;
uint32_t blocked_space = 0;

/*
 *  01  02  03  04
 *05  06  07  08
 *  09  10  11  12
 *13  14  15  16 
 *  17  18  19  20
 *21  22  23  24 
 *  25  26  27  28
 *29  30  31  32 
 *
 */
uint32_t left_3_mask      = 0b00000000111000001110000011100000;
uint32_t right_3_mask     = 0b00000111000001110000011100000000;
uint32_t left_5_mask      = 0b00000111000001110000011100000111;
uint32_t right_5_mask     = 0b11100000111000001110000011100000;
//uint32_t right_bound_mask = 0b00010000000100000001000000010000;
//uint32_t left_bound_mask  = 0b10000000100000001000000010001000;
uint32_t right_bound_mask = 0b10001000100010001000100010001000;
uint32_t left_bound_mask =  0b00010001000100010001000100010001;
uint32_t bottom_bound_mask= 0b11110000000000000000000000000000;
uint32_t top_bound_mask   = 0b00000000000000000000000000001111;


void print_board_from_bitfields(uint32_t bf, char color){
    int row_num = 8;
    printf("  A B C D E F G H\n");

    for(int i = 0; i < 32; i++){
        if(i%4==0){
            printf("\n%d ", row_num);
            row_num--;
        } 
        if((IS_SET(bf, i) && (row_num%2 == 0))){
            printf("%c - ", color);
        }

        else if((IS_SET(bf, i) && (row_num%2 != 0))){
            printf(" - %c", color);
        }
        else if(row_num %2 !=0){
            printf(" - -");
        }
        else if(row_num%2 == 0){
            printf("- - ");
        }
    }
    printf("\n");
}

uint32_t get_white_movers(){
    uint32_t not_occupied = ~(white_on_top | black_on_top);
    uint32_t white_kings  = white_on_top&kings;
    uint32_t movers = (not_occupied << 4) & white_on_top;
    movers |= ((not_occupied&left_3_mask)<<3) & white_on_top;
    movers |= ((not_occupied&left_5_mask)<<5) & white_on_top;
    if(white_kings){
        movers |= (not_occupied >> 4) & white_kings;
        movers |= ((not_occupied & right_3_mask) >> 3) & white_kings;
        movers |= ((not_occupied & right_5_mask) >> 5) & white_kings;
    }
    return movers;

}


uint32_t get_black_movers(){ //TODO bugfix

    uint32_t not_occupied = ~(black_on_top | black_on_top);
    uint32_t black_kings  = black_on_top&kings;
    uint32_t movers = (not_occupied >> 4) & black_on_top;
    movers |= ((not_occupied&right_3_mask)>>3) & black_on_top;
    movers |= ((not_occupied&right_5_mask)>>5) & black_on_top;
    if(black_kings){
        movers |= (not_occupied >> 4) & black_kings;
        movers |= ((not_occupied & left_3_mask) << 3) & black_kings;
        movers |= ((not_occupied & left_5_mask) << 5) & black_kings;
    }
    return movers;

}

uint32_t get_white_jumpers(){
    uint32_t not_occupied = ~(white_on_top | black_on_top);
    uint32_t white_kings = white_on_top&kings;
    uint32_t jumpers = 0;
    uint32_t temp = (not_occupied << 4) & black_on_top;

    jumpers |= (((temp & left_3_mask) << 3) | ((temp & left_5_mask) << 5));
    temp = (((not_occupied & left_3_mask) << 3) | ((not_occupied & left_5_mask) << 5)) & black_on_top;
    jumpers |= (temp << 4);
    jumpers &= white_on_top;
    temp = (not_occupied >> 4) & black_on_top;
    if(temp){
        jumpers |= (((temp & right_3_mask) >> 3) | ((temp & right_5_mask) >> 5)) & white_on_top;
    }
    temp = (((not_occupied & right_3_mask) >> 3) | ((not_occupied & right_5_mask) >> 5)) & black_on_top;
    //jumpers |= (temp >> 4) & white_on_top;
    if(temp) jumpers |= (temp >> 4) & white_on_top;
    if(white_kings){
        printf("found white kings\n");
        temp = (not_occupied >> 4) & black_on_top;
        print_board_from_bitfields(temp, 'T');
        if(temp& right_3_mask){
            printf("temp&right_3_mask\n");
        // TODO maybe (temp&(~(not_occupied)))
            if(temp&white_kings) printf("temp&white_kings\n");
            if(temp&right_bound_mask) printf("temp&right_bound_mask\n");
            if(temp&top_bound_mask) printf("temp&top_bound_mask\n");
            while(((temp&white_kings)/* | (temp&right_bound_mask) | (temp & top_bound_mask)*/ )==0){
                printf("while1\n");

                if(temp == 0) break;

                if(temp&right_3_mask) temp = temp >>3;
                else temp = temp >>4;

            }
            temp = temp & white_kings;
            if(temp) jumpers |= temp;
        }
        else if(temp & right_5_mask){
            printf("temp&right_5_mask\n");
            while(((temp&white_kings)/* | (temp & left_bound_mask) | (temp & top_bound_mask)*/)==0){
                if(temp == 0) break;
                if(temp&right_5_mask) temp = temp >> 5;
                else temp = temp >>4;
            }
            temp = temp & white_kings;
            if(temp) jumpers |= temp;
        }
        temp = ((not_occupied & right_3_mask) >>3) & black_on_top;
        while(((temp & white_kings)/* | (temp & right_bound_mask) | (temp & top_bound_mask)*/)==0){
            if(temp == 0) break;
            if(temp&right_3_mask) temp = temp >>3;
            else temp = temp >>4;
        }
        temp = temp & white_kings;
        if(temp) jumpers |= temp;
        temp = ((not_occupied & right_5_mask) >>5) & black_on_top;
        while(((temp & white_kings) /*| (temp & left_bound_mask) | (temp & top_bound_mask)*/) ==0){
            if(temp == 0) break;
            if(temp&right_5_mask) temp = temp >> 5;
            else temp = temp >>4;
        }
        temp = temp & white_kings;
        if(temp) jumpers |= temp;

        //TODO left shifts


    }//white_kings


    return jumpers;
}

        
uint32_t get_black_jumpers(){
    uint32_t not_occupied = ~(white_on_top | black_on_top);
    uint32_t black_kings = black_on_top&kings;
    uint32_t jumpers = 0;
    uint32_t temp = (not_occupied >> 4) & white_on_top;
    jumpers |= (((temp & right_3_mask) >> 3) | ((temp & right_5_mask) >> 5));
    temp = (((not_occupied & right_3_mask) >> 3) | ((not_occupied & right_5_mask) >> 5)) & white_on_top;
    jumpers |= (temp >> 4);
    jumpers &= black_on_top;


    temp = (not_occupied << 4) & white_on_top;
    if(temp){
        jumpers |= (((temp & left_3_mask) << 3) | ((temp & left_5_mask) << 5)) & black_on_top;
    }
    temp = (((not_occupied & left_3_mask) << 3) | ((not_occupied & left_5_mask) << 5)) & white_on_top;
    if(temp) jumpers |= (temp << 4) & black_on_top;
    if(black_kings){
        //TODO same as white kings

    }

    return jumpers;
}
//king contains one king
//color is color of king
//already_jumped is to avoid already calculated jumps
//blocked space is 0 at start and contains all spaces that would cause a 180 deg jump in a consecuteive jump
//use blocked space as already jumped in consecutieve calls tu function
uint32_t get_royal_jump(uint32_t king, char color, uint32_t already_jumped){

    uint32_t dest = 0;
    uint32_t temp = 0;
    uint32_t not_occupied = (~(white_on_top | black_on_top | already_jumped | blocked_space));
    printf("(get_royal_jump) called with:\n");
    print_board_from_bitfields(king, 'K');
    print_board_from_bitfields(blocked_space, 'L');
   

    if(color == 'w'){
        /* if king &left 5 mask move diagonally to bottom right
        */
        temp = king & left_5_mask;
        if(temp){
            //left shift 5 manually so temp is on unoccupied space
            temp = temp << 5;
            blocked_space |= temp;
            //if temp is not occupied contionue diagonally until border or occupied space is reached
            if(temp&not_occupied) { //KING & L5MASK
                while(((temp & bottom_bound_mask) | (temp & right_bound_mask)) ==0 && (temp & not_occupied))
                {
                    if(temp&left_5_mask) temp = temp <<5;
                    else temp = temp <<4;
                    blocked_space |= temp;
                }
                //check if black piece can be jumped
                if(temp & black_on_top && (~((temp&right_bound_mask)|(temp & bottom_bound_mask)))){
                    if(temp&left_5_mask && (temp << 5)&not_occupied){
                        if(temp & towers) jumped_tower = true;
                        dest = temp <<5;
                        return dest;
                    }
                    else if((temp << 4)&not_occupied){
                        if(temp &towers) jumped_tower = true;
                        dest = temp <<4;
                        return dest;
                    }
                    else blocked_space = 0;

                }

            }
            //if temp << 5 is occupied check for regular jump
            else{
                //regular jump
                temp = temp & black_on_top;
                if(temp && (temp << 4)& not_occupied){
                    if(temp & towers) jumped_tower = true;
                    dest =temp << 4;
                    return dest;
                }
            }

            //move king to bottom left
            temp = king; //no need to and with L5M
            blocked_space |= temp;  
            temp = temp << 4;

            blocked_space |= temp;  
            //if temp is not occupied move diagonally to bottom left 
            if(temp & not_occupied){
                printf("entering buggy loop\n");
                while((((temp & bottom_bound_mask) | (temp & left_bound_mask))==0) && (temp & not_occupied)){
                    //sleep(1);
                    if(temp&left_3_mask){

                        print_board_from_bitfields(temp, 'T');
                        temp = temp << 3;
                        print_board_from_bitfields(temp, 'T');
                        printf ("temp & left_3_mask\n");
                    }
                    else{
                        printf("temp << 4\n");
                        temp = temp << 4;
                    }

                    blocked_space |= temp;  
                    if(temp & not_occupied) printf("not not_occupied\n");

                }
                printf("end of buggy loop\n");
                //check if black piece can be jumped 
                if(temp & black_on_top && (~((temp &left_bound_mask) | (temp & bottom_bound_mask)))){
                    printf("found black pawn\n");
                    if(temp&left_3_mask && (temp<<3)&not_occupied){
                        printf("and it is jumpable (<<3)\n");
                        if(temp&towers) jumped_tower = true;
                        dest = temp << 3;
                        return dest;
                    }
                    else if((temp <<4) & not_occupied){
                        if(temp & towers) jumped_tower = true;

                        printf("and it is jumpable (<<4)\n");
                        print_board_from_bitfields(temp<<4, 'D');
                        dest = temp <<4;
                        return dest;
                    }
                    else blocked_space = 0;
                }
            }
            else{ //normal jump
                temp = temp &black_on_top;
                if((temp&left_3_mask) && (temp <<3)&not_occupied){
                    if(temp & towers) jumped_tower = true;
                    dest = temp << 3;
                    return dest;
                }

            }


        }//END if(king & l5_mask)

            //if king is on l3 mask move diagonally to bottom left
        temp = king & left_3_mask;
        blocked_space |= temp;
        if(temp){
            // << 3 so temp temp is on unoccupied space
            temp = temp <<3;
            if(temp & not_occupied){
                //move diagonally
                while(((temp & bottom_bound_mask) | (temp & left_bound_mask)) == 0 && (temp & not_occupied)){
                    if(temp&left_3_mask) temp = temp <<3;
                    else temp = temp <<4;

                    blocked_space |= temp;  
                }
                //check if black piece can be jumped

                if(temp & black_on_top && (~((temp &left_bound_mask) | (temp & bottom_bound_mask)))){
                    if(temp&left_3_mask && (temp<<3)&not_occupied){
                        if(temp&towers) jumped_tower = true;
                        dest = temp << 3;
                        return dest;
                    }
                    else if((temp <<4) & not_occupied){
                        if(temp & towers) jumped_tower = true;
                        dest = temp <<4;
                        return dest;
                    }
                    else blocked_space = 0;
                }
            }
            else{

                //regular jump
                temp = temp & black_on_top;
                if((temp) && (temp <<4)&not_occupied){
                    if(temp & towers) jumped_tower = true;
                    dest = temp <<4;
                    return dest;
                }

            }
            //move king to bottom right
            temp = king << 4;
            blocked_space |= temp;
            if(temp & not_occupied){
                //move diagonally bottom right
                while(((temp & bottom_bound_mask) | (temp & right_bound_mask)) == 0 && (temp & not_occupied)){
                    if(temp & left_5_mask) temp = temp << 5;
                    else temp =temp << 4;

                    blocked_space |= temp;  
                }
                //check if black piece can be jumped
                if(temp & black_on_top && (~((temp & right_bound_mask) | (temp & bottom_bound_mask)))){
                    if((temp&left_5_mask) && (temp <<5)&not_occupied){

                        if(temp & towers) jumped_tower = true;
                        dest = temp << 5;
                        return dest;
                    }
                    else  if ((temp << 4) & not_occupied){

                        if(temp & towers) jumped_tower = true;
                        dest =temp <<4;
                        return dest;
                    }
                    else blocked_space = 0;


                }

            }
            else{
                //regular jump 
                temp = temp & black_on_top;
                if((temp & left_5_mask) && (temp << 4)&not_occupied){

                    if(temp & towers) jumped_tower = true;
                    dest = temp << 5;
                    return dest;
                }
            }
        }//end if(king & l_3mask)
        temp = king;
        blocked_space |= temp;
        if(temp) //if king is on neiteher l3 or l3 mask
        {//king << 4
            temp = temp << 4;
            blocked_space |= temp;
            if(temp & left_3_mask){
                if(temp &not_occupied){
                    //move diagonally (bottom left)
                    while(((temp &bottom_bound_mask) | (temp & left_bound_mask)) ==0 && (temp&not_occupied)){
                        if(temp&left_3_mask) temp = temp <<3;
                        else temp = temp <<4;
                        blocked_space |= temp;
                    }
                    //check if black piece can be jumped
                    if(temp & black_on_top && (~((temp & left_bound_mask) | (temp&bottom_bound_mask)))){
                        if(temp&left_3_mask && (temp <<3) &not_occupied){
                            if(temp&towers) jumped_tower = true;
                            dest = temp <<3;
                            return dest;

                        }
                        else blocked_space = 0;

                    }

                }
                //regular jump
                else{
                    temp = temp &black_on_top;
                    if((temp) && (temp<<3)&not_occupied){
                        if(temp & towers) jumped_tower = true;
                        dest = temp << 3;
                        return dest;
                    }
                }

        }//end temp&l3mask
            else if( temp & left_5_mask){

                if(temp &not_occupied){
                    //move diagonally (bottom left)
                    while(((temp &bottom_bound_mask) | (temp & left_bound_mask)) ==0 && (temp&not_occupied)){
                        if(temp&left_5_mask) temp = temp <<5;
                        else temp = temp <<4;
                        blocked_space |= temp;
                    }
                    //check if black piece can be jumped
                    if(temp & black_on_top && (~((temp & left_bound_mask) | (temp&bottom_bound_mask)))){
                        if(temp&left_5_mask && (temp <<5) &not_occupied){
                            if(temp&towers) jumped_tower = true;
                            dest = temp <<5;
                            return dest;

                        }
                        else blocked_space = 0;

                    }

                }
                //regular jump
                else{
                    temp = temp &black_on_top;
                    if((temp) && (temp<<5)&not_occupied){
                        if(temp & towers) jumped_tower = true;
                        dest = temp << 5;
                        return dest;
                    }
                }

            }//end temp & l5mask


        }//End king <<4
        
        /* if king &right 5 mask move diagonally to bottom right
        */
        temp = king & right_5_mask;
        if(temp){
            //right shift 5 manually so temp is on unoccupied space
            temp = temp >> 5;
            blocked_space |= temp;
            //if temp is not occupied contionue diagonally until border or occupied space is reached
            if(temp&not_occupied) { //KING & L5MASK
                while(((temp & top_bound_mask) | (temp & left_bound_mask)) ==0 && (temp & not_occupied))
                {
                    if(temp&right_5_mask) temp = temp >>5;
                    else temp = temp >>4;
                    blocked_space |= temp;
                }
                //check if black piece can be jumped
                if(temp & black_on_top && (~((temp&left_bound_mask)|(temp & top_bound_mask)))){
                    if(temp&right_5_mask && (temp >> 5)&not_occupied){
                        if(temp & towers) jumped_tower = true;
                        dest = temp >>5;
                        return dest;
                    }
                    else if((temp >> 4)&not_occupied){
                        if(temp &towers) jumped_tower = true;
                        dest = temp >>4;
                        return dest;
                    }
                    else blocked_space = 0;

                }

            }
            //if temp >> 5 is occupied check for regular jump
            else{
                //regular jump
                temp = temp & black_on_top;
                if(temp && (temp >> 4)& not_occupied){
                    if(temp & towers) jumped_tower = true;
                    dest =temp >> 4;
                    return dest;
                }
            }

            //move king to top right
            temp = king; //no need to and with r5M
            blocked_space |= temp;  
            temp = temp >> 4;

            blocked_space |= temp;  
            //if temp is not occupied move diagonally to top right 
            if(temp & not_occupied){
                //printf("entering buggy loop\n");
                while((((temp & top_bound_mask) | (temp & right_bound_mask))==0) && (temp & not_occupied)){
                    //sleep(1);
                    if(temp&right_3_mask){

                        print_board_from_bitfields(temp, 'T');
                        temp = temp >> 3;
                        print_board_from_bitfields(temp, 'T');
                 //       printf ("temp & right_3_mask\n");
                    }
                    else{
                  //      printf("temp >> 4\n");
                        temp = temp >> 4;
                    }

                    blocked_space |= temp;  
                   // if(temp & not_occupied) printf("not not_occupied\n");

                }
                //printf("end of buggy loop\n");
                //check if black piece can be jumped 
                if(temp & black_on_top && (~((temp &right_bound_mask) | (temp & top_bound_mask)))){
                 //   printf("found black pawn\n");
                    if(temp&right_3_mask && (temp>>3)&not_occupied){
                  //      printf("and it is jumpable (>>3)\n");
                        if(temp&towers) jumped_tower = true;
                        dest = temp >> 3;
                        return dest;
                    }
                    else if((temp >>4) & not_occupied){
                        if(temp & towers) jumped_tower = true;

                   //     printf("and it is jumpable (>>4)\n");
                        dest = temp >>4;
                        return dest;
                    }
                    else blocked_space = 0;
                }
            }
            else{ //normal jump
                temp = temp &black_on_top;
                if((temp&right_3_mask) && (temp >>3)&not_occupied){
                    if(temp & towers) jumped_tower = true;
                    dest = temp >> 3;
                    return dest;
                }

            }


        }//END if(king & r5_mask)

            //if king is on r3 mask move diagonally to top right
        temp = king & right_3_mask;
        blocked_space |= temp;
        if(temp){
            // >> 3 so temp temp is on unoccupied space
            temp = temp >>3;
            if(temp & not_occupied){
                //move diagonally
                while(((temp & top_bound_mask) | (temp & right_bound_mask)) == 0 && (temp & not_occupied)){
                    if(temp&right_3_mask) temp = temp >>3;
                    else temp = temp >>4;

                    blocked_space |= temp;  
                }
                //check if black piece can be jumped

                if(temp & black_on_top && (~((temp &right_bound_mask) | (temp & top_bound_mask)))){
                    if(temp&right_3_mask && (temp>>3)&not_occupied){
                        if(temp&towers) jumped_tower = true;
                        dest = temp >> 3;
                        return dest;
                    }
                    else if((temp >>4) & not_occupied){
                        if(temp & towers) jumped_tower = true;
                        dest = temp >>4;
                        return dest;
                    }
                    else blocked_space = 0;
                }
            }
            else{

                //regular jump
                temp = temp & black_on_top;
                if((temp) && (temp >>4)&not_occupied){
                    if(temp & towers) jumped_tower = true;
                    dest = temp >>4;
                    return dest;
                }

            }
            //move king to bottom right
            temp = king >> 4;
            blocked_space |= temp;
            if(temp & not_occupied){
                //move diagonally bottom right
                while(((temp & top_bound_mask) | (temp & left_bound_mask)) == 0 && (temp & not_occupied)){
                    if(temp & right_5_mask) temp = temp >> 5;
                    else temp =temp >> 4;

                    blocked_space |= temp;  
                }
                //check if black piece can be jumped
                if(temp & black_on_top && (~((temp & left_bound_mask) | (temp & top_bound_mask)))){
                    if((temp&right_5_mask) && (temp >>5)&not_occupied){

                        if(temp & towers) jumped_tower = true;
                        dest = temp >> 5;
                        return dest;
                    }
                    else  if ((temp >> 4) & not_occupied){

                        if(temp & towers) jumped_tower = true;
                        dest =temp >>4;
                        return dest;
                    }
                    else blocked_space = 0;


                }

            }
            else{
                //regular jump 
                temp = temp & black_on_top;
                if((temp & right_5_mask) && (temp >> 5)&not_occupied){

                    if(temp & towers) jumped_tower = true;
                    dest = temp >> 5;
                    return dest;
                }
            }
        }//end if(king & l_3mask)
        temp = king;
        blocked_space |= temp;
        if(temp) //if king is on neiteher r3 or r3 mask
        {//king >> 4
            temp = temp >> 4;
            blocked_space |= temp;
            if(temp & right_3_mask){
                if(temp &not_occupied){
                    //move diagonally (bottom right)
                    while(((temp &top_bound_mask) | (temp & right_bound_mask)) ==0 && (temp&not_occupied)){
                        if(temp&right_3_mask) temp = temp >>3;
                        else temp = temp >>4;
                        blocked_space |= temp;
                    }
                    //check if black piece can be jumped
                    if(temp & black_on_top && (~((temp & right_bound_mask) | (temp&top_bound_mask)))){
                        if(temp&right_3_mask && (temp >>3) &not_occupied){
                            if(temp&towers) jumped_tower = true;
                            dest = temp >>3;
                            return dest;

                        }
                        else blocked_space = 0;

                    }

                }
                //regular jump
                else{
                    temp = temp &black_on_top;
                    if((temp) && (temp>>3)&not_occupied){
                        if(temp & towers) jumped_tower = true;
                        dest = temp >> 3;
                        return dest;
                    }
                }

        }//end temp&r3mask
            else if( temp & right_5_mask){

                if(temp &not_occupied){
                    //move diagonally (bottom right)
                    while(((temp &top_bound_mask) | (temp & left_bound_mask)) ==0 && (temp&not_occupied)){
                        if(temp&right_5_mask) temp = temp >>5;
                        else temp = temp >>4;
                        blocked_space |= temp;
                    }
                    //check if black piece can be jumped
                    if(temp & black_on_top && (~((temp & left_bound_mask) | (temp&top_bound_mask)))){
                        if(temp&right_5_mask && (temp >>5) &not_occupied){
                            if(temp&towers) jumped_tower = true;
                            dest = temp >>5;
                            return dest;

                        }
                        else blocked_space = 0;

                    }

                }
                //regular jump
                else{
                    temp = temp &black_on_top;
                    if((temp) && (temp>>5)&not_occupied){
                        if(temp & towers) jumped_tower = true;
                        dest = temp >> 5;
                        return dest;
                    }
                }

            }//end temp & r5mask


        }//End king >>4
        




    }//end if(color == 'w')

    return dest;
}
//finds possible jump form jumper, color is color of jumper
//already jumped is used to exclude jumps that were already calculated
uint32_t get_jump(uint32_t jumper, char color, uint32_t already_jumped){

    uint32_t dest = 0;
    uint32_t temp = 0;
    uint32_t not_occupied = (~(white_on_top | black_on_top | already_jumped));
    //uint32_t white_kings = kings & white_on_top;
    //uint32_t black_kings = kings & black_on_top;
   // print_board_from_bitfields(not_occupied, 'n');
    print_board_from_bitfields(jumper, 'j');
    if(color == 'w'){ 
        if(jumper&kings) return get_royal_jump(jumper, 'w', already_jumped);     
        temp = jumper & left_3_mask;
        if(temp){
                printf("case jumper << 3\n");
                temp = (temp << 3) & black_on_top;
                if(temp){ 
                    dest = (temp << 4)& not_occupied;
                    if((temp&towers)&&dest) jumped_tower = true;
                }
                if(dest) return dest;
            }
        temp= jumper & left_5_mask;
        if(temp){
                printf("case jumper << 5\n");
                temp = (temp << 5) & black_on_top;
                if(temp) {
                    dest = (temp << 4)& not_occupied;
                    if((temp&towers)&&dest) jumped_tower = true;
                }
                if(dest) return dest;
            }
        temp= jumper & right_5_mask;
        if(temp){
            printf("case jumper >> 5\n");
            temp = (temp >> 5) & black_on_top;
            if(temp){
                dest = (temp >> 4)& not_occupied;

                if((temp&towers)&&dest) jumped_tower = true;
            }
            if(dest) return dest;
        }
        temp = jumper & right_3_mask;
        if(temp){
            printf("case jumper >> 3\n");
            temp = (temp >> 3) & black_on_top;
            if(temp){
                dest = (temp >> 4)& not_occupied;
 
                if((temp&towers)&&dest) jumped_tower = true;
            }
            if(dest) return dest;
        }
        temp = (jumper << 4) & black_on_top;
        if(temp){
            printf("case jumper << 4\n");
            if(temp&left_3_mask){
                dest = (temp <<3) & not_occupied;

                if((temp&towers)&&dest) jumped_tower = true;
                if(dest) return dest;
            }
            if(temp&left_5_mask){
                dest = (temp << 5) &not_occupied;

                if((temp&towers)&&dest) jumped_tower = true;
                if(dest) return dest;
            }
        }
        temp = (jumper >> 4) & black_on_top;
        if(temp){
            printf("case jumper >> 4\n");
            if(temp&right_3_mask){
                dest = (temp >>3) & not_occupied;
                if((temp&towers)&&dest) jumped_tower = true;
                if(dest) return dest;
            }
            if(temp&right_5_mask){
                dest = (temp >> 5) & not_occupied;

                if((temp&towers)&&dest) jumped_tower = true;
                if(dest) return dest;
            }
        }
    } //END if color = 'w'
    else if(color == 'b'){

        if(jumper&kings) return get_royal_jump(jumper, 'b', already_jumped);     
        temp = jumper & left_3_mask;
        if(temp){
                printf("case jumper << 3\n");
                temp = (temp << 3) & white_on_top;
                if(temp){ 
                    dest = (temp << 4)& not_occupied;

                    if((temp&towers)&&dest) jumped_tower = true;
                }
                if(dest) return dest;
            }
        temp= jumper & left_5_mask;
        if(temp){


                printf("case jumper << 5\n");
                temp = (temp << 5) & white_on_top;
                if(temp) {
                    dest = (temp << 4)& not_occupied;
                    if((temp&towers)&&dest) jumped_tower = true;
                }
                if(dest) return dest;
            }

        temp= jumper & right_5_mask;
        if(temp){

            printf("case jumper >> 5\n");
            temp = (temp >> 5) & white_on_top;

            if(temp){
                dest = (temp >> 4)& not_occupied;

                if((temp&towers)&&dest) jumped_tower = true;
            }
            if(dest) return dest;
        }
        temp = jumper & right_3_mask;
        if(temp){

            printf("case jumper >> 3\n");
            temp = (temp >> 3) & white_on_top;
            if(temp){
                dest = (temp >> 4)& not_occupied;
 
                if((temp&towers)&&dest) jumped_tower = true;
            }
            if(dest) return dest;
        }
        temp = (jumper << 4) & white_on_top;
        if(temp){

            printf("case jumper << 4\n");
            if(temp&left_3_mask){
                dest = (temp <<3) & not_occupied;

                if((temp&towers)&&dest) jumped_tower = true;
                if(dest) return dest;
            }
            if(temp&left_5_mask){
                dest = (temp << 5) &not_occupied;

                if((temp&towers)&&dest) jumped_tower = true;
                if(dest) return dest;
            }
        }

        temp = (jumper >> 4) & white_on_top;
        if(temp){
            printf("case jumper >> 4\n");
            if(temp&right_3_mask){
                dest = (temp >>3) & not_occupied;

                if((temp&towers)&&dest) jumped_tower = true;
                if(dest) return dest;
            }
            if(temp&right_5_mask){
                dest = (temp >> 5) & not_occupied;

                if((temp&towers)&&dest) jumped_tower = true;
                if(dest) return dest;
            }
        }
    }

    //print_board_from_bitfields(dest, 'd');
    return dest;
}

//TODO consider towers
void get_all_jumps(uint32_t jumpers, char color){
    printf("color is %c\n", color);
    //uint32_t old_kings = kings;
    int j = 0;
    for(int i = 0; i< 32; i++){
        if(IS_SET(jumpers, i)){
            uint32_t temp = 0;
            SET_BIT(temp, i);
            pos_node_from_bf(temp, &moves[j]);
            printf("callinf get jump\n");
            uint32_t dest = get_jump(temp, color, 0);
            if(temp&kings){
                kings |= dest;
            }
            if(dest == 0){
                printf("(get_all_jumps) could not find jump!!\n");
                //exit(EXIT_SUCCESS);
            }
            while(dest){ //check if jumper can jump in multiple directions
       //         sleep(1);

        //        print_board_from_bitfields(dest, 'd');
                node_t* new_node = create_new_list(0, 0);
                pos_node_from_bf(dest, new_node);
       //         printf("get_tail_1\n");
                printf("appending new node to list %c%c list nr %i\n", moves[j].type, moves[j].val, j); 
                append_to_list(new_node, get_tail(&moves[j]));
                uint32_t prev_dest = dest; //only works for no. jumps < 2
                dest = get_jump(temp, color, prev_dest); // if jumper can only jump in one direction dest will bi 0
                j++;
                if(j == 32){
                    //exit(EXIT_FAILURE);
                    printf("(get all jumps) something went wrong\n");
                    break;
                }
            }
        }
    }
    //moves now contains all single jumps now looking for multi jump
    j=0;
    for(int i = 0; i<32; i++){
       // printf("get_tail_2");
        uint32_t current_pos = bf_from_pos_node(get_tail(&moves[j]));
        print_board_from_bitfields(current_pos, 'c');
        if (current_pos == 0) break;
        //print_board_from_bitfields(current_pos, 'c');
        //printf("get_tail_3");
        uint32_t prev_pos =0;
        if(get_tail(&moves[j])->prev != NULL){
            prev_pos = bf_from_pos_node(get_tail(&moves[j])->prev); //to avoid 180deg turns
        }
        //print_board_from_bitfields(prev_pos, 'p');
        

        uint32_t dest = get_jump(current_pos, color, prev_pos);
        if(current_pos&kings){
            kings &= ~current_pos;
            kings |= dest;
            printf("current_pos&kings\n");
            print_board_from_bitfields(kings, 'K');
        }
        if(dest == 0){
            //kings = old_kings;
            j++;
      //      printf("j is now %d\n", j);
        }
        else{
       //     printf("appending new_node\n");
       //     print_board_from_bitfields(dest, 'd');
            node_t* new_node = create_new_list(0,0);

            printf("appending new node to list %c%c list nr %i\n", moves[j].type, moves[j].val, j); 
            new_node = pos_node_from_bf(dest, new_node);
            append_to_list(new_node, get_tail(&moves[j]));
        }
    }

}
//mover shoud be one single mover not all of them
//in case a mover can move in two directions set previouslu calculated move to already move to find other move
uint32_t get_move(uint32_t mover, char color, uint32_t already_moved){
    uint32_t not_occupied = (~(white_on_top | black_on_top | already_moved));
    uint32_t temp =0;
    if(color == 'w'){

        temp = (mover >> 4) & not_occupied;
        if (temp) return temp;
        temp = mover & right_3_mask;
        if(temp && (temp >> 3)&not_occupied) return temp >>3;
        temp = mover & right_5_mask;
        if(temp && (temp >>5)&not_occupied) return temp >>5;
    }
    else if(color == 'b'){
        temp = (mover << 4) & not_occupied;
        if (temp) return temp;
        temp = mover & left_3_mask;
        if(temp && (temp << 3)&not_occupied) return temp <<3;
        temp = mover & left_5_mask;
        if(temp && (temp <<5)&not_occupied) return temp <<5;


    }
    else{
        printf("(get_move) invalid color!\n");
        return 0;
    }
    printf("(get_move) could not find valid move\n");
    return 0;
}

void get_all_moves(uint32_t movers, char color){

    int j = 0;
    for(int i = 0; i<32; i++){
        if(moves[i].next == 0 && moves[i].prev == NULL && moves[i].type == 0 && moves[i].val == 0){
            j=i;
            break;
        }

    }
    for(int i = 0; i< 32; i++){
        if(IS_SET(movers, i)){
            uint32_t temp = 0;
            SET_BIT(temp, i);
            pos_node_from_bf(temp, &moves[j]);
            uint32_t dest = get_move(temp, color, 0);
            int moved = 0;
            while(dest){ 
                
                if (moved >= 2) break;
                node_t* new_node = create_new_list(0, 0);
                pos_node_from_bf(dest, new_node);
                printf("(get_all_moves) appending node to list %c%c list nr %i\n", moves[j].type, moves[j].val, j);
                append_to_list(new_node, get_tail(&moves[j]));
                uint32_t prev_dest = dest; 
                dest = get_move(temp, color, prev_dest); 
                moved++;
                j++;
                if(j == 32){
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}
//generates move string from list of nodes 

//head has to be head of list!!    
char* move_str_from_list(node_t* head){
    //get list len
    int len= count_list_elements(head);
    char* move = malloc(sizeof(char)*(3*len+2));
  //  memset(&move, 0, sizeof(move));
    strcpy(move, "\0");
    while(head != NULL){
        strncat(move, &head->type, 1);

    //    printf("type %s\n", move);

        strncat(move, &head->val,1 );

     //   printf("val %s\n", move);

        strcat(move, ":");

      //  printf(": %s\n", move);
        head = head->next;
    }
    move[strlen(move)-1] = '\n';
    printf("(move_str_from_list) : %s", move);
    return move;
}
        
        
    
     

    





//uses node_t board[32] to generate white, black, king and tower bitfields
void gen_bitfields(){
    white_on_top = 0;
    black_on_top = 0;
    kings = 0;
    towers = 0;
    for(int i =0; i < 32; i++){
        node_t* temp = get_tail(&board[i]);
        //printf("(gen_bitfields) current type is: %c\n", temp->type);
        if((temp->type == 'W') | (temp->type == 'w')){
            SET_BIT(white_on_top, i);
        }
        if((temp->type == 'B') | (temp->type == 'b')){
            SET_BIT(black_on_top, i);
        }
        if((temp->type == 'W') | (temp->type == 'B')){
            SET_BIT(kings, i);
        }
        if(temp->prev != NULL){
            SET_BIT(towers, i);
        }
    }

}
void init_board(){
    for (int i = 0; i < 32; i++){

        if(board[i].next != NULL){
            node_t* temp = board[i].next;
            while(temp != NULL){
                node_t* next_node = temp->next;
                free(temp);
                temp = next_node;
            }
        }
        board[i].prev = NULL;
        board[i].next = NULL;
        board[i].type = ' ';
    }
}

int think(sharestruct* sharedmem){
    printf("(think) thinking\n");
    lock_mem_segment(&sharedmem, sizeof(sharestruct));
    char color = sharedmem->spielernumer==0 ? 'w' : 'b';
    printf("(think) printing piecelist:\n");
    for(int i =0; i<64; i++){//replace 64 with global var
        if(sharedmem->piecelist[i][0] == '\0'){
            printf("sharedmem->piecelist[%d] == '\\0'\n", i);
            break;
        }
        else{
            printf("(think) piece[%d]: %s",i, sharedmem->piecelist[i]);
        }
    }
    unlock_mem_segment(&sharedmem, sizeof(sharestruct));
    init_board();
    init_moves();
    create_board_from_piecelist(sharedmem, board);
    gen_bitfields();
    print_board_from_bitfields(white_on_top, 'w');
    print_board_from_bitfields(black_on_top, 'b');
    print_board_from_bitfields(towers, 't');
    print_board_from_bitfields(kings, 'K');
    uint32_t white_movers = get_white_movers();
    uint32_t black_movers = get_black_movers();
    uint32_t white_jumpers = 0;
    uint32_t black_jumpers = 0;
    printf("white_movers:\n");
    print_board_from_bitfields(white_movers, 'w');
    printf("black_movers:\n");
    print_board_from_bitfields(black_movers, 'b');
    if(color == 'w'){
        printf("white_jumpers:\n");
        white_jumpers = get_white_jumpers();
        print_board_from_bitfields(white_jumpers, 'w');
    }
    if(color == 'b'){
        printf("black jumpers:\n");
        black_jumpers = get_black_jumpers();
        print_board_from_bitfields(black_jumpers, 'b');
    }
    printf("looking for jumps:\n");
    init_moves();
    blocked_space = 0;
    if(white_jumpers && color == 'w'){
        printf("looking for white jumps\n");
        get_all_jumps(white_jumpers, 'w');
        for(int i = 0; i <32 ; i++){
            if(moves[i].val == 0) break;
            printlist(&moves[i]);
        }

    }
    else if(black_jumpers && color == 'b'){
        printf("looking for black jumps\n");
        get_all_jumps(black_jumpers, 'b');
    }
    if (color == 'w'){

        printf("looking for white movers\n");
        get_all_moves(white_movers, 'w');
    }
    if(color == 'b'){
        printf("looking for black movers\n");
        get_all_moves(black_movers, 'b');
    }

    return EXIT_SUCCESS;
}

void sig_handler() 
{
    got_signal=1;
}
void sigterm_handler(){
        printf("got sigterm\n");
            exit(EXIT_SUCCESS);
}


int run_thinker(sharestruct* sharedmem, int* fields){
    while(1){
        printf("(run_thinker) waiting for signal from child\n");
        init_moves();
        while(got_signal == 0 ){
            lock_mem_segment(&sharedmem->sent_signal, sizeof(bool));
            if(sharedmem->sent_signal == true){
                sharedmem->sent_signal = false;
                unlock_mem_segment(&sharedmem->sent_signal, sizeof(bool));
                signal(SIGUSR1, sig_handler);
                signal(SIGTERM, sigterm_handler);
            }
            if(kill(sharedmem->connector_pid, 0) !=0) return EXIT_SUCCESS;
            unlock_mem_segment(&sharedmem->sent_signal, sizeof(bool));
        }
        got_signal =0; //reset siganl;
        printf("(sig_handler)got signal form child\n");
        think(sharedmem);
        int longes_move = 0;
        int j = 0;
   //     printf("(run_thinker) count_list_elements test\n");
  //      int test = count_list_elements(get_head(&moves[1]));
 //       printf("(run_thinker list 1 is %d elems long\n", test);
        printf("(run_thinker) finding longest move..\n");
        for(int i = 0; i < 32; i++){
            if(moves[i].val == 0) break;
            if(count_list_elements(get_head(&moves[i])) > longes_move){
                longes_move = count_list_elements(get_head(&moves[i]));
                j = i;
            }
        }
//        printf("generating chess notation from list..\n");
        char* final_move = move_str_from_list(get_head(&moves[j]));
        printf("(run_thinker) longes_move: %s", final_move);

        write(fields[1], final_move, strlen(final_move));
        free(final_move);
    }
    printf("(run_thinker) finished... terminating\n");
    return 0;
}

