# Stage 2 — 키 한 번 받아 한 칸 이동

> 보드 한가운데에 `O`를 놓고, `w/a/s/d`로 이동시킨다. 처음으로 **사용자 입력**과 **반복 갱신 화면**을 만진다.

## 오늘의 목표

- 보드에 플레이어 `O`를 그린다.
- `w`(위) / `a`(왼쪽) / `s`(아래) / `d`(오른쪽)를 누르고 Enter를 치면 한 칸 이동.
- 벽에 부딪히면 더 이상 안 나간다 (clamp).
- `q`를 누르면 종료.

이 단계에서 만나는 건 정작 게임이 아니라 **터미널이 어떻게 글자를 다루는가**다. 키 한 개를 받는데도 `getchar`가 Enter를 기다리는 것도, 화면을 깨끗이 다시 그리는 것도, 한글이 깨지는 것도 — 다 터미널과 OS의 동작 방식 때문.

---

## 배경 이야기

### 표준 입출력은 "파일처럼" 다뤄진다

프로세스가 태어나면 OS는 세 개의 **스트림**을 자동으로 끼워준다:
- `stdin` (표준 입력, fd 0) — 보통 키보드
- `stdout` (표준 출력, fd 1) — 보통 화면
- `stderr` (표준 에러, fd 2) — 보통 화면

이름은 다르지만 **모두 파일과 같은 인터페이스**(읽고, 쓰고, 닫고)로 다뤄진다. 그래서 `./snake > out.txt`로 명령을 치면 `stdout`이 화면 대신 파일을 가리키게 되고, 우리 코드는 한 줄도 바꾸지 않은 채로 출력이 파일로 간다. **"모든 것은 파일이다"** — 유닉스 철학의 첫 만남.

### 라인 버퍼링 — `getchar`가 Enter를 기다리는 이유

`getchar()`를 부르면 한 글자가 바로 들어올 것 같지만, 실제로는 사용자가 **Enter를 칠 때까지** 멈춰서 기다린다. 왜일까?

터미널은 사용자가 입력 도중에 **Backspace**로 글자를 지우거나 줄을 수정할 여지를 주려고, 한 줄이 완성될 때까지 입력을 보내지 않고 자기 안에 모아둔다. 이게 **라인 버퍼링(line buffering)** 이다. Enter를 치면 그제야 모은 줄이 `stdin`으로 흘러간다.

그래서 `wd<Enter>`를 치면 `getchar()`는 `'w'`만 받고, **`'d'`와 `'\n'`은 stdin에 그대로 남는다**. 다음 `getchar()`가 그 잔여를 마저 먹는다. 우리는 이걸 **버리는 루프** 한 줄로 처리할 것이다 (코드 보면 나옴).

Stage 3에서는 이 라인 버퍼링을 우회해서 키를 즉시 받는 방법을 다룬다. 오늘은 일단 "Enter를 친다"를 받아들이고 간다.

### ANSI escape — 터미널을 조종하는 작은 명령어

`printf("Hello")`는 글자를 그대로 찍는다. 그런데 `printf("\033[2J")`는 글자 대신 **화면 전체를 지운다**. 어떻게?

터미널은 단순한 글자 표시기가 아니다. 작은 **상태 기계**다. 평소엔 들어온 바이트를 그냥 화면에 찍지만, **이스케이프 시퀀스**(`ESC` 문자 `\033` + `[` + 명령)를 만나면 그건 출력 대신 **명령**으로 해석한다.

자주 쓰는 것 몇 개:
- `\033[2J` — 화면 전체 지우기
- `\033[H` — 커서를 화면 맨 위 왼쪽(home)으로
- `\033[31m` — 이후 글자 빨강 / `\033[0m` — 색 초기화

이 단계에서는 매 프레임 `\033[2J\033[H`로 **화면을 깨끗이 지우고 처음부터 다시 그리는** 풀-리드로(full-redraw) 패턴을 쓴다. Stage 3에서 더 똑똑한 방법으로 바꾼다.

### 한글이 깨지는 이유 — 문자 인코딩

소스 파일은 보통 UTF-8로 저장된다. 그런데 Windows 콘솔의 기본 출력 코드페이지는 한국 환경에서 CP949(EUC-KR류)다. **같은 바이트가 두 인코딩에서 다른 글자로 해석**되니, 콘솔이 UTF-8 한글을 CP949로 읽으면 깨진 글자가 나온다.

해결: 프로그램 시작 시 `SetConsoleOutputCP(65001)` 한 줄. 65001은 UTF-8의 코드페이지 번호. 이 함수는 Windows API라 `#include <windows.h>`가 필요하다.

---

## 새 C 문법

### `#include <windows.h>`
- Windows API를 쓰기 위한 헤더. **플랫폼 종속** 코드가 처음 등장.
- `SetConsoleOutputCP`, `Sleep`(Stage 3) 등이 여기서 옴.

### `SetConsoleOutputCP(65001);`
- 콘솔 출력 코드페이지를 UTF-8로 변경. `main` 진입 직후에 호출.

