//
// 2048.c
//
// Copyright (c) 2014 Ben Kogan <http://benkogan.com>
// Gameplay based on 2048 by Gabriele Cirulli <http://gabrielecirulli.com>
//

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>

#define GOAL 2048
#define SIZE 4
#define QUIT -1
#define LOSE 0
#define WIN 1
/*
 * boardLt is the canonical board; every other board reflects boardLt in its
 * respective direction.
 */

typedef struct mygame{

    int *(boardLt[SIZE][SIZE]); // left (canonical)
    int *(boardRt[SIZE][SIZE]); // right
    int *(boardUp[SIZE][SIZE]); // up
    int *(boardDn[SIZE][SIZE]); // down
    int *lastAdd; // address of last tile added
    int score;
    int win;

    void (*cleanup)(struct mygame *g);
    int (*quit)(int op,struct mygame *g);
    void (*printBoard)(struct mygame *g);
    void (*addRandom)(struct mygame *g);
    int (*move)(struct mygame *g);
    bool (*slide)(int *b[SIZE][SIZE],struct mygame *g);
    bool (*isFull)(struct mygame *g);
}mygame;

/*
* initialize struct
*/

mygame* initialize(){
    mygame* g = malloc(sizeof(mygame));
    g->lastAdd = 0;
    g->score = 0;
    g->win = 0;
        for (int r = 0; r < SIZE; r++) { // row for canonical board
            for (int c = 0; c < SIZE; c++) { // column for canonical board
                int *tile = (int *) malloc(sizeof(int));
                if (tile == NULL){
                    perror("malloc returned NULL");
                    exit(1);
                }
                *tile = 0;

                // add tile to equivalent location in all directional boards
                g->boardLt[r][c]        = tile;
                g->boardRt[r][SIZE-1-c] = g->boardLt[r][c];
                g->boardUp[SIZE-1-c][r] = g->boardLt[r][c];
                g->boardDn[c][SIZE-1-r] = g->boardLt[r][c];
            }
        }

    return g;
}

/*
 * Free boards using boardLt.
 */

void
cleanup(struct mygame *g) {
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            free(g->boardLt[r][c]);
        }
    }
}

/*
 * Print outcome and exit gracefully.
 */

int
quit(int op,struct mygame *g) {
    const char *msg = op == 1 ? "YOU WIN!": op == 0 ? "GAME OVER." : "QUIT";

    printf("\n\n%s\n", msg);
    cleanup(g);
    exit(0);
}

/*
 * Signal handler function for SIGINT.
 */

void
terminate(int signum) {
    printf("Terminate %d\n",signum);
    exit(0);    
    //quit(QUIT);   //quit need parameter
}

/*
 * Print the specified board.
 */

void
printBoard(struct mygame *g) {
    printf("\n2048\n\nSCORE: %d\n\n", g->score);
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (*g->boardLt[r][c]) {
                if (g->lastAdd == g->boardLt[r][c])
                    printf("\033[036m%6d\033[0m", *g->boardLt[r][c]); // with color
                else
                    printf("%6d", *g->boardLt[r][c]);
            } else {
                printf("%6s", ".");
            }
        }
        printf("\n\n");
    }

    printf("\nMOVEMENT:\n   w\n a s d              ");
}

/*
 * Add tile to random open space on board.
 * Tile is a 2 or a 4, randomly chosen.
 */

void
addRandom(struct mygame *g) {
    int r, c;
    do {
        r = rand()%4;
        c = rand()%4;
    } while (*g->boardLt[r][c] != 0);
    int two_or_four = 2 * rand()%2 + 2;
    *g->boardLt[r][c] = two_or_four;
    g->lastAdd = g->boardLt[r][c];
}

/*
 * Slide all tiles left on the specified board.
 */

bool
slide(int *b[SIZE][SIZE],struct mygame *g) {
    bool success = 0; // true if something moves
    int marker = -1;  // marks a cell one past location of a previous merge

    for (int r = 0; r < SIZE; r++) { // rows
        for (int c = 1; c < SIZE; c++) { // don't need to slide first col
            if (!(*b[r][c])) continue; // no tile

            // advance to proper merge target in new column (newc)
            int newc = c;
            while (newc && (!*b[r][newc-1] || *b[r][newc-1] == *b[r][c])) {
                newc--;
                if (*b[r][newc] == *b[r][c] || marker == newc) break;
            }

            // merge tile with target tile
            if (newc != c) {
                if (*b[r][newc]) {
                    g->score += *b[r][newc] += *b[r][c];
                    marker = newc+1;
                    if (*b[r][newc] == GOAL) g->win = true;
                } else {
                    *b[r][newc] += *b[r][c];
                }

                *b[r][c] = 0;
                success = true;
            }
        }
        marker = -1; // reset for next row
    }

    return success;
}

/*
 * Get and act on next move from user.
 */

int
move(struct mygame *g) {
    bool success = false;

    char direction = getchar();
    switch(direction) {
        case 119:      // 'w' key; up
            success = slide(g->boardUp,g);
            break;
        case 97:       // 'a' key; left
            success = slide(g->boardLt,g);
            break;
        case 115:       // 's' key; down
            success = slide(g->boardDn,g);
            break;
        case 100:       // 'd' key; right
            success = slide(g->boardRt,g);
            break;
        case 113:       // 'q' key; quit
            quit(QUIT,g);
            break;
            //     default:
            //         success = false;
    }

    return success;
}

/*
 * Check for possible moves.
 *
 * Return: true if no moves are possible
 *         false if a move exists
 */

bool
isFull(struct mygame *g) {
    for (int r = SIZE-1; r >= 0; r--) {
        for (int c = SIZE-1; c >= 0; c--) {

            // check tile above where there is a row above
            if (r &&
                (*g->boardLt[r-1][c] == 0 ||
                 *g->boardLt[r-1][c] == *g->boardLt[r][c]))
                return false;

            // check tile to left where there is a column to the left
            if (c &&
                (*g->boardLt[r][c-1] == 0 ||
                 *g->boardLt[r][c-1] == *g->boardLt[r][c]))
                return false;
        }
    }
    return true; // no possible moves found
}

/*
 * Main.
 */

int
main(){
    mygame* g=initialize();

    g->cleanup = cleanup;
    g->quit = quit;
    g->printBoard = printBoard;
    g->addRandom = addRandom;
    g->move = move;
    g->slide = slide;
    g->isFull = isFull;

    srand(time(NULL));         // seed random number
    signal(SIGINT, terminate); // set up signal to handle ctrl-c
    system("stty cbreak");     // read user input immediately
    g->addRandom(g);

    for (;;) {
        printf("\e[1;1H\e[2J"); // clear screen
        g->addRandom(g);
        g->printBoard(g);
        if (g->isFull(g)) break;
        if (g->win) quit(WIN,g);
        while(!g->move(g))
            ;
    }

    quit(LOSE,g); // game over
    g->cleanup(g);  // not reached
    return 0;
}