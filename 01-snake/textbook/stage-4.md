# Stage 4 — 뱀 몸통 자료구조 (struct + malloc)

> 점 하나 움직이던 뱀이 **여러 칸으로 길어질 준비**를 한다. 길이가 변할 거니까 자료구조를 새로 짠다. 이 한 단계에 **구조체**, **포인터**, **동적 메모리**, **스택 vs 힙**, **배열-포인터 등가성**, **큐(FIFO)** 가 한꺼번에 등장한다. C에서 가장 중요한 만남.

## 오늘의 목표

- 좌표 두 개(`r`, `c`)를 한 덩어리로 묶는 **구조체 `Cell`**을 정의한다.
- 뱀 몸통을 `Cell *body`로 **힙에 동적 할당**한다.
- 길이는 아직 1이지만, **언제든 늘릴 수 있는 구조**로 바꿔둔다.
- 종료 시 `free(body)`로 메모리를 반환한다.

화면 동작은 Stage 3과 똑같다 — `O`가 자동 전진. 하지만 그 뒤의 자료구조가 완전히 바뀐다. 다음 단계(사과·길어짐)의 토대.

---

## 배경 이야기

### 왜 자료구조를 바꿔야 했나

Stage 3까지의 상태 변수:
```c
int player_r, player_c;     // 머리 한 칸
int prev_r, prev_c;          // 직전 머리 한 칸 (지우기용)
```

길이 1짜리 뱀에는 충분했다. 그런데 길이 5짜리 뱀을 표현하려면 좌표 5쌍이, 길이 10이면 10쌍이 필요하다. 두 가지 문제가 생긴다.

1. **변수 이름은 컴파일 시점에 고정된다.** `player_r1`, `player_r2`, ... 식으론 늘릴 수 없다.
2. **`r`과 `c`는 짝이다.** `body_r[100]`, `body_c[100]` 두 배열로 가는 것도 가능은 하지만, 좌표를 옮길 때마다 두 배열을 같이 건드려야 해서 실수가 잦다.

답:
1. `r`과 `c`를 **한 덩어리**로 묶기 → `struct Cell`.
2. 그 덩어리를 **여러 개** 보관 + 길이가 변할 수 있게 → 힙에 **`malloc`**.

### 구조체(struct) — 관련된 필드를 한 타입으로 묶기

```c
typedef struct {
  int r;
  int c;
} Cell;
```

이제 `Cell`은 새로운 **타입**이다. `int`나 `char`처럼. `Cell head;`로 변수를 만들 수 있고, `head.r`, `head.c`로 필드에 접근한다. 점(`.`)이 멤버 접근 연산자.

**핵심 성질**: `Cell a = b;`라고 쓰면 **모든 필드가 한 번에 복사**된다. `a.r = b.r; a.c = b.c;` 두 줄을 한 줄로 줄인다. Stage 4의 shift 루프 `body[i] = body[i-1]`이 이 한 줄로 좌표 한 쌍을 통째 옮기는 이유.

`typedef`는 그 묶음 타입에 **별명**을 붙이는 것. 빼면 매번 `struct Cell head;`로 `struct` 키워드를 붙여야 해서 귀찮다.

### 포인터 — "변수의 주소"를 담는 변수

```c
Cell *body;
```

`*`가 붙은 변수는 "그 타입을 가리키는 **주소**를 담는다". `body`는 Cell이 아니라, Cell이 어디 있는지 적힌 **쪽지**다.

비유: `body`는 사물함 번호 적힌 쪽지. 진짜 짐(Cell 배열)은 사물함 안에. `body[0]`은 "쪽지가 가리키는 사물함 0번 칸"이다. `*body`로도 같은 칸을 본다.

### 스택 vs 힙 — 메모리는 한 종류가 아니다

함수 안에서 `int x;`라고 쓰면 그 변수는 **스택(stack)** 에 잡힌다.
- 자동으로 잡히고 함수 끝나면 자동으로 사라진다.
- 빠르다. 하지만 크기가 작다(보통 1~8MB).
- 배열을 통째로 잡아도 거기 잡힌다 (`char board[6][18]`처럼).

