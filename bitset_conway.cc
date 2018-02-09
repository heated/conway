#include <bitset>
#include <ctime>
#include <iostream>

constexpr int rows = 1<<11;
constexpr int size = rows * rows;
constexpr int gens = 100;

std::bitset<size> alive;

void randomizeCells() {
  for (int i = 0; i < size; ++i) {
    alive[i] = rand()&1;
  }
}

void printCells() {
  std::cout << "\033[H\033[2J";
  for (int x = 0; x < rows && x < 16; ++x) {
    for (int y = 0; y < 16; ++y) {
      std::cout << (alive[x * rows + y] ? "o" : " ");
    }
    std::cout << std::endl;
  }
}

void nextGeneration() {
  std::bitset<size> n1;
  std::bitset<size> n2;
  std::bitset<size> n4;

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      if (dx != 0 || dy != 0) {
        int shift = (size + dx * rows + dy) % size;
        int unshift = size - shift;
        std::bitset<size> n = alive >> shift | alive << unshift;
        std::bitset<size> carry = n1 & n;
        n1 ^= n;
        n4 |= n2 & carry;
        n2 ^= carry;
      }
    }
  }

  alive = n2 & (n1 | alive) & ~n4;
}

int main(void) {
  randomizeCells();

  std::clock_t start, stop;
  start = std::clock();

  for (int i = 0; i < gens; ++i) {
    nextGeneration();
  }

  stop = std::clock();
  float efficiency = float(long(size) * gens) / (stop - start) * CLOCKS_PER_SEC;
  std::cout << "C++ Efficiency in cellhz: " << efficiency << std::endl;

  return 0;
}
