#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define BOARD_SIZE 8
#define MINE_NUM 10

/* Cell Status */
#define CLOSED 0  // Cell is still closed
#define OPENED 1  // Cell was opened
#define MARKED 2  // Cell was marked as mine
#define SPACED 3  // Cell was opend by chain

/* Operate code */
#define OPEN 0        // One or more mines around cell
#define CHAIN 1       // No mines around opened cell
#define MARK 2        // Mark cell as mine
#define GAME_OVER 3   // Opend Cell was mine
#define GAME_CLEAR 4  // All cells was opened

typedef struct {
  int   status; // Cell status
  int   around_mine_num; // mine num around cell
  bool  is_mine;  // this cell is mine?
} cell_t;

void initialize(cell_t cell[][BOARD_SIZE]);
void display_navi(void);
void display_board(cell_t cell[][BOARD_SIZE]);
void input(int *, int *, char *, cell_t cell[][BOARD_SIZE]);
bool judge_input(int, int, char, cell_t cell[][BOARD_SIZE]);
void locate_mine(cell_t cell[][BOARD_SIZE]);
void count_mine(cell_t cell[][BOARD_SIZE]);
int  get_mine(int x, int y, cell_t cell[][BOARD_SIZE]);
int  judge_operation(int, int, char, cell_t cell[][BOARD_SIZE]);
bool is_clear(cell_t cell[][BOARD_SIZE]);
void update(int, int, char, cell_t cell[][BOARD_SIZE]);
void chain(int, int, int, int, cell_t cell[][BOARD_SIZE]);
void display_result(int operate_code); 

int main(void) {
  cell_t cell[BOARD_SIZE][BOARD_SIZE];
  int   operate_code;
  int   c_x, c_y; // current_x, y
  char  c_command;

  initialize(cell);

  do {
    display_board(cell);

    input(&c_x, &c_y, &c_command, cell);

    operate_code = judge_operation(c_x, c_y, c_command, cell);

    update(c_x, c_y, operate_code, cell);

  } while(operate_code != GAME_OVER && operate_code != GAME_CLEAR);

  return 0;
}

/* ゲーム初期化 */
void initialize(cell_t cell[][BOARD_SIZE]) {
  for(int i = 0; i < BOARD_SIZE; i++) {
    for(int j = 0; j < BOARD_SIZE; j++) {
      cell[i][j].status = CLOSED;
      cell[i][j].is_mine = false;
    }
  }

  locate_mine(cell);
  count_mine(cell);

  display_navi();
}

/* 最初のナビ表示 */
void display_navi(void) {
  printf("*** M Sweeper ***\n");
  printf("コマンドの入力:x y [asm]\n");
  printf("  x y ... 座標[0-7]\n");
  printf("  a   ... (x,y)の周囲の点を自動的にチェック\n");
  printf("  s   ... (x,y)を安全な点としてチェック\n");
  printf("  m   ... (x,y)にMマークをつける\n");
}

/* 盤面表示 */
void display_board(cell_t cell[][BOARD_SIZE]) {

  printf("\n ");

  for(int i = 0; i < BOARD_SIZE; i++) {
    printf(" %d", i); // display x coordinate
  }
  printf("\n");

  for(int y = 0; y < BOARD_SIZE; y++) {
    printf("%d", y);  // display y coordinate

    for(int x = 0; x < BOARD_SIZE; x++) {
      switch(cell[y][x].status) {
        case OPENED:  printf(" %d", cell[y][x].around_mine_num);
                      break;
        case CLOSED:  printf(" .");
                      break;
        case MARKED:  printf(" M");
                      break;
        case SPACED:  printf("  "); 
                      break;
        default:      printf(" ?"); //TODO: 異常終了させる
      }
    }

    printf("\n");
  }
}

/* 入力 */
void input(int *x, int *y, char *command, cell_t cell[][BOARD_SIZE]) {
  char meta;
  bool is_inputtable = false;

  while(is_inputtable == false) {
    printf(">");
    scanf("%d%c%d%c%c%c", x, &meta, y, &meta, command, &meta);

    // judge whether satisfying input criteria
    is_inputtable = judge_input(*x, *y, *command, cell);
  }

}

