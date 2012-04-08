#include <stdio.h>
#include <stdlib.h>


enum {
    SUCCESS = 1,
    FAILURE = -2,
    CPU = 4,
    USER = 1,
    INPROGRESS = 0,
    DRAW = 0,
    CPUWIN = 2,
    USERWIN = -1
};

typedef struct move_s {
    struct move_s* up;
    struct move_s* down[9];
    short int board[9];
    short int rank;
    short int turn;
}move;

typedef struct tictactoe_s {
    short int board[9];
    int winner;
    int turn;
    int status;
}tictactoe;




int iswinner(short int *board)
{

    short int i;
    short int sums[8] = { board[0] + board[1] + board[2],
                          board[3] + board[4] + board[5], 
                          board[6] + board[7] + board[8],
                          board[0] + board[3] + board[6],
                          board[1] + board[4] + board[7],
                          board[2] + board[5] + board[8],
                          board[0] + board[4] + board[8],
                          board[2] + board[4] + board[6] };
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
        if ( board[i] == 0 ) {
            return INPROGRESS;
        }
    }

    //draw
    return DRAW;
}






move* initMiniMax(short int *currentBoard, int turn)
{
    int i;

    move *initial = malloc( sizeof( move ) );

    initial->up = NULL;

    for( i = 0; i < 9; i++ ) {
        initial->down[i] = NULL;
        initial->board[i] = currentBoard[i];
    }

    initial->rank = 0;
    initial->turn = turn;
    return initial;
}








int buildMiniMax(move* initialMove)
{
    short int i,j;

    move *new = NULL;
    move *temp = NULL;

    for ( i = 0; i < 9; i++ ) {

        if ( initialMove->board[i] == 0 ) {

            if ( ( new = malloc( sizeof( move ) ) ) == NULL ) {
                return FAILURE;
            }
 
            new->up = initialMove;

            for( j = 0; j < 9; j++ ) {
                new->down[j] = NULL;
                new->board[j] = initialMove->board[j];
            }

            if( initialMove->turn == CPU ) {
                new->board[i] = USER;
                new->turn = USER;
            }
            else {
                new->board[i] = CPU;
                new->turn = CPU;
            }

            
            new->rank = iswinner( new->board );
            //rank of current board affects all boards higher in the tree
            for(temp = new->up; temp != NULL; temp = temp->up) {
                temp->rank += new->rank;
            }

            
            initialMove->down[i] = new;

            if ( new->rank == 0 ) {
                buildMiniMax( new );
            }
        }
    }
    
    return SUCCESS;
}



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
    game->winner = DRAW;
    return game;
}
    

void drawGame( tictactoe* game )
{
    int i;
    char *unoccupiedColor = "\033[0;37m";
    char *userColor = "\033[0;32m";
    char *cpuColor = "\033[0;31m";    
    char *reset = "\033[0m";

    for( i = 0; i < 9; i++ ) {
        if( game->board[i] == 0 ) {
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





int cpuMove( move* current )  
{
    short int i, rank; 
    short int choice = -1;

    for(i = 0; current->down[i] == NULL; i++ ) 
        ;  

    rank = current->down[i]->rank;

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

int printWinner( tictactoe* game )
{
    if( iswinner( game->board ) == DRAW ) {
        printf("The game was a draw\n");
    }
    else if( iswinner( game->board ) == CPUWIN ) {
        printf("The computer won\n");
    }
    else {
        printf("The user won\n");
    }
    return SUCCESS;
}



int gameLoop( tictactoe* game, move* initial )
{
    int optimalMove;
    int userMove;

    move *current = initial;

    while( game->status == INPROGRESS ) {
        if (game->turn == USER ) { //user just went

            //get optimal move, if there is in error, return.
            if( ( optimalMove = cpuMove( current ) ) < 0 ) {
                return FAILURE;
            }
            
            printf("Computers turn\nComputer picks %d\n", optimalMove);

            game->board[optimalMove] = CPU;

            drawGame( game );
            
            initial = current;
            current = current->down[optimalMove];
            initial->down[optimalMove] = NULL;
            destroyTree( initial );
            
            game->turn = CPU;
        }

        game->status = iswinner( game->board );

        if( game->turn == CPU && game->status == INPROGRESS ) {

            //get user move, keep trying if failure
            while( ( userMove = getUserMove( game ) ) == FAILURE )
                ;
            game->board[userMove] = USER;
            
            drawGame( game );

            initial = current;
            current = current->down[userMove];
            initial->down[userMove] = NULL;
            destroyTree( initial );

            game->turn = USER;
        }
        
        game->status = iswinner( game->board );
    }
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
    if ( ( initial = initMiniMax( game->board, USER ) ) == NULL ) {
        return FAILURE;
    }

    //build the tree, if it fails, quit.
    if( ( buildMiniMax( initial ) ) == FAILURE ) {
        return FAILURE;
    }

    gameLoop( game, initial );

    printWinner( game );
}
