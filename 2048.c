#include "2048.h"

/* 추가함
* 위 함수는 mygame구조체를 초기화한다
*/
mygame* initialize(){
    mygame* g = malloc(sizeof(mygame));
    g->score = (int*) malloc(sizeof(int));
    g->win = (int*) malloc(sizeof(int));

    g->lastAdd = 0;
    *g->score = 0;
    *g->win = 0;

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

    g->cleanup = cleanup;
    g->quit = quit;
    g->printBoard = printBoard;
    g->addRandom = addRandom;
    g->move = move;
    g->slide = slide;
    g->isFull = isFull;
    g->play = play;
    return g;
}


mygame* getObject()
{
    static mygame *object = NULL;

    if(object==NULL)
        object = initialize();

    return object;
}


/*
 * 위 함수는 메모리를 해제한다
 */
void
cleanup(struct mygame *g) {
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            free(g->boardLt[r][c]);
        }
    }
    free(g->score);
    free(g->win);
    free(g);
}

/*
 * 위 함수는 종료 루틴을 수행한다
 */
int
quit(int op,struct mygame *g) {
    const char *msg = op == 1 ? "YOU WIN!": op == 0 ? "GAME OVER." : "QUIT";

    printf("\n\n%s\n", msg);
    cleanup(g);
    exit(0);
}

/*
 * 위 함수는 게임 화면을 출력한다
 */
void
printBoard(int *boardLt[SIZE][SIZE], int *lastAdd, int *score) {
    printf("\n2048\n\nSCORE: %d\n\n", *score);
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (*boardLt[r][c]) {
                if (lastAdd == boardLt[r][c])
                    printf("\033[036m%6d\033[0m", *boardLt[r][c]); // with color
                else
                    printf("%6d", *boardLt[r][c]);
            } else {
                printf("%6s", ".");
            }
        }
        printf("\n\n");
    }

    printf("\nMOVEMENT:\n   w\n a s d              ");
}

/*
 * 위 함수는 새로운 난수를 생성한다
 * 수정함
 * int형 변수 two_or_four가 숫자 2만 생성하는 버그 수정함
 */
void
addRandom(int *boardLt[SIZE][SIZE], int *lastAdd) {
    int r, c;
    srand(time(NULL));
    do {
        r = rand()%4;
        c = rand()%4;
    } while (*boardLt[r][c] != 0);
    int two_or_four = 2 * (rand()%2) + 2;
    *boardLt[r][c] = two_or_four;
    lastAdd = boardLt[r][c];
}

/*
 * 위 함수는 입력에 따른 타일 이동을 수행한다
 */
bool
slide(int *b[SIZE][SIZE],int *score, int *win) {
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
                    *score += *b[r][newc] += *b[r][c];
                    marker = newc+1;
                    if (*b[r][newc] == GOAL) *win = true;
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
 * 위 함수는 입력에 따른 타일이동 함수를 호출한다
 * 수정됨
 * caps lock이 켜져있을 시 작업이 수행되지 않은 부분 수정
 */
int
move(struct mygame *g) {
    bool success = false;

    char direction = getchar();
    switch(direction) {
        case 119:      // 'w' key; up
        case 87:
            success = slide(g->boardUp, g->score, g->win);
            break;
        case 97:       // 'a' key; left
        case 65:
            success = slide(g->boardLt, g->score, g->win);
            break;
        case 115:       // 's' key; down
        case 83:
            success = slide(g->boardDn, g->score, g->win);
            break;
        case 100:       // 'd' key; right
        case 68:
            success = slide(g->boardRt, g->score, g->win);
            break;
        case 113:       // 'q' key; quit
        case 81:
            quit(QUIT,g);
            break;
            //     default:
            //         success = false;

    }

    return success;
}

/*
 * 위 함수는 움직일 수 있는지 확인한다
 */
bool
isFull(int *boardLt[SIZE][SIZE]) {
    for (int r = SIZE-1; r >= 0; r--) {
        for (int c = SIZE-1; c >= 0; c--) {

            // check tile above where there is a row above
            if (r &&
                (*boardLt[r-1][c] == 0 ||
                 *boardLt[r-1][c] == *boardLt[r][c]))
                return false;

            // check tile to left where there is a column to the left
            if (c &&
                (*boardLt[r][c-1] == 0 ||
                 *boardLt[r][c-1] == *boardLt[r][c]))
                return false;
        }
    }
    return true; // no possible moves found
}

/*
 * 위 함수는 게임 실행을 담당한다
 */
void
play(struct mygame *g){
    g->addRandom(g->boardLt, g->lastAdd);

    for (;;) {
        printf("\e[1;1H\e[2J"); // clear screen
        g->addRandom(g->boardLt, g->lastAdd);
         g->printBoard(g->boardLt, g->lastAdd, g->score);

        if (g->isFull(g->boardLt)) break;
       if (*g->win) quit(WIN,g);
       while(!g->move(g));
    }

    quit(LOSE,g); // game over
    g->cleanup(g);  // not reached

}