### `while (1) { ... }`
- 조건이 항상 참 → **무한 루프**. 안에서 `break` 또는 `return`으로 빠져나간다.

### `getchar()`
- `stdin`에서 한 글자를 읽는다. 반환 타입은 `char`가 아니라 **`int`**.
  - 왜 `int`? `EOF`(파일 끝, 값은 보통 `-1`)를 "어떤 글자와도 다른 값"으로 표현하려고. `char`로 받으면 EOF와 정상 글자가 충돌할 수 있다.

### `switch (값) { case 'w': ...; break; ... }`
- 값에 따른 분기. 정수 또는 정수로 취급되는 값에만 쓸 수 있다 (`char`도 정수 취급이라 OK).
- **`break`를 빼면 다음 case로 흘러넘친다(fall-through).** 일부러 그렇게 쓰는 경우도 있지만, 처음엔 항상 `break` 붙여두는 게 안전.
- `default:` 절을 넣으면 어느 case에도 안 걸렸을 때 실행.

### `if ( (c = getchar()) != '\n' && c != EOF )`
- C의 관용구. 대입(`=`)이 식 안에서 값을 가진다는 성질을 이용해, "읽으면서 동시에 비교".
- 처음 보면 헷갈리지만 자주 보게 됨. 의미: "한 글자를 읽어 `c`에 넣고, 그게 `\n`도 아니고 `EOF`도 아닌 동안".

### `printf("\033[2J\033[H")`
- 화면 지우기 + 커서 홈. 이스케이프 시퀀스를 그대로 문자열에 넣는다.
- `\033`은 **8진수 표기**의 `033` = 27 = `ESC` 문자.

### 한 문장 if (중괄호 생략)
- `if (player_r < 1) player_r = 1;` — 본문이 한 문장이면 `{}` 생략 가능.
- 두 문장 이상이거나, `else`와 결합되면 `{}` 권장.

---

## 코드

`01-snake/snake.c`를 다음으로 덮어쓴다.

**Stage 1 → Stage 2 변경점**
- **추가**: `#include <windows.h>` — Windows API 헤더 (`SetConsoleOutputCP` 위해).
- **추가**: `SetConsoleOutputCP(65001);` — 콘솔 출력 UTF-8로(한글 깨짐 방지).
- **추가**: `int player_r = ROWS/2; int player_c = COLS/2;` — 플레이어 좌표(보드 한가운데).
- **추가**: `while (1) { ... }` 메인 루프 — 화면 지우기(`\033[2J\033[H`), 매 프레임 보드 재칠하기 + `O` 박기, `getchar()` 입력 + 잔여 비우기, switch wasd/q 분기, clamp.
- **유지**: `#include <stdio.h>`, ROWS/COLS 매크로, `char board[ROWS][COLS];`, 첫 보드 그리기 이중 for(그대로), `int main(void)`, `return 0;`.
- (stage-1에서 줄 단위로 사라진/바뀐 코드가 없어 코드 안 흔적은 없음 — 추가만.)

```c
#include <windows.h>   // Windows API — SetConsoleOutputCP 사용
#include <stdio.h>     // 표준 입출력 — printf, putchar, getchar

#define ROWS 6
#define COLS 18

int main(void) {
  // 콘솔 출력 인코딩을 UTF-8로. 한글 깨짐 방지.
  // 코드페이지 65001 = UTF-8. Windows 콘솔은 기본이 CP949(한국)라 안 맞춘다.
  SetConsoleOutputCP(65001);

  printf("Hello, snake!\n");

  // 보드 선언. 내용물은 아래 루프에서 채운다.
  char board[ROWS][COLS];

  // 보드 첫 그림(루프 진입 전 한 번).
  // 이 블록은 곧바로 화면에 찍기까지 함 — Stage 1과 동일한 패턴.
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

  // 플레이어 좌표 — 보드의 정가운데에서 시작.
  // 정수 나눗셈이라 ROWS=6 → 3, COLS=18 → 9.
  int player_r = ROWS / 2;
  int player_c = COLS / 2;

  // 메인 루프: 키 한 번 받기 → 이동 → 화면 갱신 반복.
  // 종료는 'q' 입력 시 return으로.
  while (1) {
    // ANSI escape: 화면 전체 지우기(\033[2J) + 커서 홈으로(\033[H).
    // 매 프레임 처음부터 다시 그리는 "풀-리드로" 방식.
    printf("\033[2J\033[H");

    // 보드를 다시 채우면서 출력.
    // 매번 board 전체를 새로 칠하는 이유: 플레이어가 지나간 자리를 '.'로 되돌리려고.
    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLS; c++) {
        if (r == 0 || r == ROWS - 1 || c == 0 || c == COLS - 1) {
          board[r][c] = '#';
        } else {
          board[r][c] = '.';
        }
        // 플레이어 좌표면 'O'로 덮어쓴다.
        // 안쪽 루프에서 검사하는 방식 — 매 칸마다 비교라 살짝 비효율적이지만 코드는 단순.
        board[player_r][player_c] = 'O';
        putchar(board[r][c]);
      }
      putchar('\n');
    }

    printf("이동: wasd, 종료: q\n");

    // 키 한 글자 받기.
    // 주의: getchar는 Enter를 칠 때까지 기다린다 (라인 버퍼링).
    int key = getchar();

    // 남은 글자 + 개행(\n) 버리기.
    // 사용자가 'wd<Enter>'처럼 두 글자를 친 경우 'd'와 '\n'이 stdin에 남는다.
    // 다음 getchar가 그 잔여를 먹지 않도록 미리 비운다.
    // C에서 stdin을 줄 단위로 다룰 때의 관용구.
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
      // 아무것도 안 함 — 그냥 읽어서 버린다.
    }

    // 키별 분기.
    // 좌표 한 칸 변경. 음수가 될 수 있으니 아래에서 clamp가 필요.
    switch (key) {
      case 'w':                  // 위 = 행이 줄어드는 방향
        player_r--;
        break;
      case 's':                  // 아래
        player_r++;
        break;
      case 'a':                  // 왼쪽 = 열이 줄어드는 방향
        player_c--;
        break;
      case 'd':                  // 오른쪽
        player_c++;
        break;
      case 'q':                  // 종료
        return 0;                // main에서 return → 프로세스 종료
    }

    // 벽 clamp — 항상 switch 뒤에 둔다.
    // 유효 범위: 1 ~ ROWS-2 / 1 ~ COLS-2 (테두리는 # 이므로 그 안쪽).
    // switch에서 좌표를 먼저 바꾼 다음 여기서 가두는 순서가 자연스럽다.
    if (player_r < 1) player_r = 1;
    if (player_r > ROWS - 2) player_r = ROWS - 2;
    if (player_c < 1) player_c = 1;
    if (player_c > COLS - 2) player_c = COLS - 2;
  }

  return 0;
}
```

