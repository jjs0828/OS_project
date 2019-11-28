//
// 2048.c
//
// Copyright (c) 2014 Ben Kogan <http://benkogan.com>
// Gameplay based on 2048 by Gabriele Cirulli <http://gabrielecirulli.com>
//

//수정사항 확인에 대한 수정 주석입니다.

/*
 * 시그널 제어에 따른 헤더파일 추가
 */
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>

/* 수정함
 * 게임 내의 디폴트 값
 */
#define GOAL 2048
#define SIZE 4
#define QUIT -1
#define LOSE 0
#define WIN 1

/* 추가함
 * 이 구조체는 게임 데이터와 함수를 관리한다
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


/* 추가함
* 위 함수는 mygame구조체를 초기화한다
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
 * 위 함수는 메모리를 해제한다
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
 * 위 함수는 종료를 실행한다
 * 수정 내역
 * 게임 데이터를 받는 부분이 전역변수에서 구조체로 수정되면서 기존 quit함수를 호출할 수 없게 됨
 * ctrl+z를 통한 종료는 메모리 누수를 발생시킴 이를 해결하기 위해 kill을 수행
 */
void
terminate(int signum) {
    printf("Terminate %d\n",signum);
    pid_t self = getpid();
    kill(self,SIGKILL);   
    //quit(QUIT);   //quit need parameter
}

/*
 * 위 함수는 게임 화면을 출력한다
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
 * 위 함수는 새로운 난수를 생성한다
 * 수정함
 * int형 변수 two_or_four가 숫자 2만 생성하는 버그 수정함
 */
void
addRandom(struct mygame *g) {
    int r, c;
    do {
        r = rand()%4;
        c = rand()%4;
    } while (*g->boardLt[r][c] != 0);
    int two_or_four = 2 * (rand()%2) + 2;
    *g->boardLt[r][c] = two_or_four;
    g->lastAdd = g->boardLt[r][c];
}

/*
 * 위 함수는 입력에 따른 타일 이동을 수행한다
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
            success = slide(g->boardUp,g);
            break;
        case 97:       // 'a' key; left
        case 65:
            success = slide(g->boardLt,g);
            break;
        case 115:       // 's' key; down
        case 83:
            success = slide(g->boardDn,g);
            break;
        case 100:       // 'd' key; right
        case 68:
            success = slide(g->boardRt,g);
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
 * 위 함수는 메인이다
 * 수정함
 * 전역변수에서 구조체로 바뀜에 따라 구조체 이니셜라이져 호출 및 함수포인터를 함수와 연결하도록 수정
 * 이에 따른 모든 함수의 매개변수 수정
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