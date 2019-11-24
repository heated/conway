#include <ctime>
#include <iostream>
#include "immintrin.h" // for AVX

constexpr int rows = 1 << 10;
constexpr int cols = rows / 64;
constexpr int gens = 100;
__m256i all_on = _mm256_set1_epi8(0x11);

alignas(32) __m256i cells[rows][cols];
alignas(32) __m256i neighbors[rows][cols];

void randomizeCells() {
  for (int x = 0; x < rows; ++x) {
    for (int y = 0; y < cols; ++y) {
      uint8_t buffer[32];
      for (int i = 0; i < 32; ++i) {
        buffer[i] = rand() & 0x11;
      }
      cells[x][y] = _mm256_loadu_si256((__m256i *)&buffer);
      cells[x][y] = _mm256_setzero_si256();
    }
  }
  cells[0][0] = _mm256_set_epi64x(0x0100000000000000,0,0,0);
  cells[1][0] = _mm256_set_epi64x(0x0010000000000000,0,0,0);
  cells[2][0] = _mm256_set_epi64x(0x1110000000000000,0,0,0);
}

void printCells() {
  int height = 19;
  for (int x = 0; x < rows && x < height; ++x) {
    uint64_t buffer[4];
    _mm256_store_si256((__m256i *)&buffer, neighbors[x][0]);

    for (int i = 0; i < 4; ++i) {
      for (int y = 0; y < 16; ++y) {
        int n = (buffer[i] >> uint(60-y*4))&0xf;
        if (n == 0) {
          std::cout << " ";
        } else {
          std::cout << n;
        }
      }
    }
    std::cout << std::endl;
  }
  std::cout << "----------------" << std::endl;

  for (int x = 0; x < rows && x < height; ++x) {
    uint64_t buffer[4];
    _mm256_store_si256((__m256i *)&buffer, cells[x][0]);

    for (int i = 0; i < 4; ++i) {
      for (int y = 0; y < 16; ++y) {
        bool on = ((buffer[i] >> uint(60-y*4))&1) == 1;
        std::cout << (on ? "o" : " ");
      }
    }
    std::cout << std::endl;
  }
  std::cout << "----------------" << std::endl;
}

__m256i next(__m256i alive, __m256i neighbors) {
  __m256i b4 = _mm256_srli_epi16(
          _mm256_and_si256(neighbors, _mm256_slli_epi16(all_on, 2)),
          2);
  __m256i b2 = _mm256_srli_epi16(
          _mm256_and_si256(neighbors, _mm256_slli_epi16(all_on, 1)),
          1);
  __m256i b1 = _mm256_and_si256(neighbors, all_on);
  return _mm256_andnot_si256(b4, _mm256_and_si256(b2, _mm256_or_si256(b1, alive)));
}

void updateCells() {
  for (int x = 0; x < rows; ++x) {
    for (int y = 0; y < cols; ++y) {
      cells[x][y] = next(cells[x][y], neighbors[x][y]);
    }
  }
}

void nextGeneration() {
  for (int x = 0; x < rows; ++x) {
    for (int y = 0; y < cols; ++y) {
      neighbors[x][y] = _mm256_setzero_si256();
      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          if (dx != 0 || dy != 0) {
            int nx = (rows + x + dx) % rows;
            int ny = (cols + y + dy) % cols;
            __m256i alive = cells[nx][y];
            __m256i last = cells[nx][ny];
            __m256i zeroed = _mm256_and_si256(alive, _mm256_set_epi32(
                        0x00000000, 0xffffffff,
                        0xffffffff, 0xffffffff,
                        0xffffffff, 0xffffffff,
                        0xffffffff, 0xffffffff));
            __m256i bulk, indices, remaining;

            switch (dy) {
            case 1:
              bulk = _mm256_slli_epi64(alive, 4);
              last = _mm256_set_epi32(0,0,0,0,0,0,0,_mm256_extract_epi32(last, 0));
              indices = _mm256_set_epi32(0,2,0,4,0,6,0,0);
              remaining = _mm256_permutevar8x32_epi32(zeroed, indices);
              remaining = _mm256_srli_epi32(_mm256_or_si256(remaining, last), 28);
              remaining = _mm256_and_si256(remaining, _mm256_set1_epi64x(1));
              alive = _mm256_or_si256(bulk, remaining);
              break;
            case -1:
              bulk = _mm256_srli_epi64(alive, 4);
              last = _mm256_set_epi32(_mm256_extract_epi32(last, 7),0,0,0,0,0,0,0);
              indices = _mm256_set_epi32(0,0,1,0,3,0,5,0);
              remaining = _mm256_permutevar8x32_epi32(zeroed, indices);
              remaining = _mm256_slli_epi32(_mm256_or_si256(remaining, last), 28);
              remaining = _mm256_and_si256(remaining, _mm256_set1_epi64x(0x1000000000000000));
              alive = _mm256_or_si256(bulk, remaining);
              break;
            }

            neighbors[x][y] = _mm256_add_epi8(neighbors[x][y], alive);
          }
        }
      }
    }
  }

  updateCells();
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