`malloc`을 부르면 **힙(heap)** 에서 메모리를 빌려온다.
- 크기 제한이 사실상 없다 (시스템 RAM까지).
- 자동 해제 안 됨. **`free`로 직접 돌려줘야** 한다.
- 살짝 느리다.
- 함수가 끝나도 메모리는 살아남는다 (포인터를 다른 함수로 넘기면 거기서 계속 쓸 수 있음).

뱀 몸통은 "런타임에 크기가 변할 수 있는 데이터"라 힙이 자연스럽다. (현재 크기로는 사실 스택에 잡아도 되지만, **습관**을 들이는 게 목적.)

### `malloc(sizeof(Cell) * MAX_LEN)` — 한 줄 해부

```c
Cell *body = malloc(sizeof(Cell) * MAX_LEN);
```

오른쪽에서 왼쪽으로 읽으면:
1. `sizeof(Cell)` — 컴파일러가 "Cell 한 개가 몇 바이트야?"를 계산해 박아넣음. `int r` + `int c` = 보통 8바이트.
2. `* MAX_LEN` — 그 크기에 최대 개수를 곱해서 총 바이트 수 산출. `MAX_LEN = ROWS * COLS = 144`니까 약 1152바이트.
3. `malloc(N)` — 힙에서 N바이트짜리 공간을 빌리고 그 **시작 주소**를 반환.
4. 그 주소를 `Cell *body`에 저장.

**왜 직접 `8 * 144`로 안 쓰나?** 플랫폼/컴파일러에 따라 `int`가 4바이트가 아닐 수도 있고, struct에 패딩이 들어가면 단순 합산과 안 맞는다. `sizeof`를 쓰면 **이식성**이 보장된다.

### 배열-포인터 등가성 — C의 핵심 규칙

```c
body[0]   ≡  *(body + 0)   // 첫 번째 요소
body[i]   ≡  *(body + i)   // i번째 요소
```

C에선 **포인터에 `[]`를 붙이면 배열처럼 동작**한다. 정확히는 `body[i]`가 `*(body + i)`의 문법 설탕(syntactic sugar)이다.

`body + i`는 단순한 숫자 덧셈이 아니라 **포인터 산술**이다 — `i`개의 Cell만큼 주소가 이동한다 (`i * sizeof(Cell)` 바이트). 그래서 `body[1]`은 `body[0]` 바로 옆 Cell.

이 등가성 덕에 `malloc`으로 잡은 힙 영역을 마치 일반 배열처럼 `body[i].r`로 다룰 수 있다.

### 큐(Queue, FIFO) — 뱀 몸통의 자연스러운 모델

뱀이 한 칸 움직인다는 건 무슨 뜻일까?

```
이동 전: [H][A][B][C][T]      H=머리, T=꼬리
이동 후:    [H'][H][A][B][C]   새 머리가 앞에, 꼬리는 사라짐
```

머리 쪽에 새로 들어오고, 꼬리 쪽에서 빠진다. 이게 **큐(Queue)**, 정확히는 **FIFO(First In First Out)** 다. 줄을 서서 들어온 순서대로 빠지는 구조.

코드로 옮기면:

```c
// 1. 다음 프레임에 지울 꼬리 좌표를 저장
prev_tail = body[length - 1];

// 2. 몸통을 뒤에서 앞으로 한 칸씩 당긴다 (shift)
for (int i = length - 1; i > 0; i--) {
  body[i] = body[i - 1];      // struct 통째 복사
}

// 3. 머리만 방향대로 이동
body[0].r += dr;
body[0].c += dc;
```

길이 1이면 shift 루프는 한 번도 안 돈다. 그래도 코드는 그대로 일반화돼 있어 길이 N으로 확장돼도 그대로 작동한다.

### shift 루프 방향 — 왜 뒤에서 앞으로 가야 하나

```c
for (int i = length - 1; i > 0; i--) body[i] = body[i - 1];
```

뒤에서 앞으로 도는 이유:
```
초기:    [A][B][C]
앞→뒤로 i=1부터: body[1]=body[0] →  [A][A][C]   ← B를 잃었다!
                  body[2]=body[1] →  [A][A][A]   ← 망함

뒤→앞으로 i=2부터: body[2]=body[1] →  [A][B][B]
                  body[1]=body[0] →  [A][A][B]   ← OK
```

