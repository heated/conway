#include <iostream>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define size 1000
#define test_length 10

bool coinflip() {
  return rand() % 2 == 0;
}

void randomizeCells(bool cells[size][size]) {
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      cells[i][j] = coinflip();
    }
  }
}

void printCells(bool cells[size][size]) {
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      printf(cells[i][j] ? "o" : " ");
    }
    printf("\n");
  }
}

void nextGeneration(bool cells[size][size]) {
  bool new_cells[size][size];
  memset(new_cells, false, sizeof(bool) * size * size);

  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      int neighbors = 0;

      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          int x = (size + i + dx) % size;
          int y = (size + j + dy) % size;
          if ((dx != 0 || dy != 0) && cells[x][y]) {
            neighbors++;
          }
        }
      }

      new_cells[i][j] = neighbors == 3 || neighbors == 2 && cells[i][j];
    }
  }

  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      cells[i][j] = new_cells[i][j];
    }
  }
}

int main(void) {
  bool cells[size][size];

  randomizeCells(cells);

  struct timeval start, stop;
  gettimeofday(&start, NULL);

  for (int i = 0; i < test_length; ++i) {
    nextGeneration(cells);
  }

  gettimeofday(&stop, NULL);

  float seconds = (stop.tv_usec - start.tv_usec) / 1e6 + stop.tv_sec - start.tv_sec;
  int ops = size * size * test_length;

  // printCells(cells);
  std::cout << "C++ Efficiency in cellhz: " << ops / seconds << std::endl;

  return 0;
}
