/* init disp input decide_mine calc_mnum get_mine OK */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define B_SIZE 8

/* Cell Status */
#define CLOSED 0  // マスが未開拓
#define OPENED 1  // マスが開拓済み
#define MARKED 2  // マスが地雷としてマークされている
#define SPACED 3  // 

/* Status Code */
#define GAMEOVER 0
#define CHAIN 1
#define MARK 2
#define NORMAL 3


typedef struct {
  int   status;       // Cell Status
  int   mine_around;  // 周りの地雷の数
  bool  is_mine;      // 自分が地雷かどうか
} data_t;

void  init(data_t data[][B_SIZE]);
void  disp(data_t data[][B_SIZE]);
void  input(int *x, int *y, char *command, data_t  data[][B_SIZE]);
void  decide_mine(data_t data[][B_SIZE]);
void  calc_mnum(data_t data[][B_SIZE]);
int   get_mine(data_t data[][B_SIZE], int x, int y);
int   judge(data_t data[][B_SIZE], int x, int y, char command);
void  update(data_t data[][B_SIZE], int x, int y, char status_code);
void  chain(data_t data[][B_SIZE], int x, int y, int p_x, int p_y);


int main(void) {
  data_t data[B_SIZE][B_SIZE];  // data[y][x]
  int c_x;  // 現在入力されたx
  int c_y;  // 現在入力されたy
  char c_command; // 現在入力されたコマンド
  int status_code;  // Status Code
  
  do { 
    /* 初期化 */  
    init(data);

    /* 表示 */
    disp(data);

    /* 入力 */
    input(&c_x, &c_y, &c_command, data);

    /* 判定 */
    status_code = judge(data, c_x, c_y, c_command);

    printf("status_code = %d\n", status_code);

    /* データ更新 */
    update(data, c_x, c_y, status_code);
  } while(status_code != GAMEOVER);

    return 0;
}

void init(data_t data[][B_SIZE]) {
  /* 盤面のすべての値を初期化 */
  for(int i = 0; i < B_SIZE; i++) {
    for(int j = 0; j < B_SIZE; j++) {
      data[i][j].status = CLOSED;
      data[i][j].is_mine = false;
    }
  }

  /* 地雷の場所を決定 */
  decide_mine(data);

  /* 周囲の地雷の個数を調査 */
  calc_mnum(data);

  /* 最初の表示 */
  printf("*** M Sweeper ***\n");
  printf("コマンドの入力:x y [asm]\n");
  printf("  x y ... 座標[0-7]\n");
  printf("  a   ... (x,y)の周囲の点を自動的にチェック\n");
  printf("  s   ... (x,y)を安全な点としてチェック\n");
  printf("  m   ... (x,y)にMマークをつける\n");
}

void disp(data_t data[][B_SIZE]) {
  printf("\n ");

  /* x座標数字表示 */
  for(int i = 0; i < B_SIZE; i++) {
    printf(" %d", i);
  }
  printf("\n");

  for(int i = 0; i < B_SIZE; i++) {
    /* y座標数字表示 */
    printf("%d", i);

    /* データ表示 */
    for(int j = 0; j < B_SIZE; j++) {
      switch(data[i][j].status) {
        case OPENED:
          printf(" %d", data[i][j].mine_around);
          break;
        case CLOSED:
          if(data[i][j].is_mine == true) printf(" @");
          else printf(" .");
          break;
        case MARKED:
          printf(" M");
          break;
        default:
          printf(" ?");
      }
    }
    printf("\n");
  }
}

void input(int *x, int *y, char *command, data_t data[][B_SIZE]) {
  char    meta;     // 入力時の邪魔文字用
  int    is_satisfied = 0;
  
  while(is_satisfied != 3) {
    /* 入力 */
    scanf("%d%c%d%c%c%c", x, &meta, y, &meta, command, &meta);

    /* 盤面内を指定しているか */
    if(0 <= *x && *x <= 7 && 0 <= *y && *y <= 7) {
      is_satisfied ++;
    }
    /* コマンドはa s mで指定されているか */
    if(*command == 'a' || *command == 's' || *command == 'm') {
      is_satisfied ++;
    }

    /* 開拓済みのマスを指定していないか */
    if(data[*y][*x].status != OPENED) {
      is_satisfied ++;
    }
  }
}

void decide_mine(data_t data[][B_SIZE]) {
  int mine_num = 10;
  int rx;
  int ry;

  srand((unsigned)time(NULL));
  
  for(int i = 0; i < mine_num; i++) {
    do {
      rx = rand() % 8;
      ry = rand() % 8;
    } while(data[ry][rx].is_mine == true);

    data[ry][rx].is_mine = true; 
  }
}

void calc_mnum(data_t data[][B_SIZE]) {
  for(int i = 0; i < B_SIZE; i++) {
    for(int j = 0; j < B_SIZE; j++) {
      data[j][i].mine_around = get_mine(data, i, j);
    }
  }
}

int get_mine(data_t data[][B_SIZE], int x, int y) {
  int mnum = 0;

  for(int i = -1; i < 2; i++) {
    /* y軸の範囲内ならば調べる */
    if(0 <= y + i && y + i <= 7) {
      for(int j = -1; j < 2; j++) {
        /* x軸の範囲内ならば調べる */
        if(0 <= x + j && x + j <= 7) {
          if(i != 0 || j != 0) {
            mnum += data[y + i][x + j].is_mine;
          }
        }
      }
    }
  }

  return mnum;
}

int judge(data_t data[][B_SIZE], int x, int y, char command) {
  int result;

  /* 地雷を踏んだか、連鎖反応は起きるか */
  if(data[y][x].is_mine == true) {
    printf("ゲームオーバ\n");
    result = GAMEOVER;
  } else if(data[y][x].mine_around == 0) {
    result = CHAIN;
  } else if(command == 's') {
    result = MARK;
  } else {
    result = NORMAL;
  }

  return result;
}

void update(data_t data[][B_SIZE], int x, int y, char status_code) {
  switch(status_code) {
    case GAMEOVER:
      break;
    case CHAIN:
      chain(data, x, y, x, y);      
      break;
    case MARK:
      data[y][x].status = MARKED;
      break;
    case NORMAL:
      data[y][x].status = OPENED;
      break;
    default:
      printf("ステータスコードが渡されませんでした\n");
  }  
}

void chain(data_t data[][B_SIZE], int x, int y, int p_x, int p_y) {
  printf("x = %d, y = %d, p_x = %d, p_y = %d\n", x, y, p_x, p_y);

  for(int i = -1; i < 2; i++) {
    for(int j = -1; j < 2; j++) {
      /* 真ん中のマスは調べない */
      if(i != 0 || j != 0) {
        /* 周りのマスが0~7の範囲内で調査可能な時 */
        if(0 <= y + i && y + i <= 7 && 0 <= x + j && x + j <= 7) {
          if(p_x != x + j || p_y != y + i) {
            if(data[y + i][x + j].mine_around == 0) {
              chain(data, x + i, y + i, x, y);
              data[y][x].status = SPACED;
            } else {
              data[y + i][x + i].status = OPENED;
            }
          }
        }
      }
    }
  }
}