아직 안 옮긴 칸을 먼저 덮어쓰면 데이터를 잃는다. **항상 "쓰는 자리에 있는 값을 더 이상 안 쓸 때까지 기다린다"** — 큐/배열 shift의 일반 원리.

(memmove처럼 잘 만든 함수는 방향을 자동 판단해주지만, 손으로 짤 땐 직접 신경 써야 한다.)

### 메모리 누수와 `free` 위치 — 빠지면 안 되는 한 줄

```c
free(body);
```

`malloc`한 메모리는 OS가 자동으로 회수하지 않는다. `free`로 직접 돌려줘야 한다. 빼먹으면 **메모리 누수(memory leak)** — 프로그램이 계속 도는 동안 힙에 짐이 쌓인다.

**짧은 프로그램은 별 문제 없다**. 프로세스가 끝나면 OS가 그 프로세스의 힙을 통째 회수하니까. 하지만 장기 실행 서버에서 누수는 치명적이다. 며칠 돌리면 메모리가 다 차서 죽는다. "**`malloc` 짝에는 `free`**"는 그래서 습관으로 들여야 한다.

`free`의 **위치도 중요**하다. `while(1)`로 무한 루프를 도는데 `case 'q': return 0;`로 함수에서 곧장 빠져나가면, 루프 뒤의 `free(body);`는 **영영 실행되지 않는다**. 종료 경로가 여러 군데 생길 가능성을 고려해서 **`while` 끝에 한 군데**로 모으는 게 깔끔하다.

```c
int running = 1;
while (running) {
  ...
  case 'q': running = 0; break;   // 깃발만 내림
  ...
}
free(body);   // 루프 빠진 뒤 한 번만
return 0;
```

이렇게 두면 종료 사유가 늘어나도(게임오버, 충돌 등) `free` 자리를 옮길 필요가 없다.

---

## 새 C 문법

### `#include <stdlib.h>`
- `malloc`, `free` 등 표준 라이브러리 함수.

### `typedef struct { int r; int c; } Cell;`
- 익명 구조체에 `Cell`이라는 별명을 붙임.
- 이후 `Cell x;` 로 변수 선언. `struct` 키워드 생략 가능.

### `Cell x = {3, 5};` / `x.r`, `x.c`
- 구조체 초기화와 멤버 접근. 점 연산자 `.`.

### `Cell a = b;`
- 같은 타입 구조체끼리 **통째 복사**. 모든 필드가 한 번에 복사됨.

### `Cell *body;`
- "Cell을 가리키는 포인터". `Cell body;`(본체)와 **다른** 타입.

### `malloc(sizeof(Cell) * N)`
- 힙에서 N개 분량의 바이트를 빌려옴. 시작 주소(`void *`)를 반환 → `Cell *`로 암묵 변환.
- 메모리가 부족하면 `NULL` 반환. 엄격하게 짠다면 `if (body == NULL) return 1;` 같은 체크가 있어야 함 (지금은 일단 생략).

### `sizeof(타입)`
- 타입의 바이트 수. 컴파일 시점에 계산되는 상수.

### `body[i]` ≡ `*(body + i)`
- 포인터에 `[]` 접근하면 배열처럼 동작. C의 핵심 규칙.

### `body[i].r`
- 포인터 인덱싱 → 구조체 → 멤버 접근까지 한 줄에 연결.

### `free(ptr);`
- `malloc`으로 빌린 메모리를 OS에 반환. 한 번만 호출. 두 번 호출(double free)은 undefined behavior.

### `#define MAX_LEN (ROWS * COLS)`
- 매크로 안에서 식 사용 가능. **괄호로 감싸두는 게 안전** — 매크로 치환 후 우선순위 꼬임 방지.

---

## 코드

`01-snake/snake.c`를 다음으로 덮어쓴다. (Stage 3 코드를 잔재 없이 갈아엎는다.)