> **clamp 위치가 왜 switch 뒤인가?** 만약 앞에 두면, 이전 프레임 끝의 좌표를 가두는 셈이라 새 입력으로 빠져나간 좌표는 못 막는다. **"바꾼 다음 가둔다"** 순서가 핵심.

---

## 돌려보기

```sh
gcc 01-snake/snake.c -o 01-snake/snake
./01-snake/snake
```

조작:
- `w` + Enter → 위로 한 칸
- `a` + Enter → 왼쪽으로
- `s` + Enter → 아래로
- `d` + Enter → 오른쪽으로
- `q` + Enter → 종료

`O`가 벽까지 가면 더 안 나가는지, `q`를 누르면 정상 종료되는지 확인.

---

## 흔히 빠지는 함정

- **한글이 깨진다.** `SetConsoleOutputCP(65001);`을 빠뜨렸거나, `main`의 다른 출력 뒤에 호출했다. 반드시 `main` 시작 직후.
- **`switch`에서 `break`를 빼먹는다.** `'w'`만 눌렀는데 `player_r--`와 `player_r++`가 둘 다 실행되는 등 황당한 동작. `break` 빠뜨림은 워낙 흔해서 컴파일러에 따라 경고를 켜는 옵션(`-Wimplicit-fallthrough`)도 있다.
- **clamp를 switch 앞에 둔다.** 새로 이동한 좌표를 못 막아서 `O`가 벽 밖으로 사라짐.
- **`getchar()`로 글자 하나만 처리하고 끝.** 다음 입력 때 잔여 글자가 한꺼번에 처리되어 `O`가 두 칸 점프하는 듯한 버그. 잔여 비우는 루프가 그래서 있다.
- **`if (key = 'q')`처럼 `=`를 씀.** `==`가 맞음. 컴파일은 통과하므로 가장 짜증나는 종류의 버그.

---

## 체크 질문

소리 내어 답해보기:

1. 왜 `getchar()`는 Enter를 눌러야 반응할까? **라인 버퍼링**의 의미를 한 문장으로.
2. `stdin`, `stdout`은 파일이 아닌데 왜 파일처럼 다뤄질까? "모든 것은 파일이다" 한 마디로.
3. `switch`에서 `break`를 빼면 어떤 일이 일어나지? 의도적으로 그렇게 쓰는 경우도 있을까?
4. `\033[2J\033[H`는 어떤 두 가지 동작을 합쳐 놓은 거지? 터미널이 이 바이트를 "글자가 아니라 명령"으로 보는 이유는?
5. 같은 한글 문자열이 왜 콘솔에선 깨지고 파일에선 멀쩡할까? `SetConsoleOutputCP`가 정확히 무엇을 바꾸지?
6. clamp 코드를 switch **앞**으로 옮기면 무엇이 깨질까?
7. `getchar()`의 반환 타입이 `char`가 아니라 `int`인 이유는?

---

## 다음 단계 예고

[Stage 3](stage-3.md) — 키를 안 누르고 가만히 있어도 뱀이 **알아서 한 칸씩 전진**한다. 그러려면 `getchar`가 더 이상 Enter를 기다리면 안 된다. **블로킹 vs 논블로킹**, **게임 루프**, 그리고 매 프레임 전체를 다시 그리지 않는 **차분 렌더링**까지.
