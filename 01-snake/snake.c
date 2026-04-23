#include <windows.h>
#include <stdio.h>
#include <conio.h>

#define ROWS 8
#define COLS 18

int main(void) {

  // 한글 인코딩
  SetConsoleOutputCP(65001);

  printf("Hello, snake!\n");

  // board 정의
  char board[ROWS][COLS];

  // for (int r = 0; r < ROWS; r++) {
  //   for (int c = 0; c < COLS; c++) {
  //     if (r == 0 || r == ROWS - 1 || c == 0 || c == COLS - 1) {
  //       board[r][c] = '#';
  //       putchar(board[r][c]);
  //     } else {
  //       board[r][c] = '.';
  //       putchar(board[r][c]);
  //     }
  //   }
  //   putchar('\n');
  // }
  
  // 프레임 종료 시마다 이동할 칸 수 설정(기본은 우측으로 전진)
  int dr = 0, dc = 1;
  
  // 보드 채우기 -> 보드에 플레이어 찍기 -> 보드 출력
  // 루프에 안넣음(최적화)
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      if (r == 0 || r == ROWS - 1 || c == 0 || c == COLS - 1) {
        board[r][c] = '#';
      } else {
        board[r][c] = '.';
      }
    }
  }

  int player_r = ROWS / 2;
  int player_c = COLS / 2;
  int prev_r = player_r;
  int prev_c = player_c;
  
  while (1) {
    // 화면 전체 지우기
    // printf("\033[2J\033[H");

    // 커서만 맨 위로(차분 렌더링)
    printf("\033[H");
    
    // Differential Rendering - 보드 전체 초기화보다 성능 좋은듯?
    // board[player_r - dr][player_c - dc] = '.'; // 불완전
    // int prev_r = player_r;
    // int prev_c = player_c;
    board[prev_r][prev_c] = '.';
    
    // 정가운데. 출력 루프 돌기 전
    board[player_r][player_c] = 'O';

    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLS; c++) {
        // 출력
        putchar(board[r][c]);
      }
      putchar('\n');
    }
    
    printf("이동: wasd, 종료: q\n");
    
    // 키 입력받기
    // int key = getchar(); // 블로킹 함수 getchar()
    if (_kbhit()) {         // 논블로킹 함수로 교체
      int key = _getch();
      // 키별 분기
      switch (key) {
        case 'w':
        // player_r--;
        dr = -1, dc = 0;
        break;
        case 's':
        // player_r++;
        dr = 1, dc = 0;
        break;
        case 'a':
        // player_c--;
        dr = 0, dc = -1;
        break;
        case 'd':
        // player_c++;
        dr = 0, dc = 1;
        break;
        case 'q':
        return 0;
      }
    }
    
    // int c;
    
    // stage 3부터 뱀이 알아서 움직이도록 구현. 엔터 안 써서 주석처리
    // 입력받은 Enter(\n)는 버리기 - C의 관용구급 패턴
    // while ((c = getchar()) != '\n' && c != EOF) {
    //   // 아무것도 안 함 -> 그냥 버림
    // }

    prev_r = player_r;
    prev_c = player_c;
    // 매 프레임 전진
    player_r += dr;
    player_c += dc;

    // 벽 clamp (switch보다 뒤에 와야 함)
    if (player_r < 1) player_r = 1;
    if (player_r > ROWS - 2) player_r = ROWS - 2;
    if (player_c < 1) player_c = 1;
    if (player_c > COLS - 2) player_c = COLS - 2;

    // test
    printf("pos: %d, %d\n", player_r, player_c);
    
    // 200ms, 5 FPS
    Sleep(200);
  }
  

  return 0;
}