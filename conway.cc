#include <ctime>
#include <iostream>

constexpr int rows = 1 << 10;
constexpr int cols = rows / 16;
constexpr int gens = 10;
constexpr uint64_t all_on = 0x1111111111111111;

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
    std::cout << std::endl;
  }
}

uint64_t next(uint64_t alive, uint64_t neighbors) {
  uint64_t b4 = (neighbors & (all_on << 2)) >> 2;
  uint64_t b2 = (neighbors & (all_on << 1)) >> 1;
  uint64_t b1 = neighbors & all_on;
  return b2 & (b1 | alive) & ~b4;
}

void nextGeneration() {
  for (int x = 0; x < rows; ++x) {
    for (int y = 0; y < cols; ++y) {
      neighbors[x][y] = 0;
      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          if (dx != 0 || dy != 0) {
            int nx = (rows + x + dx) % rows;
            int ny = (cols + y + dy) % cols;
            uint64_t alive = cells[nx][y];
            uint64_t last = cells[nx][ny];

            switch (dy) {
            case 1:
              alive <<= 4;
              last >>= 60;
              alive |= last;
              break;
            case -1:
              alive >>= 4;
              last <<= 60;
              alive |= last;
              break;
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

  std::clock_t start, stop;
  start = std::clock();

  for (int i = 0; i < gens; ++i) {
    nextGeneration();
  }

  stop = std::clock();
  float efficiency = float(long(rows) * rows * gens) / (stop - start) * CLOCKS_PER_SEC;
  std::cout << "C++ Efficiency in cellhz: " << efficiency << std::endl;

  return 0;
}
