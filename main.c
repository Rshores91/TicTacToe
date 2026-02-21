#include <pthread.h> // For threading and synchronization primitives
#include <stdio.h> // For printf()
#include <stdlib.h> // For rand() and srand()
#include <stdbool.h> // For boolean type
#include <time.h> // For time-based seeding of the random number generator
#include <windows.h> // For GetTickCount on Windows
#include <stdint.h>

static char board[3][3]; // Game board
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // Mutex to protect shared game state (board, current_player, game_finished)
static pthread_cond_t turn_cv = PTHREAD_COND_INITIALIZER; // pthread condition variable to signal turns between players
static int current_player = 0; // shared turn state: 0 for player 0, 1 for player 1
static bool game_finished = false; // shared game state to indicate if the game has finished (either by win or draw)
bool check_winner(char mark); // Function prototype for checking if a player has won
bool board_is_full(void); // Function prototypes for checking win conditions and board state

static unsigned int next_random(unsigned int *state) {
    unsigned int x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}


void print_board_locked(void) {  // Function to print the current state of the board, assumes the caller has already locked the mutex
    printf("\n");
    for (int r = 0; r < 3; r++) {
        printf(" %c | %c | %c \n", board[r][0], board[r][1], board[r][2]);
        if (r < 2) {
            printf("---|---|---\n");
        }
    }
}




bool board_is_full(void) { // Function to check if the board is full (no empty spaces), assumes the caller has already locked the mutex
    for (int r=0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (board[r][c] == ' ') {
                return false; // Found an empty cell, board is not full
            }
        }
    }
    return true; // Board is full if no empty cells are found
}



bool check_winner(char mark) { // Function to check if the given mark ('X' or 'O') has won the game, assumes the caller has already locked the mutex
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
    char mark = (player == 1) ? 'X' : 'O'; // Assign mark based on player number
    unsigned int seed = (unsigned int)time(NULL)
                      ^ (unsigned int)clock()
                      ^ (unsigned int)(player * 2654435761u)
                      ^ (unsigned int)(uintptr_t)&player
                      ^ (unsigned int)GetTickCount()
                      ^ (unsigned int)GetCurrentThreadId();

    if (seed == 0u) {
        seed = 2463534242u;
    }

    while (true) {

        int placed = 0; // Flag to indicate if the move was placed successfully
        bool won_now = false; // Placeholder for win condition check
        bool board_full = false; // Placeholder for board full check


        pthread_mutex_lock(&lock); // Lock the mutex to check the game state and wait for the turn
        while (current_player != player && !game_finished) { //
            pthread_cond_wait(&turn_cv, &lock); // Wait for the turn or game to finish
        } // end of turn wait loop
        if (game_finished) {
            pthread_mutex_unlock(&lock); // Unlock before exiting
            break;
        } // end of game finished loop


        // TODO: get move, validate, update board
        printf("Player %d's turn\n", player);

        

        int empties[9];
        int empty_count = 0;

        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                if (board[r][c] == ' ') {
                    empties[empty_count++] = r * 3 + c;  // store cell index 0..8
                }
            }
        } // end of loop to find empty cells

        if (empty_count == 0) {
            game_finished = true;  // tie
        } else {
            int pick = (int)(next_random(&seed) % (unsigned)empty_count);
            int move = empties[pick];
            int row = move / 3;
            int col = move % 3;

            board[row][col] = mark;
            printf("Player %d places %c at (%d, %d)\n", player, mark, row, col);
            print_board_locked();
            placed = 1; // Mark the move as placed

        } // end of move selection and placement


        if (!placed) {
            printf("Player %d failed to place a move after 9 tries. Ending game.\n", player);
            game_finished = true; // If no valid move was placed after 9 tries, end the game (should not happen in a normal game)
        }

        
        if (placed) {
        won_now = check_winner(mark); // Check if the current player has won
        board_full = board_is_full(); // Check if the board is full
            if(won_now || board_full) {
                if (won_now) {
                    printf("Player %d wins!\n", player);
                } else {
                    printf("The game is a draw!\n");
                }
            game_finished = true; // Set game finished if there's a winner or the board is full
            pthread_cond_broadcast(&turn_cv); // Wake up the other player to let them know the game is finished
            pthread_mutex_unlock(&lock); // Unlock before exiting

            }
        }


        current_player = 1 - current_player; // Switch to the other player
        pthread_cond_broadcast(&turn_cv);
        pthread_mutex_unlock(&lock);
    } // end of for loop
    return NULL;

} // End of player_thread function




void init_board(void) { // Function to initialize the game board to empty spaces
    for (int r = 0; r < 3; r++) { // Loop through each row
        for (int c = 0; c < 3; c++) { // Loop through each column in the current row
            board[r][c] = ' '; // Set the cell to a space character to indicate it's empty
        }
    }
}



int main(void) {
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