/* 入力条件を満たしているか判定する */
bool judge_input(int x, int y, char command, cell_t cell[][BOARD_SIZE]) {
  if(x < 0 || 7 < x || y < 0 || 7 < y) {
    printf("Out of range.\n");
    return false;
  }

  if(command != 'a' && command != 's' && command != 'm') {
    printf("This command isn't appropriate.\n");
    return false;
  }

  if(cell[y][x].status == OPENED) {
    printf("This cell is already opened.\n");
    return false;
  }

  if(cell[y][x].status == SPACED) {
    printf("This cell is already opend by chain.\n");
    return false;
  }

  return true;
}

/* 地雷の場所を決定する */
void locate_mine(cell_t cell[][BOARD_SIZE]) {
  int r_x, r_y; // random location of mine 

  srand((unsigned)time(NULL));

  for(int i = 0; i < MINE_NUM; i++) {
    // locate mine without duplication
    do {
      r_x = rand() % 8;
      r_y = rand() % 8;
    } while(cell[r_y][r_x].is_mine == true);

    cell[r_y][r_x].is_mine = true;
  }
}

/* 盤面各々のマスの周囲の地雷数を数える */
void count_mine(cell_t cell[][BOARD_SIZE]) {
  for(int y = 0; y < BOARD_SIZE; y++) {
    for(int x = 0; x < BOARD_SIZE; x++) {
      cell[y][x].around_mine_num = get_mine(x, y, cell);
    }
  }
}

/* 隣接しているマスの地雷数を数える */
int get_mine(int x, int y, cell_t cell[][BOARD_SIZE]) {
  int around_mine_num = 0;

  for(int i = -1; i < 2; i++) {

    if(y + i < 0 || 7 < y + i) {
      continue;
    }

    for(int j = -1; j < 2; j++) {

      if(x + j < 0 || 7 < x + j) {
        continue;
      }

      if(i == 0 && j == 0) {
        continue;
      }

      around_mine_num += cell[y + i][x + j].is_mine;
    }
  }

  return around_mine_num;
}

/* 入力された操作の種類の判断 */
int judge_operation(int x, int y, char command, cell_t cell[][BOARD_SIZE]) {

  if(command == 'm') {
    return MARK;
  }

  if(cell[y][x].is_mine == true) {
    return GAME_OVER;
  }

  if(is_clear(cell) == true) {
    return GAME_CLEAR;
  }

  if(cell[y][x].around_mine_num == 0) {
    return CHAIN;
  }

  if(command == 's') {
    return OPEN;
  }
}

/* ゲームクリア条件を満たしているか判定する */
bool is_clear(cell_t cell[][BOARD_SIZE]) {

  for(int y = 0; y < BOARD_SIZE; y++) {
    for(int x = 0; x < BOARD_SIZE; x++) {
      if(cell[y][x].status == CLOSED) {
        return false;
      }
    }
  }

  return true;
}

/* 情報を更新する */
void update(int x, int y, char operate_code, cell_t cell[][BOARD_SIZE])  {
  
  switch(operate_code) {
    case OPEN:        cell[y][x].status = OPENED; break;
    case CHAIN:       chain(x, y, x, y, cell);    break;
    case MARK:        cell[y][x].status = MARKED; break;
    case GAME_OVER:   display_result(GAME_OVER);  break;
    case GAME_CLEAR:  display_result(GAME_CLEAR); break;
    default:          exit(1);
  }
}

/* 入力マスの周囲に地雷がない時のチェイン */
void chain(int x, int y, int p_x, int p_y, cell_t cell[][BOARD_SIZE]) {
  int f_x, f_y; // forward_x, y

  cell[y][x].status = SPACED;

  for(int i = -1; i < 2; i++) {
    f_y = y + i;

    if(f_y < 0 || 7 < f_y)  continue;

    for(int j = -1; j < 2; j++) {
      f_x = x + j;

      if(f_x < 0 || 7 < f_x)  continue;
      if(i == 0 && j == 0)    continue;
      if(p_x == f_x && p_y == f_y)        continue;
      if(cell[f_y][f_x].status == SPACED) continue;
      if(cell[f_y][f_x].status == OPENED) continue;

      if(cell[f_y][f_x].around_mine_num == 0) {
        chain(f_x, f_y, x, y, cell);
      } else {
        cell[f_y][f_x].status = OPENED;
      }
    }
  } 
}

/* ゲーム結果を表示する */
void display_result(int operate_code) {
  switch(operate_code) {
    case GAME_OVER:   printf("ゲームオーバー\n");
                      break;
    case GAME_CLEAR:  printf("ゲームクリア　おめでとう！\n");
                      break;
  }
}
