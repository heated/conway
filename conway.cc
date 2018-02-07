#include <iostream>
#include <string.h>
#include <sys/time.h>

#define rows 1024
#define cols rows / 16
#define gens 10
#define all_on 0x1111111111111111

uint64_t cells[rows][cols];
uint64_t neighbors[rows][cols];

void randomizeCells() {
  for (int x = 0; x < rows; ++x) {
    for (int y = 0; y < cols; ++y) {
      for (int i = 0; i < 8; ++i) {
        cells[x][y] = (cells[x][y] << 8) | (rand() & 0x11);
      }
    }
  }
}

void printCells() {
  std::cout << "\033[H\033[2J";
  for (int x = 0; x < rows && x < 16; ++x) {
    for (int y = 0; y < 16; ++y) {
      bool on = ((cells[x][0] >> uint(60-y*4))&1) == 1;
      std::cout << (on ? "o" : " ");
    }
    std::cout << "\n";
  }
}

uint64_t next(uint64_t alive, uint64_t neighbors) {
  uint64_t b4 = (neighbors & (all_on << 2)) >> 2;
  uint64_t b2 = (neighbors & (all_on << 1)) >> 1;
  uint64_t b1 = neighbors & all_on;
  return ((b1 & b2) | (b2 & alive & ~b1)) & ~b4;
}

void nextGeneration() {
  memset(&neighbors, 0, sizeof neighbors);

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      if (dx != 0 || dy != 0) {
        for (int x = 0; x < rows; ++x) {
          for (int y = 0; y < cols; ++y) {
            int nx = (rows + x + dx) % rows;
            int ny = (cols + y + dy) % cols;

            uint64_t alive = cells[nx][y];
            uint64_t last = cells[nx][ny];
            if (dy != 0) {
              alive <<= dy * 4;
              last >>= dy * 60;
              alive |= last;
            }
            neighbors[x][y] += alive;
          }
        }
      }
    }
  }

  for (int x = 0; x < rows; ++x) {
    for (int y = 0; y < cols; ++y) {
      cells[x][y] = next(cells[x][y], neighbors[x][y]);
    }
  }
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
  int ops = rows * rows * gens;

  std::cout << "C++ Efficiency in cellhz: " << ops / seconds << std::endl;

  return 0;
}
