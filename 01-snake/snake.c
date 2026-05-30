#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 8
#define COLS 18

typedef struct {
  int r;
  int c;
} Cell;

#define MAX_LEN (ROWS * COLS)

int main(void) {

  // 한글 인코딩
  SetConsoleOutputCP(65001);

  printf("Hello, snake!\n");

  // board 정의
  char board[ROWS][COLS];
  
  int dr = 0, dc = 1;
  
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      if (r == 0 || r == ROWS - 1 || c == 0 || c == COLS - 1) {
        board[r][c] = '#';
      } else {
        board[r][c] = '.';
      }
    }
  }

  Cell *body = malloc(sizeof(Cell) * MAX_LEN);
  int length = 1;

  body[0].r = ROWS / 2;
  body[0].c = COLS / 2;

  Cell prev_tail = body[0];

  int running = 1;

  while (running) {
    printf("\033[H");

    board[prev_tail.r][prev_tail.c] = '.';
    board[body[0].r][body[0].c] = 'O';

    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLS; c++) {
        putchar(board[r][c]);
      }
      putchar('\n');
    }
    
    printf("이동: wasd, 종료: q\n");
    
    if (_kbhit()) {
      int key = _getch();
      switch (key) {
        case 'w': dr = -1, dc =  0; break;
        case 's': dr =  1, dc =  0; break;
        case 'a': dr =  0, dc = -1; break;
        case 'd': dr =  0, dc =  1; break;
        case 'q': running = 0; break;
      }
    }

    // 이동 업데이트
    prev_tail = body[length - 1];

    for (int i = length - 1; i > 0; i--) {
      body[i] = body[i - 1];
    }

    body[0].r += dr;
    body[0].c += dc;

    // clamp 업데이트
    if (body[0].r < 1)        body[0].r = 1;
    if (body[0].r > ROWS - 2) body[0].r = ROWS - 2;
    if (body[0].c < 1)        body[0].c = 1;
    if (body[0].c > COLS - 2) body[0].c = COLS - 2;
    
    // 200ms, 5 FPS
    Sleep(200);
  }
  free(body);

  return 0;
}