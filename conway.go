package main

import (
    "math/rand"
    "time"
)

const size = 1000
var cells [size][size]bool

func coinflip() bool {
    return rand.Intn(2) == 0
}

func randomizeCells() {
    for i := 0; i < size; i++ {
        for j := 0; j < size; j++ {
            cells[i][j] = coinflip()
        }
    }
}

func printCells() {
    for i := 0; i < size; i++ {
        for j := 0; j < size; j++ {
            if cells[i][j] {
                print("o")
            } else {
                print(" ")
            }
        }
        println()
    }
}

func nextGeneration() {
    var new_cells [size][size]bool

    for i := 0; i < size; i++ {
        for j := 0; j < size; j++ {
            neighbors := 0

            for dx := -1; dx <= 1; dx++ {
                for dy := -1; dy <= 1; dy++ {
                    x := (size + i + dx) % size
                    y := (size + j + dy) % size
                    if (dx != 0 || dy != 0) && cells[x][y] {
                        neighbors++
                    }
                }
            }

            new_cells[i][j] = neighbors == 3 || neighbors == 2 && cells[i][j]
        }
    }

    cells = new_cells
}

func main() {
    randomizeCells()

    start := time.Now()
    for i := 0; i < 10; i++ {
        nextGeneration()
    }
    seconds := time.Now().Sub(start).Seconds()

    println("Efficiency in cellhz:", 10 * size * size / seconds);
}
