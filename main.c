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

void *player_thread(void *arg) {
    int player = *(int *)arg;
    while (true) {
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

        } else {
            pthread_mutex_unlock(&lock); // Unlock before retrying
            continue;
        }

        current_player = 1 - current_player; // Switch to the other player
        pthread_cond_broadcast(&turn_cv);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

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



