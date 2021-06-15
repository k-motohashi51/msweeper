#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define B_SIZE 8    // 盤面のサイズ
#define MINE_NUM 10 // 地雷の個数

/* Cell Status */
#define CLOSED 0  // マスが未開拓
#define OPENED 1  // マスが開拓済み
#define MARKED 2  // マスが地雷としてマークされている
#define SPACED 3  // 地雷が周囲にないと判明している

/* Status Code */
#define G_OVER 0  // ゲームオーバーだった
#define CHAIN 1   // 開拓の連鎖が起きる
#define MARK 2    // 地雷としてマークした
#define OPEN 3    // 開拓された
#define G_CLEAR 4 // ゲームクリア
#define UNDEF -1  // 未定義

// マスの状態や周囲の状況を表す構造体
typedef struct {
  int   status;       // Cell Status
  int   mine_around;  // 周りの地雷の数
  bool  is_mine;      // 自分が地雷かどうか
} data_t;

void  init(data_t data[][B_SIZE]);
void  disp(data_t data[][B_SIZE]);
void  input(int *x, int *y, char *command, data_t data[][B_SIZE]);
void  decide_mine(data_t data[][B_SIZE]);
void  calc_mnum(data_t data[][B_SIZE]);
int   get_mine(data_t data[][B_SIZE], int x, int y);
int   judge(data_t data[][B_SIZE], int x, int y, char command);
void  update(data_t data[][B_SIZE], int x, int y, char status_code);
void  chain(data_t data[][B_SIZE], int x, int y, int p_x, int p_y);


int main(void) {
  data_t data[B_SIZE][B_SIZE];  // data[y][x]
  int c_x;          // 現在入力されたx
  int c_y;          // 現在入力されたy
  char c_command;   // 現在入力されたコマンド
  int status_code;  // Status Code

  init(data); // 初期化
  
  // ゲームオーバーになるまで繰り返す
  do { 
    disp(data); // 表示
    input(&c_x, &c_y, &c_command, data);  // 入力
    status_code = judge(data, c_x, c_y, c_command); // 判定
    if(status_code == UNDEF) {
      printf("未定義：judge()\n");
      exit(1);
    }
    /* テスト表示 */
    printf("status_code:G_OVER=0 CHAIN=1 MARK=2 OPEN=3\n");
    printf("status_code = %d\n", status_code);

    update(data, c_x, c_y, status_code);  // データ更新
  } while(status_code != G_OVER && status_code != G_CLEAR);

    return 0;
}

void init(data_t data[][B_SIZE]) {
  for(int i = 0; i < B_SIZE; i++) {
    for(int j = 0; j < B_SIZE; j++) {
      data[i][j].status = CLOSED;
      data[i][j].is_mine = false;
    }
  }

  decide_mine(data);  // 地雷の場所を決定
  calc_mnum(data);    // 各々のマスの周囲の地雷数調査

  // 最初のナビ
  printf("*** M Sweeper ***\n");
  printf("コマンドの入力:x y [asm]\n");
  printf("  x y ... 座標[0-7]\n");
  printf("  a   ... (x,y)の周囲の点を自動的にチェック\n");
  printf("  s   ... (x,y)を安全な点としてチェック\n");
  printf("  m   ... (x,y)にMマークをつける\n");
}

void disp(data_t data[][B_SIZE]) {
  printf("\n ");

  for(int i = 0; i < B_SIZE; i++) {
    printf(" %d", i); // x座標数字表示
  }
  printf("\n");

  for(int i = 0; i < B_SIZE; i++) {
    printf("%d", i);  // y座標数字表示

    /* データ表示 */
    for(int j = 0; j < B_SIZE; j++) {
      switch(data[i][j].status) {
        case OPENED:  printf(" %d", data[i][j].mine_around);
                      break;
        case CLOSED:  if(data[i][j].is_mine == true) printf(" @");
                      else printf(" .");
                      break;
        case MARKED:  printf(" M");
                      break;
        case SPACED:  printf("  ");
                      break;
        default:      printf(" ?");
      }
    }
    printf("\n");
  }
}

