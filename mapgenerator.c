#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "bmp_writer.h"

typedef struct potfield {
  int y,x;
  int** map;
} potfield_t;

enum FIELDS {
  OBSTACLE = -2,
  EMPTY = -1,
  GOAL = 0
};

int** generate_map(int y, int x, enum FIELDS fill) {
  /*
  int** map;
  if (!(map = malloc(y*sizeof(*map)))) {
    fprintf(stderr, "malloc failed\n");
    exit(1);
  }  
  for (int i=0; i<y; ++i) {
    int* row;
    if(!(row = calloc(x, sizeof(*row)))) {
      fprintf(stderr, "malloc failed\n");
      exit(1);
    }
    map[i] = row;
  
  }*/

  if (!y||!x) {
    fprintf(stderr,"error: dimension cannot be 0\n");
    exit(1);
  }
  int** map;
  if (!(map = malloc(y*sizeof(*map)))) {
    fprintf(stderr, "error: malloc failed\n");
    exit(1);
  } 
  int* tmp;
  if (!(tmp = malloc(x*y*sizeof(*tmp)))) {
    fprintf(stderr, "error: malloc failed\n");
    free(map);
    exit(1);
  }
  for (int i=0; i<x*y; ++i)
    tmp[i] = fill;
  for (int i=0; i<y; ++i)
    map[i] = tmp+x*i;
  return map;
}

void remove_map(int** map, int y) {
  /*
  for (int i=0; i<y; ++i) {
    free(map[i]);
  }
  free(map);
  */
  free(*map);
  free(map);
}

void draw_map(int** map, int y, int x) {
  
  #define BLUE(S) "\x1b[34m"S"\x1b[0m"
  #define RED(S) "\x1b[31m"S"\x1b[0m"
  #define MAGENTA(S) "\x1b[35m"S"\x1b[0m"
  int all = x*y, goals = 0, obstacles = 0;
  for (int i=0; i<y; ++i) {
    for (int j=0; j<x; ++j) {
      int field = map[i][j];
      switch(field) {
        case GOAL:
          printf("  "MAGENTA("$")" ");
          ++goals;
          break;
        case OBSTACLE:
          printf("  "BLUE("#")" ");
          ++obstacles;
          break;
        case EMPTY:
          printf("  . ");
          break;
        default:
          printf("%3d ", map[i][j]);
          break;
      }
    }
    printf("|  %d\n", i);
  }
  for (int j=0; j<x; ++j)
    printf("----");
  puts("-");
  for (int j=0; j<x; ++j) 
    printf("%3d ", j);
  printf("\n{dim: %dx%d, "BLUE("obstacles: %d")", "MAGENTA("goals: %d")", fields: %d}\n", y, x, obstacles, goals, all); 
  #undef BLUE
  #undef RED
  #undef MAGENTA
}

void goals_prob(int** map, int y, int x, int percentage) {
  if (percentage < 0 ||percentage > 100) {
    fprintf(stderr,"percentage value not in range [0;100]\n");
    return;
  }
  srand(time(NULL));
  for (int i=0; i<y; ++i) {
    for(int j=0; j<x; ++j) {
      if (map[i][j] == EMPTY) {
        int r = rand() % 100 + 1;
        if (r <= percentage) {
          map[i][j] = GOAL;
        }
      }
    }
  }
}

void place_exact(int** map, int y, int x, int n, enum FIELDS type) {
  if (n < 0 || n > x*y) {
    fprintf(stderr,"n value not in range [0;ROWS*COLS]\n");
    return;
  }
  srand(time(NULL));
  int count = 0;
  while (count != n) {
    int xi = rand() % x;
    int yi = rand() % y;
    if (map[yi][xi] == EMPTY) {
      map[yi][xi] = type;
      ++count;
    }
  }
}