```c
#include <windows.h>   // Sleep, SetConsoleOutputCP
#include <stdio.h>     // printf, putchar
#include <conio.h>     // _kbhit, _getch
#include <stdlib.h>    // malloc, free
#include <time.h>      // 나중에 사과 랜덤 배치용 (이번 단계엔 안 씀)

#define ROWS 8
#define COLS 18

// r(행)과 c(열)를 한 덩어리로 묶는다.
// 좌표는 항상 짝으로 다니므로, 따로 두면 헷갈리고 묶으면 "Cell 하나 = 한 좌표".
typedef struct {
  int r;
  int c;
} Cell;

// 뱀이 최대로 길어질 수 있는 칸 수 = 보드 전체 칸 수.
// #define은 단순 글자 치환이라 배열/malloc 크기에 그대로 쓸 수 있다.
// 괄호로 감싸는 게 안전 — 어디서 어떻게 치환되어도 우선순위 안 꼬임.
#define MAX_LEN (ROWS * COLS)

int main(void) {
  SetConsoleOutputCP(65001);

  // 보드는 여전히 스택에 잡힌다 (컴파일 시점에 크기 고정 + 작음).
  char board[ROWS][COLS];

  // 방향 벡터. 매 프레임 머리에 더해진다.
  int dr = 0, dc = 1;

  // 보드 한 번만 초기화 (차분 렌더링).
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      if (r == 0 || r == ROWS - 1 || c == 0 || c == COLS - 1) {
        board[r][c] = '#';
      } else {
        board[r][c] = '.';
      }
    }
  }

  // === 뱀 몸통: 힙에 동적 할당 ===
  // sizeof(Cell) * MAX_LEN 바이트를 힙에서 빌려와서 그 시작 주소를 body에.
  // body는 "Cell 배열의 시작을 가리키는 쪽지"가 된다.
  Cell *body = malloc(sizeof(Cell) * MAX_LEN);
  int length = 1;                  // 현재 뱀 길이 (머리만 있으니 1)

  // body[0]이 항상 머리. 보드 한가운데에서 시작.
  body[0].r = ROWS / 2;
  body[0].c = COLS / 2;

  // 직전 꼬리 좌표 — 다음 프레임에서 이 자리를 '.'으로 지운다.
  // Stage 3의 prev_r/prev_c가 Cell 한 개로 합쳐진 것.
  Cell prev_tail = body[0];

  // 종료 깃발. 'q'에서 내리고 루프 빠져나가 free까지 한 줄로 진행.
  int running = 1;

  while (running) {
    printf("\033[H");

    // === 차분 렌더링 ===
    board[prev_tail.r][prev_tail.c] = '.';      // 직전 꼬리 지우기
    board[body[0].r][body[0].c]     = 'O';      // 현재 머리 찍기

    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLS; c++) {
        putchar(board[r][c]);
      }
      putchar('\n');
    }

    printf("이동: wasd, 종료: q\n");

    // === 입력: 논블로킹 ===
    if (_kbhit()) {
      int key = _getch();
      switch (key) {
        case 'w': dr = -1, dc =  0; break;
        case 's': dr =  1, dc =  0; break;
        case 'a': dr =  0, dc = -1; break;
        case 'd': dr =  0, dc =  1; break;
        case 'q': running = 0; break;   // 깃발만 내림. free는 루프 밖에서.
      }
    }

    // === 몸통 한 칸 전진 (큐 패턴) ===
    // 1) 다음 프레임에 지울 꼬리 좌표 저장.
    prev_tail = body[length - 1];

    // 2) 뒤에서 앞으로 한 칸씩 당김.
    //    뒤→앞 방향이 핵심 — 앞→뒤로 돌면 아직 안 옮긴 값을 덮어써서 망가짐.
    //    body[i] = body[i-1]은 struct 통째 복사 (r, c 한 번에).
    for (int i = length - 1; i > 0; i--) {
      body[i] = body[i - 1];
    }

    // 3) 머리만 방향대로 이동.
    body[0].r += dr;
    body[0].c += dc;

    // 벽 clamp (머리만 — 몸통은 머리 따라오니까 알아서 안에 머무름).
    if (body[0].r < 1)        body[0].r = 1;
    if (body[0].r > ROWS - 2) body[0].r = ROWS - 2;
    if (body[0].c < 1)        body[0].c = 1;
    if (body[0].c > COLS - 2) body[0].c = COLS - 2;

    Sleep(200);   // 약 5 FPS. FPS ≈ 1000 / Sleep_ms.
  }

  // malloc 했으면 반드시 free. 안 하면 메모리 누수.
  // 프로세스가 끝나면 OS가 회수해 주긴 하지만, "잡았으면 푼다" 규율이 중요.
  free(body);

  return 0;
}
```