void input(int *x, int *y, char *command, data_t data[][B_SIZE]) {
  char  meta;             // 入力時の邪魔文字用
  int   is_satisfied = 0; // 入力条件を満たしているか
  
  while(is_satisfied != 1) {
    // 入力
    printf(">");
    scanf("%d%c%d%c%c%c", x, &meta, y, &meta, command, &meta);
    
    // 入力条件を満たしているか判定
    if(*x < 0 || 7 < *x || *y < 0 || 7 < *y) {
      printf("範囲外です\n");
    } else if(*command != 'a' && *command != 's' && *command != 'm') {
      printf("コマンドが不適切です\n");
    } else if(data[*y][*x].status == OPENED) {
      printf("開拓済みです\n");
    } else if(data[*y][*x].status == SPACED) {
      printf("開拓済みです\n");
    } else {
      printf("OK\n");
      is_satisfied = 1;
    }
  }
}

void decide_mine(data_t data[][B_SIZE]) {
  int r_x, r_y; // 乱数により生成した地雷の場所

  srand((unsigned)time(NULL));
  
  for(int i = 0; i < MINE_NUM; i++) {
    // 地雷の場所が重複しないようにする
    do {
      r_x = rand() % 8;
      r_y = rand() % 8;
    } while(data[r_y][r_x].is_mine == true);

    data[r_y][r_x].is_mine = true; 
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
  int status_code;  // ステータスコード
  int is_clear = 0; // ゲームクリアかどうか

  /* 地雷を踏んだか、連鎖反応は起きるか */
  if(data[y][x].is_mine == true) {
    printf("ゲームオーバ\n");
    status_code = G_OVER;
  } else if(command == 'm') {
    status_code = MARK;
  } else if(data[y][x].mine_around == 0) {
    status_code = CHAIN;
  } else if(command == 's'){
    status_code = OPEN;
  } else {
    // ゲームクリアか判定
    for(int i = 0; i < B_SIZE; i++) {
      for(int j = 0; j < B_SIZE; j++) {
        if(data[i][j].status == CLOSED) {
          return UNDEF;
        }
      }
    }
    status_code = G_CLEAR;
    printf("ゲームクリア　おめでとう！\n");
  }

  return status_code;
}

void update(data_t data[][B_SIZE], int x, int y, char status_code) {
  switch(status_code) {
    case G_OVER:
      break;
    case CHAIN:
      chain(data, x, y, x, y);      
      break;
    case MARK:
      data[y][x].status = MARKED;
      break;
    case OPEN:
      data[y][x].status = OPENED;
      break;
    default:
      printf("ステータスコードが渡されませんでした\n");
  }  
}

void chain(data_t data[][B_SIZE], int x, int y, int p_x, int p_y) {
  int f_x, f_y; // 次に調べるマス
  
  data[y][x].status = SPACED; // 真ん中は必ずSPACED

  for(int i = -1; i < 2; i++) {
    for(int j = -1; j < 2; j++) {
      f_x = x + j;
      f_y = y + i;

      if(f_x < 0 || 7 < f_x || f_y < 0 || 7 < f_y) {
        continue;
      } else if(i == 0 && j == 0) {
        continue;
      } else if(p_x == f_x && p_y == f_y) {
        continue;
      } else if(data[f_y][f_x].status == SPACED) {
        continue;
      } else if(data[f_y][f_x].status == OPENED) {
        continue;
      } else {
        // 調べる条件は満たしている
        if(data[f_y][f_x].mine_around == 0) {
          chain(data, f_x, f_y, x, y);  // 次に調べる場所がSPECED 
        } else {
          data[f_y][f_x].status = OPENED;
        }
      }
    }
  }  
}