void potentialfeld(int** area, int y, int x) {
  #define NEXTPOT(Y,X) (((Y)>=0 && (Y)<y && (X)>=0 && (X)<x && area[(Y)][(X)] == EMPTY) ? (area[(Y)][(X)] = pot+1, 1) : (0))
  bool changed;
  int pot = GOAL;
  do {
    //printf("iteration: %d\n", pot);
    changed = false;
    for (int i=0; i<y; ++i) {
      for(int j=0; j<x; ++j) {
        if (area[i][j] == pot) {
          changed |= NEXTPOT(i,j-1) | NEXTPOT(i,j+1) |  NEXTPOT(i-1,j) | NEXTPOT(i+1,j);
        }
      }
    }
    ++pot;
  } while(changed);

  #undef NEXTPOT 
}

void shuffle(int* array, int n) {
  #define SWAP(A,a,b) do {int temp = A[(a)]; A[(a)] = A[(b)]; A[(b)] = temp;} while(0)
  for(int i=0; i<50; ++i) {
    int a = rand() % n;
    int b = rand() % n;
    SWAP(array, a, b);
  }
  #undef SWAP
}

void rec_gen_maze_rdfs(int** map, int y, int x, int i, int j) {
  #define CHECKCELL(Y, X) ((Y)>=0 && (Y)<y && (X)>=0 && (X)<x && map[(Y)][(X)] == OBSTACLE)

  map[i][j] = EMPTY;

  int directions[] = {0,1,2,3};
  //printf("new cell: (%d;%d)\n", i, j);
  shuffle(directions, 4);
  for (int k=0; k<4; ++k) {
    switch (directions[k]) {
      case 0:
        if (CHECKCELL(i-2,j)) {
          map[i-1][j] = EMPTY;
          rec_gen_maze_rdfs(map, y, x, i-2, j);
        }
        break;
      case 1:
        if (CHECKCELL(i+2,j)) {
          map[i+1][j] = EMPTY;
          rec_gen_maze_rdfs(map, y, x, i+2, j); 
        }
        break;
      case 2:
        if (CHECKCELL(i,j-2)) {
          map[i][j-1] = EMPTY;
          rec_gen_maze_rdfs(map, y, x, i, j-2); 
        }
        break;
      case 3:
        if (CHECKCELL(i,j+2)) {
          map[i][j+1] = EMPTY;
          rec_gen_maze_rdfs(map, y, x, i, j+2); 
        }
        break;
    }
  }
  #undef CHECKCELL
}

int** gen_maze_rdfs(int y, int x) {
  int** map = generate_map(y, x, OBSTACLE);
  srand(time(NULL));
  int i = rand() % y;
  int j = rand() % x;
  rec_gen_maze_rdfs(map, y, x, i, j);
  return map;
}

void to_pixel(int val, struct pixel* p, void* max) {
  int max_val = *(int*)max;
  switch (val) {
    case -2:
      p->r = 0;
      p->g = 0;
      p->b = 0;
      break;
    case -1:
      p->r = 100;
      p->g = 100;
      p->b = 100;
      break;
    case 0:
      p->r = 0;
      p->g = 255;
      p->b = 0;
      break;
    default:
      p->r = 255*(max_val-val)/max_val;
      p->g = 0;
      p->b = 255*val/max_val;
      break;
  }
}

int main(int argc, char** argv) {
  if (argc != 6) {
    fprintf(stderr, "usage: %s #ROWS #COLS #OBSTACLES #GOALS SCALE\n", *argv);
    return 1;
  }

  int y = atoi(argv[1]);
  int x = atoi(argv[2]);
  int n_obstacles = atoi(argv[3]);
  int n_goals = atoi(argv[4]);
  int scale = atoi(argv[5]);
  
  int** board = generate_map(y, x, EMPTY);
  
  place_exact(board, y, x, n_obstacles, OBSTACLE);
  
  //int** board = gen_maze_rdfs(y, x);
  //draw_map(board, y, x);

  place_exact(board, y, x, n_goals, GOAL);
  potentialfeld(board, y, x);
  //draw_map(board, y, x);

  int max_val = 0;
  
  for (int i=0; i<y; ++i) {
    for (int j=0; j<x; ++j) {
      int v = board[i][j];
      max_val = v > max_val ? v : max_val;
    }
  }
  printf("max_val: %d\n", max_val);
  bmp_from_array2d(board, y, x, scale, to_pixel, &max_val);

  remove_map(board, y);
  return 0;
}

