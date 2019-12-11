#include <metal_stdlib>
#import "Constants.h"

using namespace metal;

kernel void nextGen(const device uint* cells,
                    device uint* newCells,
                    uint i [[ thread_position_in_grid ]]) {
    uint b1 = 0;
    uint b2 = 0;
    uint b4 = 0;
    
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx != 0 || dy != 0) {
                int ny = (size + i + dy * cols) % size;
                int nx = (size + ny + dx) % size;
                uint alive = cells[ny];
                uint last = cells[nx];

                switch (dx) {
                case 1:
                    alive <<= 1;
                    last >>= 31;
                    alive |= last;
                    break;
                case -1:
                    alive >>= 1;
                    last <<= 31;
                    alive |= last;
                    break;
                }

                uint c2 = alive & b1;
                uint c4 = c2 & b2;
                b1 ^= alive;
                b2 ^= c2;
                b4 |= c4;
            }
        }
    }

    newCells[i] = b2 & (b1 | cells[i]) & ~b4;
}

vertex float4 vertexShader(const device float2* vertices,
                           uint vid [[ vertex_id ]]) {
    return float4(vertices[vid], 0, 1);
}

fragment float4 fragmentShader(const device uint* cells,
                               float4 pos [[ position ]]) {
    if (pos.x >= width || pos.y >= height) {
        return float4(.8, .8, .8, 1);
    }
    
    int i = int(pos.y) * width + pos.x;
    int shift = 31 - i % 32;
    bool on = cells[i/32] & (1<<shift);
    return on ? float4(1, 1, 1, 1) : float4(0, 0, 0, 1);
}
