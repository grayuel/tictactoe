/*
Copyright (c) 2012, Samuel Gray, samgray[at]sonic[dot]net 

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>


enum {
    SUCCESS = 0,
    FAILURE = -2,

    CPU = 4,            //value of a cpu occupied square
    USER = 1,           //value of a user occupied square
    UNOCCUPIED = 0,     //value of an unoccupied square

    INPROGRESS = 0,     //      The four possible
    DRAW = 0,           //      states of the game
    CPUWIN = 2,         //
    USERWIN = -1        //      Also used for ranking
};

//game tree struct
typedef struct move_s {
    struct move_s* up;          //pointer to parent
    struct move_s* down[9];     //pointer to all possible children
    short int path;             //what move led to the current branch
    short int board[9];         //current board state
    short int rank;             //rank of the current state
    short int turn;             //whose turn led to this state
}move;

//game state struct
typedef struct tictactoe_s {
    short int board[9];         //current board state
    int turn;                   //who just went
    int status;                 //is the game in progress, complete, etc.
}tictactoe;



//determines if there is a winner
int iswinner(short int *board)
{

    short int i;
    short int sums[8] = { board[0] + board[1] + board[2],   //row 1
                          board[3] + board[4] + board[5],   //row 2
                          board[6] + board[7] + board[8],   //row 3
                          board[0] + board[3] + board[6],   //col 1
                          board[1] + board[4] + board[7],   //col 2
                          board[2] + board[5] + board[8],   //col 3
                          board[0] + board[4] + board[8],   //diag 1
                          board[2] + board[4] + board[6] }; //diag 2
    for( i = 0; i < 8; i++ ) {
        //computer wins
        if ( sums[i] == CPU + CPU + CPU ) {
            return CPUWIN;
        }
        //user wins
        else if ( sums[i] == USER + USER + USER ) {
            return USERWIN;
        }
    }
    
    //game still in progress
    for( i = 0; i < 9; i++ ) {
        if ( board[i] == UNOCCUPIED ) {
            return INPROGRESS;
        }
    }

    //draw
    return DRAW;
}





//build the top of the game tree
move* initGameTree(short int *currentBoard, int turn)
{
    int i;

    move *initial = malloc( sizeof( move ) );

    initial->up = NULL;

    for( i = 0; i < 9; i++ ) {
        initial->down[i] = NULL;
        initial->board[i] = currentBoard[i];
        if( currentBoard[i] != UNOCCUPIED ) {
            initial->path = i;
        }
    }

    initial->rank = 0;
    initial->turn = turn;
    return initial;
}







//build the entire game tree.
int buildGameTree(move* initialMove)
{
    short int i,j;

    move *new = NULL;
    move *temp = NULL;

    for ( i = 0; i < 9; i++ ) {

        if ( initialMove->board[i] == UNOCCUPIED ) {
            
            //allocate memory
            if ( ( new = malloc( sizeof( move ) ) ) == NULL ) {
                return FAILURE;
            }
 
            //initialize everything
            new->path = i;
            new->up = initialMove;

            for( j = 0; j < 9; j++ ) {
                new->down[j] = NULL;
                new->board[j] = initialMove->board[j];
            }

            //add the move to the board
            if( initialMove->turn == CPU ) {
                new->board[i] = USER;
                new->turn = USER;
            }
            else {
                new->board[i] = CPU;
                new->turn = CPU;
            }

            //compute rank of current board
            new->rank = iswinner( new->board );

            //if cpu move led directly to the a user win, delete that branch
            if( new->rank == USERWIN && initialMove->up != NULL ) {
//                destroyTree does not currently work.
//                destroyTree( initialMove->up->down[initialMove->path] );
                initialMove->up->down[initialMove->path] = NULL;
            }

            //rank of current board affects all boards higher in the tree
            for(temp = new->up; temp != NULL; temp = temp->up) {
                temp->rank += new->rank;
            }

            //connect created branch to the tree
            initialMove->down[i] = new;

            //if the game can continue from current branch, do so.
            if ( new->rank == INPROGRESS ) {
                buildGameTree( new );
            }
        }
    }
    
    return SUCCESS;
}


//currently broken
//recursively free memory allocated to the tree.
int destroyTree( move *top )
{
    int i;
    for( i = 0; i < 9; i++ ) {
        if( top->down[i] != NULL ) {
            destroyTree( top->down[i] );
            top->down[i] = NULL;
        }
    }

    free( top );
    return SUCCESS;
}



//initialize the game struct
tictactoe* initGame( void )
{
    tictactoe* game = NULL;

    if ( ( game = malloc( sizeof( tictactoe ) ) ) == NULL ) {
        return game;
    }
    
    int i;
    for( i = 0; i < 9; i++ ) {
        game->board[i] = 0;
    }
    game->turn = USER;
    game->status = INPROGRESS;
    return game;
}
  
  
//draws the game, x for user, o for cpu, 1-9 for unoccupied squares
void drawGame( tictactoe* game )
{
    int i;
    char *unoccupiedColor = "\033[0;37m";   //gray
    char *userColor = "\033[0;32m";         //green
    char *cpuColor = "\033[0;31m";          //red 
    char *reset = "\033[0m";                //reset

    for( i = 0; i < 9; i++ ) {
        if( game->board[i] == UNOCCUPIED ) {
            printf("%s %i ", unoccupiedColor, i);
        }
        else if( game->board[i] == CPU ) {
            printf("%s o ", cpuColor);
        }
        else {
            printf("%s x ", userColor);
        }
        printf("%s", reset);
        if( i % 3 == 2 ) { 
            printf("\n");
        }
    }

    printf("\n");
}





//gets the users move
//need to fix double printing of the prompt, due to enter key I think.
int getUserMove( tictactoe* game )
{
    printf("Enter the number of an unoccupied square: ");
    char userMove = getchar();
    if( userMove - '0' < 9 && userMove - '0' >= 0 ) {
        if( (game->board[userMove - '0']) == 0 ) {
            printf("You picked %d\n", userMove - '0');
            return userMove - '0';
        }
    }
    else {
        return FAILURE;
    }
}




//find the cpu move with the highest rank
int cpuMove( move* current )  
{
    short int i, rank; 
    short int choice = -1;

    //set i equal to the first possible move
    for(i = 0; current->down[i] == NULL; i++ ) 
        ;  

    //rank of first possible move
    rank = current->down[i]->rank;

    //find a higher rank;
    for( ; i < 9; i++ ) {
        if ( current->down[i] != NULL ) {
            if ( rank <= current->down[i]->rank ) {
                printf("ranks: %d\n", rank);
                rank = current->down[i]->rank;
                choice = i;
            }
        }
    }
    return choice;
}




//prints a message saying who won.
int printWinner( int winner )
{
    if( winner == DRAW ) {
        printf("The game was a draw\n");
    }
    else if( winner == CPUWIN ) {
        printf("The computer won\n");
    }
    else if( winner == USERWIN ) { //this should never happen
        printf("The user won\n");
    }
    else {
        return FAILURE;
    }

    return SUCCESS;
}


//the game loop, starts after the users first move
int gameLoop( tictactoe* game, move* initial )
{
    int optimalMove;
    int userMove;

    move *current = initial;

    while( game->status == INPROGRESS ) {
        if (game->turn == USER ) {              //user just went

            //get optimal move, if there is in error, quit.
            if( ( optimalMove = cpuMove( current ) ) < 0 ) {
                return FAILURE;
            }
            
            printf("Computers turn\nComputer picks %d\n", optimalMove);

            game->board[optimalMove] = CPU;

            drawGame( game );

            //move to the branch of the cpu move
            //disconnect from the old tree
            //destroy the old tree           
            initial = current;
            current = current->down[optimalMove];
            initial->down[optimalMove] = NULL;
            destroyTree( initial );
            
            game->turn = CPU;
        }

        //see if the cpu just won or drew
        game->status = iswinner( game->board );

        //cpu just went, game still not over
        if( game->turn == CPU && game->status == INPROGRESS ) {

            //get user move, keep trying if failure
            while( ( userMove = getUserMove( game ) ) == FAILURE )
                ;

            game->board[userMove] = USER;
            
            drawGame( game );

            //move to the branch of the cpu move
            //disconnect from the old tree
            //destroy the old tree           
            initial = current;
            current = current->down[userMove];
            initial->down[userMove] = NULL;
            destroyTree( initial );

            game->turn = USER;
        }
        
        //check for winner again
        game->status = iswinner( game->board );
    }
//    destroyTree( initial );
    return( game->status );
}





int main()
{
    int userMove;
    int optimalMove;

    tictactoe *game = NULL;
    if( ( game = initGame() ) == NULL) {
        return FAILURE;
    }

    drawGame( game );

    while( ( userMove = getUserMove( game ) ) == FAILURE )
        ;
    game->board[userMove] = USER;    

    drawGame( game );

    //make the top of the tree, if it fails, quit.
    move *initial = NULL;
    if ( ( initial = initGameTree( game->board, USER ) ) == NULL ) {
        return FAILURE;
    }

    //build the tree, if it fails, quit.
    if( ( buildGameTree( initial ) ) == FAILURE ) {
        return FAILURE;
    }

    printWinner( gameLoop( game, initial ) );
}
