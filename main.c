#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>


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


/* 設定各種 */
typedef struct {
  
  int   board_size;
  int   mine_num;
  int   elapsed_time;
  int   c_x;        // current_x
  int   c_y;        // current_y
  char  c_command;  // current_command
  int   operate_code;

} settings_t;

/* マスの情報 */
typedef struct {

  int   status;           // Cell status
  int   around_mine_num;  // mine num around cell
  bool  is_mine;          // this cell is mine?

} cell_t;


cell_t** initialize(settings_t *settings, cell_t **cell);
void display_navi(void);
void display_board(settings_t settings, cell_t **cell);
void input(settings_t *settings, cell_t **cell);
bool judge_input(settings_t settings, cell_t **cell);

void locate_mine(settings_t settings, cell_t **cell);
void count_mine(settings_t settings, cell_t **cell);
int  get_mine(int x, int y, settings_t settings, cell_t **cell);
int  judge_operation(settings_t settings, cell_t **cell);
bool is_clear(int, int, settings_t settings, cell_t **cell);
void update(settings_t settings, cell_t **cell);
void chain(int, int, int, int, int, cell_t **cell);
void display_result(int operate_code); 


int main(void) {
  
  settings_t settings;
  cell_t **cell;

  cell = initialize(&settings, cell);

  printf("init ok\n");
  
  do {
    display_board(settings, cell);

    input(&settings, cell);

    settings.operate_code = judge_operation(settings, cell);

    update(settings, cell);
  } while(settings.operate_code != GAME_OVER && settings.operate_code != GAME_CLEAR);

  for(int i = 0; i < settings.board_size; i++) {
    free(cell[i]);
  }

  free(cell);

  return 0;
  
}

