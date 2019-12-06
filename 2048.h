
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

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
    int *score;
    int *win;

    void (*cleanup)(struct mygame *g);
    int (*quit)(int op,struct mygame *g);
    void (*printBoard)(int *b[SIZE][SIZE], int *lastAdd, int *score);
    void (*addRandom)(int *boardLt[SIZE][SIZE], int *lastAdd);
    int (*move)(struct mygame *g);
    bool (*slide)(int *b[SIZE][SIZE],int *score, int *win);
    bool (*isFull)(int *boardLt[SIZE][SIZE]);
    void (*play)(struct mygame *g);
}mygame;

mygame * initialize();
mygame * getObject();

void cleanup(struct mygame *g);
int quit(int op,struct mygame *g);
void printBoard(int *b[SIZE][SIZE], int *lastAdd, int *score);
void addRandom(int *boardLt[SIZE][SIZE], int *lastAdd);
bool slide(int *b[SIZE][SIZE],int *score, int *win);
int move(struct mygame *g);
bool isFull(int *boardLt[SIZE][SIZE]);
void play(struct mygame *g);
