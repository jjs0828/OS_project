//
// 2048.c
//
// Copyright (c) 2014 Ben Kogan <http://benkogan.com>
// Gameplay based on 2048 by Gabriele Cirulli <http://gabrielecirulli.com>
//

//수정사항 확인에 대한 수정 주석입니다.

/*
 * 시그널 제어에 따른 헤더파일 추가
 * 소스코드 분할에 따른 헤더파일 추가
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "2048.h"
/* 
 * 위 함수는 종료를 실행한다
 * 수정 내역
 * 게임 데이터를 받는 부분이 전역변수에서 구조체로 수정되면서 기존 quit함수를 호출할 수 없게 됨
 * ctrl+z를 통한 종료는 메모리 누수를 발생시킴 이를 해결하기 위해 kill을 수행
 */
void
terminate(int signum) {
    printf("\nTerminate %d\n",signum); 
    mygame* g = getObject();
    g->quit(QUIT, g);
    exit(1);
}

/*
 * 위 함수는 메인이다
 */
int
main(){
    mygame* g = getObject();

    signal(SIGINT, terminate); // set up signal to handle ctrl-c
    signal(SIGTSTP, terminate); // set up signal to handle ctrl-z
    system("stty cbreak");     // read user input immediately
    g->play(g);
    return 0;
}