#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

static char board[3][3]; // Game board
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t turn_cv = PTHREAD_COND_INITIALIZER;
static int current_player = 0;
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

void *player_thread(void *arg) {
    int player = *(int *)arg;
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

        if (board[row][col] == ' ') { // Check if the cell is empty
            if (player == 0) {
                board[row][col] = 'O'; // Mark player 0's move
                printf("Player 0 places marker at (%d, %d)\n", row, col);
            } else {
                board[row][col] = 'X'; // Mark player 1's move
                printf("Player 1 places marker at (%d, %d)\n", row, col);
            }

            placed = 1; // Move placed successfully
            
            char mark = (player == 0) ? 'O' : 'X';
    


            if (placed) {
                won_now = check_winner(mark); // Check if the current player has won
                board_full = board_is_full(); // Check if the board is full
                if(won_now || board_full) {
                    game_finished = true; // Set game finished if there's a winner or the board is full
                }
            }

        } // End of move validation loop



        current_player = 1 - current_player; // Switch to the other player
        pthread_cond_broadcast(&turn_cv);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
} // End of while Loop
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

    pthread_create(&t1, NULL, player_thread, &p0); // Create thread for player 0
    pthread_create(&t2, NULL, player_thread, &p1); // Create thread for player 1

    pthread_join(t1, NULL); // Wait for player 0 thread to finish
    pthread_join(t2, NULL); // Wait for player 1 thread to finish

    printf("Game over!\n");
    return 0;
}



