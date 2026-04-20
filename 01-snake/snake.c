#include <stdio.h>

#define ROWS 6
#define COLS 18

int main(void) {
  printf("Hello, snake!\n");

  char board[ROWS][COLS];
  
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      if (r == 0 || r == ROWS - 1 || c == 0 || c == COLS - 1) {
        board[r][c] = '#';
        putchar(board[r][c]);
      } else {
        board[r][c] = '.';
        putchar(board[r][c]);
      }
    }
    putchar('\n');
  }


  return 0;
}