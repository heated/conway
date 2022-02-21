#include <algorithm>
#include <ctime>
#include <iostream>
#include <iomanip>

constexpr uint32_t rows = 1 << 8;
constexpr uint32_t cols = rows / 64;
constexpr uint64_t target_load = 1e9;
constexpr uint64_t total_cells = rows * rows;
constexpr uint64_t gens = target_load / total_cells;
constexpr uint64_t total_cell_updates = gens * total_cells;

uint64_t board1[rows][cols];
uint64_t board2[rows][cols];
auto cells = board1;
auto buffer = board2;

void randomizeCells() {
  for (int y = 0; y < rows; ++y) {
    for (int x = 0; x < cols; ++x) {
      for (int i = 0; i < 8; ++i) {
        cells[y][x] = (cells[y][x] << 8) | (rand() & 0xff);
      }
    }
  }
}

void printCells() {
  //std::cout << "\033[H\033[2J";
  for (int y = 0; y < rows && y < 16; ++y) {
    for (int x = 0; x < 16; ++x) {
      bool on = ((cells[y][0] >> uint(63-x))&1) == 1;
      std::cout << (on ? "o" : " ");
    }
    std::cout << std::endl;
  }
}

void nextGeneration() {
  for (int y = 0; y < rows; ++y) {
    for (int x = 0; x < cols; ++x) {
      uint64_t b1 = 0, b2 = 0, b4 = 0;

      for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
          if (dx != 0 || dy != 0) {
            int ny = (rows + y + dy) % rows;
            int nx = (cols + x + dx) % cols;
            uint64_t alive = cells[ny][x];
            uint64_t last = cells[ny][nx];

            switch (dx) {
            case 1:
              alive <<= 1;
              last >>= 63;
              alive |= last;
              break;
            case -1:
              alive >>= 1;
              last <<= 63;
              alive |= last;
              break;
            }

            uint64_t c2 = alive & b1;
            uint64_t c4 = c2 & b2;
            b1 ^= alive;
            b2 ^= c2;
            b4 |= c4;
          }
        }
      }

      buffer[y][x] = b2 & (b1 | cells[y][x]) & ~b4;
    }
  }

  std::swap(cells, buffer);
}

int main(void) {
  randomizeCells();

  std::clock_t start, stop;
  start = std::clock();

  for (int i = 0; i < gens; ++i) {
    nextGeneration();
  }

  stop = std::clock();
  float duration_sec = 1.0 * (stop - start) / CLOCKS_PER_SEC;
  float cellghz = total_cell_updates / duration_sec / 1e9;
  std::cout << std::fixed;
  std::cout << std::setprecision(1);
  std::cout << "C++ implementation cellghz: " << cellghz << std::endl;
  // printCells();

  return 0;
}
