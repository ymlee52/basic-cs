#include <windows.h>
#include <stdio.h>

#define ROWS 6
#define COLS 18

int main(void) {

  // 한글 인코딩
  SetConsoleOutputCP(65001);

  printf("Hello, snake!\n");

  // board 정의
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

  
  
  int player_r = ROWS / 2;
  int player_c = COLS / 2;
  
  while (1) {
    // 화면 지우기
    printf("\033[2J\033[H");
    
    // 보드 채우기 -> 보드에 플레이어 찍기 -> 보드 출력
    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLS; c++) {
        if (r == 0 || r == ROWS - 1 || c == 0 || c == COLS - 1) {
          board[r][c] = '#';
        } else {
          board[r][c] = '.';
        }
        // 정가운데. 출력 루프 돌기 전
        board[player_r][player_c] = 'O';
        // 출력
        putchar(board[r][c]);
      }
      putchar('\n');
    }
    
    printf("이동: wasd, 종료: q\n");
    
    // 키 받기
    int key = getchar();
    int c;

    // 입력받은 Enter(\n)는 버리기 - C의 관용구급 패턴
    while ((c = getchar()) != '\n' && c != EOF) {
      // 아무것도 안 함 -> 그냥 버림
    }

    // 키별 분기
    switch (key) {
      case 'w':
      player_r--;
      break;
      case 's':
      player_r++;
      break;
      case 'a':
      player_c--;
      break;
      case 'd':
      player_c++;
      break;
      case 'q':
      return 0;
    }

    // 벽 clamp (switch보다 뒤에 와야 함)
    if (player_r < 1) player_r = 1;
    if (player_r > ROWS - 2) player_r = ROWS - 2;
    if (player_c < 1) player_c = 1;
    if (player_c > COLS - 2) player_c = COLS - 2;
  }
  

  return 0;
}