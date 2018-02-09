#include <ctime>
#include <iostream>

int rows = 1 << 13;
int cols = rows / 8;
int size = rows * cols;
int gens = 1000;
int space = size * sizeof(unsigned int);

void randomizeCells(unsigned int *cells) {
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < 4; ++j) {
      cells[i] <<= 8;
      cells[i] |= rand() & 0x11;
    }
  }
}

void printCells(unsigned int *cells) {
  std::cout << "\033[H\033[2J";
  for (int x = 0; x < rows && x < 16; ++x) {
    for (int y = 0; y < 16; ++y) {
      int idx = x * rows + y/8;
      bool on = ((cells[idx] >> (28-(y%8)*4))&1) == 1;
      std::cout << (on ? "o" : " ");
    }
    std::cout << std::endl;
  }
}

__global__
void neighborKernel(int size, int rows, unsigned int *cells, unsigned int *neighbors) {
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  for (int i = index; i < size; i += stride) {
    neighbors[i] = 0;
    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        if (dx != 0 || dy != 0) {
          int ni = (size + i + dx * rows) % size;
          int ny = (size + ni + dy) % size;
          unsigned int alive = cells[ni];
          unsigned int last = cells[ny];

          switch (dy) {
          case 1:
            alive <<= 4;
            last >>= 28;
            alive |= last;
            break;
          case -1:
            alive >>= 4;
            last <<= 28;
            alive |= last;
            break;
          }

          neighbors[i] += alive;
        }
      }
    }
  }
}

__global__
void lifeKernel(int size, unsigned int *cells, unsigned int *neighbors) {
  unsigned int all_on = 0x11111111;
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  for (int i = index; i < size; i += stride) {
    unsigned int b4 = (neighbors[i] & (all_on << 2)) >> 2;
    unsigned int b2 = (neighbors[i] & (all_on << 1)) >> 1;
    unsigned int b1 = neighbors[i] & all_on;
    cells[i] = b2 & (b1 | cells[i]) & ~b4;
  }
}

int main(void) {
  unsigned int *gen1 = new unsigned int[size];
  randomizeCells(gen1);

  unsigned int *cells, *neighbors;

  cudaMalloc(&cells, space);
  cudaMalloc(&neighbors, space);
  unsigned int *result = new unsigned int[size];

  std::clock_t start, stop;
  start = std::clock();

  cudaMemcpy(cells, gen1, space, cudaMemcpyHostToDevice);
  for (int i = 0; i < gens; ++i) {
    neighborKernel<<<(size+255)/256, 256>>>(size, rows, cells, neighbors);
    lifeKernel<<<(size+255)/256, 256>>>(size, cells, neighbors);
  }
  cudaMemcpy(result, cells, space, cudaMemcpyDeviceToHost);

  stop = std::clock();
  float efficiency = float(long(rows) * rows * gens) / (stop - start) * CLOCKS_PER_SEC;

  cudaFree(cells);
  cudaFree(neighbors);

  std::cout << "C++ Efficiency in cellhz: " << efficiency << std::endl;
}