> **`prev_tail = body[0];`로 초기화하는 이유**: 첫 프레임에서 차분 렌더링이 "직전 꼬리"를 지우려고 하므로, 시작 시점엔 머리 자리와 같게 두면 머리를 한 번 지웠다가 바로 다시 찍는 게 되어 무해하다.

---

## 돌려보기

```sh
gcc 01-snake/snake.c -o 01-snake/snake
./01-snake/snake
```

기대 동작은 Stage 3과 **똑같다** — 길이 1짜리 `O`가 자동 전진. 화면에 보이는 건 변화 없지만, 이제 길이가 변할 수 있는 자료구조 위에서 돌아가는 중. 다음 단계에서 사과를 먹으면 그제야 길어진다.

종료 후 콘솔에서 메모리 누수 없이 깨끗이 빠져나오는지 확인 (`q`로 종료).

---

## 흔히 빠지는 함정

- **`Cell body;`라고 쓰고 `body[0]`을 접근.** `body`는 본체 하나라 인덱싱 불가. 배열로 다루려면 `Cell *body;` 또는 `Cell body[N];`.
- **`free(body)`를 `while(1)` 다음에 쓰고 `case 'q': return 0;`로 종료.** `free`가 절대 실행 안 됨 → 메모리 누수. `running` 깃발 또는 `case 'q'` 안에서 `free` 먼저.
- **shift 루프를 앞→뒤로 돈다.** 아직 안 옮긴 값을 덮어써서 뱀 몸통이 한 글자로 짜부라짐.
- **`sizeof(Cell)` 대신 `8`을 직접 씀.** 플랫폼/패딩에 따라 안 맞을 수 있음. 항상 `sizeof`.
- **`malloc`이 `NULL`을 반환했는데 그대로 사용.** 메모리 부족 시 세그폴트. 실무 코드는 항상 `if (body == NULL) return 1;` 같은 가드. 학습 코드에선 일단 생략.
- **`free` 두 번 호출.** Undefined behavior. `free(body); free(body);`는 절대 금지.
- **`free` 후 포인터 계속 사용.** 댕글링 포인터. 안전하게 가려면 `free(body); body = NULL;`.

---

## 체크 질문

소리 내어 답해보기:

1. `typedef struct { ... } Cell;`에서 `typedef`를 빼면 뭐가 달라지나? 그때 변수 선언은 어떻게 써야 하지?
2. `Cell body;` vs `Cell *body;` — 메모리에 어떻게 놓이는지 그림으로 그려보자.
3. `sizeof(Cell)`이 "보통 8바이트"인 이유는? 왜 꼭 8이 아닐 수도 있나? (힌트: 패딩, 플랫폼별 int 크기)
4. `body[0]`이 왜 `*(body + 0)`과 같은가? 그럼 `body[1]`은 주소상 어디?
5. `body[i] = body[i - 1]` 한 줄로 struct 전체가 복사되는 이유는?
6. shift 루프를 `i = 1; i < length; i++`로 **앞에서부터** 돌리면 무엇이 망가지나? 실제로 그렇게 짜보고 그려보자.
7. `malloc`한 걸 `free` 안 하면 즉시 문제가 되나? 언제·어떤 맥락에서 진짜로 문제가 되지?
8. `int *`나 `void *`로 `body`를 받으면 왜 안 되나? 왜 굳이 `Cell *`?
9. `free(body)`를 `while(1)` 뒤에 두고 `case 'q'`에서 `return 0;`을 하면 무슨 일이 일어나지?
10. 스택 vs 힙 — 셋 이상의 차이를 말해보자. (수명 / 크기 / 속도 / 관리 방식 등)

---

## 다음 단계 예고

[stage-5.md] _(예정)_ — 사과 `*`를 보드에 랜덤 배치하고, 머리가 사과 위에 오면 `length++`. 꼬리가 그 프레임만 지워지지 않게 분기. **난수**(`rand`, `srand`)와 **상태 머신**이 들어온다.
