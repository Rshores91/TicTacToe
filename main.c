#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

static char board[3][3]; // Game board
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t turn_cv = PTHREAD_COND_INITIALIZER;
static int current_player = 0; // shared turn state: 0 for player 0, 1 for player 1
static bool game_finished = false;
bool check_winner(char mark);
bool board_is_full(void);


void print_board_locked(void) { 
    printf("\n");
    for (int r = 0; r < 3; r++) {
        printf(" %c | %c | %c \n", board[r][0], board[r][1], board[r][2]);
        if (r < 2) {
            printf("---|---|---");
        }
    }
}




bool board_is_full(void) {
    for (int r=0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (board[r][c] == ' ') {
                return false; // Found an empty cell, board is not full
            }
        }
    }
    return true; // Board is full if no empty cells are found
}



bool check_winner(char mark) {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == mark && board[i][1] == mark && board[i][2] == mark) {
            return true; // Row win
        }
    }
    for (int i = 0; i < 3; i++) {
        if (board[0][i] == mark && board[1][i] == mark && board[2][i] == mark) {
            return true; // Column win
        }
    }
    if (board[0][0] == mark && board[1][1] == mark && board[2][2] == mark) {
        return true; // Diagonal win
    }
    if (board[0][2] == mark && board[1][1] == mark && board[2][0] == mark) {
        return true; // Anti-diagonal win
    }
    return false; // No win condition met
}

void *player_thread(void *arg) { // Thread function for each player: works by passing the player number as an argument to the thread, which then uses it to determine its mark and when to play. The thread will loop until the game is finished, checking for its turn and making moves accordingly.
    int player = *(int *)arg; // Get player number from argument
    char mark = (player == 0) ? 'X' : 'O'; // Assign mark based on player number
    while (true) {

        int placed = 0; // Flag to indicate if the move was placed successfully
        bool won_now = false; // Placeholder for win condition check
        bool board_full = false; // Placeholder for board full check


        pthread_mutex_lock(&lock); // Lock the mutex to check the game state and wait for the turn
        while (current_player != player && !game_finished) { //
            pthread_cond_wait(&turn_cv, &lock); // Wait for the turn or game to finish
        }
        if (game_finished) {
            pthread_mutex_unlock(&lock); // Unlock before exiting
            break;
        }


        // TODO: get move, validate, update board
        printf("Player %d's turn\n", player);

        
        int move = 0;


        for (int tries = 0; tries < 9 && !placed; tries++) {
        move = rand() % 9; // Random move for demonstration
        int row = move / 3; // Calculate row from the move index
        int col = move % 3; // Calculate column from the move index


        // char mark = (player == 0) ? 'X' : 'O';   // or swap if you want player 0 = O

        if (board[row][col] == ' ') {
            board[row][col] = mark;
            printf("Player %d places %c at (%d, %d)\n", player, mark, row, col);
            placed = 1;
            print_board_locked();
}

        placed = 1; // Move placed successfully
            

        } // End of move validation loop

        
        if (placed) {
        won_now = check_winner(mark); // Check if the current player has won
        board_full = board_is_full(); // Check if the board is full
            if(won_now || board_full) {
            game_finished = true; // Set game finished if there's a winner or the board is full
            }
        }


        current_player = 1 - current_player; // Switch to the other player
        pthread_cond_broadcast(&turn_cv);
        pthread_mutex_unlock(&lock);
    } // end of for loop
    return NULL;

} // End of player_thread function




void init_board(void) {
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            board[r][c] = ' ';
        }
    }
}



int main(void) {
    srand((unsigned)time(NULL));
    init_board();

    pthread_t t1, t2;
    int p0 = 0, p1 = 1;

    pthread_create(&t1, NULL, player_thread, &p0); // Create thread for player 0 - each argument is as follows - &t1: pointer to thread identifier, NULL: default thread attributes, player_thread: function to run in the thread, &p0: argument to pass to the thread function (player number)
    pthread_create(&t2, NULL, player_thread, &p1); // Create thread for player 1

    pthread_join(t1, NULL); // Wait for player 0 thread to finish
    pthread_join(t2, NULL); // Wait for player 1 thread to finish

    printf("Game over!\n");
    return 0;
}



