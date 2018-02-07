#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define size 1024
#define gens 10

bool cells[size][size];
bool new_cells[size][size];

void randomizeCells() {
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      cells[x][y] = rand() % 2 == 0;
    }
  }
}

void printCells() {
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      printf(cells[x][y] ? "o" : " ");
    }
    printf("\n");
  }
}

void nextGeneration() {
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      int neighbors = 0;

      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          int nx = (size + x + dx) % size;
          int ny = (size + y + dy) % size;
          if ((dx != 0 || dy != 0) && cells[nx][ny]) {
            neighbors++;
          }
        }
      }

      new_cells[x][y] = neighbors == 3 || (neighbors == 2 && cells[x][y]);
    }
  }

  memcpy(cells, new_cells, sizeof cells);
}

int main(void) {
  randomizeCells();

  struct timeval start, stop;
  gettimeofday(&start, NULL);

  for (int i = 0; i < gens; ++i) {
    nextGeneration();
  }

  gettimeofday(&stop, NULL);

  float seconds = (stop.tv_usec - start.tv_usec) / 1e6 + stop.tv_sec - start.tv_sec;
  int ops = size * size * gens;

  printf("C Efficiency in cellhz: %e\n", 1.0 * ops / seconds);

  return 0;
}
