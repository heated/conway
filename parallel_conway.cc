#include <algorithm>
#include <ctime>
#include "ctpl_stl.h"
#include <iostream>

constexpr int rows = 1 << 14;
constexpr int cols = rows / 64;
constexpr int gens = 5;
constexpr int threads = 2;
constexpr int batch = rows / threads;

uint64_t board1[rows][cols];
uint64_t board2[rows][cols];
auto cells = board1;
auto buffer = board2;

void randomizeCells() {
  for (int x = 0; x < rows; ++x) {
    for (int y = 0; y < cols; ++y) {
      for (int i = 0; i < 8; ++i) {
        cells[x][y] = (cells[x][y] << 8) | (rand() & 0xff);
      }
    }
  }
}

void printCells() {
  std::cout << "\033[H\033[2J";
  for (int x = 0; x < rows && x < 16; ++x) {
    for (int y = 0; y < 16; ++y) {
      bool on = ((cells[x][0] >> uint(63-y))&1) == 1;
      std::cout << (on ? "o" : " ");
    }
    std::cout << std::endl;
  }
}

void nextGeneration() {
  ctpl::thread_pool p(threads);
  std::vector<std::future<void>> results(threads);

  for (int i = 0; i < threads; i++) {
    int row_start = i * batch;
    results[i] = p.push([row_start](int){ 
      for (int y = row_start; y < row_start + batch; ++y) {
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

          buffer[y][x] = b2 & (b1 | cells[y][x]) & !b4;
        }
      }
    });
  }


  for (int i = 0; i < threads; ++i) {
    results[i].get();
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
  float efficiency = float(long(rows) * rows * gens) / (stop - start) * CLOCKS_PER_SEC;
  std::cout << "C++ Efficiency in cellhz: " << efficiency << std::endl;

  return 0;
}
