package main

import (
	"math/rand"
	"sync"
	"time"
)

const total int = 1e7
const size = 1000
const gens = total / size / size
const threads = 4
const rpt = size / threads

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
    for _, col := range cells {
        for _, cell := range col {
			if cell {
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
	var wg sync.WaitGroup
	wg.Add(threads)

	for t := 0; t < threads; t++ {
		go func(start_row int) {
			for i := start_row; i < start_row+rpt; i++ {
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
			wg.Done()
		}(t * rpt)
	}

	wg.Wait()
	cells = new_cells
}

func main() {
	rand.Seed(time.Now().UnixNano())
	randomizeCells()

	start := time.Now()
	for i := 0; i < gens; i++ {
		nextGeneration()
	}
	seconds := time.Now().Sub(start).Seconds()

	println("Go Efficiency in cellhz:", 10*size*size/seconds)
}