/* ゲーム初期化 */
cell_t** initialize(settings_t *settings, cell_t **cell) {
  char dummy;

  printf("ボードの大きさ\t>");
  scanf("%d%c", &(settings->board_size), &dummy);

  printf("地雷の個数\t>");
  scanf("%d", &(settings->mine_num));
  printf("OK..\n");
  
  // 構造体cellのメモリ確保
  cell = (cell_t **)malloc(sizeof(cell_t *) * settings->board_size);
  for(int i = 0; i < settings->board_size; i++) {
    cell[i] = (cell_t *)malloc(sizeof(cell_t) * settings->board_size);
  }

  for(int i = 0; i < settings->board_size; i++) {
    for(int j = 0; j < settings->board_size; j++) {
      cell[i][j].status = CLOSED;
      cell[i][j].is_mine = false;
    }
  }

  locate_mine(*settings, cell);
  count_mine(*settings, cell);

  display_navi();

  return cell;

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
void display_board(settings_t settings, cell_t **cell) {
  int board_size = settings.board_size;

  printf("\n ");

  for(int i = 0; i < board_size; i++) {
    printf(" %d", i); // display x coordinate
  }

  printf("\n");

  for(int y = 0; y < board_size; y++) {
    printf("%d", y);  // display y coordinate
    for(int x = 0; x < board_size; x++) {
      switch(cell[y][x].status) {
        case OPENED:  printf(" %d", cell[y][x].around_mine_num);
                      break;
        case CLOSED:  if(cell[y][x].is_mine == true) printf(" @"); else
                      printf(" .");
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
void input(settings_t *settings, cell_t **cell) {

  char meta;  // dummy input
  bool is_inputtable = false;

  while(is_inputtable == false) {
    printf(">");
    scanf("%d%c%d%c%c%c", &(settings->c_x), &meta, &(settings->c_y), &meta, &(settings->c_command), &meta);

    is_inputtable = judge_input(*settings, cell);
  }

}

/* 入力条件を満たしているか判定する */
bool judge_input(settings_t settings, cell_t **cell) {
  int x = settings.c_x;
  int y = settings.c_y;
  int command = settings.c_command;
  int board_size = settings.board_size;

  if(x < 0 || board_size - 1 < x || y < 0 || board_size - 1 < y) {
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
/********************************************************************************/
/* 地雷の場所を決定する */
void locate_mine(settings_t settings, cell_t **cell) {

  int r_x, r_y; // random location of mine 

  srand((unsigned)time(NULL));

  for(int i = 0; i < settings.mine_num; i++) {
    // locate mine without duplication
    do {
      r_x = rand() % settings.board_size;
      r_y = rand() % settings.board_size;
    } while(cell[r_y][r_x].is_mine == true);

    cell[r_y][r_x].is_mine = true;
  }

}

/* 盤面各々のマスの周囲の地雷数を数える */
void count_mine(settings_t settings, cell_t **cell) {

  for(int y = 0; y < settings.board_size; y++) {
    for(int x = 0; x < settings.board_size; x++) {
      cell[y][x].around_mine_num = get_mine(x, y, settings, cell);
    }
  }

}

/* 隣接しているマスの地雷数を数える */
int get_mine(int x, int y, settings_t settings, cell_t **cell) {
  int around_mine_num = 0;
  int board_size = settings.board_size;

  for(int i = -1; i < 2; i++) {
    if(y + i < 0 || board_size - 1 < y + i) {
      continue;
    }

    for(int j = -1; j < 2; j++) {
      if(x + j < 0 || board_size - 1 < x + j) {
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
int judge_operation(settings_t settings, cell_t **cell) {
  int x = settings.c_x;
  int y = settings.c_y;
  char command = settings.c_command;

  if(command == 'm') {
    return MARK;
  }

  if(cell[y][x].is_mine == true) {
    return GAME_OVER;
  }
  
  if(is_clear(x, y, settings, cell) == true) {
    return GAME_CLEAR;
  }

  if(cell[y][x].around_mine_num == 0) {
    return CHAIN;
  }

  if(command == 's') {
    return OPEN;
  }

  exit(1);

}

/* ゲームクリア条件を満たしているか判定する */
bool is_clear(int x, int y, settings_t settings, cell_t **cell) {

  for(int i = 0; i < settings.board_size; i++) {
    for(int j = 0; j < settings.board_size; j++) {
      if(i == y && j == x) {
        continue;
      }

      if(cell[i][j].is_mine == true) {
        continue;
      }

      if(cell[i][j].status == CLOSED) {
        return false;
      }
    }
  }

  return true;

}

/* 情報を更新する */
void update(settings_t settings, cell_t **cell)  {
  int x = settings.c_x;
  int y = settings.c_y;
  int board_size = settings.board_size;
  
  switch(settings.operate_code) {
    case OPEN:        cell[y][x].status = OPENED; break;
    case CHAIN:       chain(board_size, x, y, x, y, cell);    break;
    case MARK:        cell[y][x].status = MARKED; break;
    case GAME_OVER:   display_result(GAME_OVER);  break;
    case GAME_CLEAR:  display_result(GAME_CLEAR); break;
    default:          exit(1);
  }

}

/* 入力マスの周囲に地雷がない時のチェイン */
void chain(int board_size, int x, int y, int p_x, int p_y, cell_t **cell) {
  int f_x, f_y; // forward_x, y

  cell[y][x].status = SPACED;

  for(int i = -1; i < 2; i++) {
    f_y = y + i;

    if(f_y < 0 || board_size - 1 < f_y)  continue;

    for(int j = -1; j < 2; j++) {
      f_x = x + j;

      if(f_x < 0 || board_size - 1 < f_x)  continue;
      if(i == 0 && j == 0)    continue;
      if(p_x == f_x && p_y == f_y)        continue;
      if(cell[f_y][f_x].status == SPACED) continue;
      if(cell[f_y][f_x].status == OPENED) continue;

      if(cell[f_y][f_x].around_mine_num == 0) {
        chain(board_size, f_x, f_y, x, y, cell);
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
