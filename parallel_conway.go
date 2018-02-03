package main

import (
	"math/rand"
	"sync"
	"time"
)

const total int = 1e8
const size = 1024
const gens int = total / size / size
const threads = 4
const batch = size / threads

type grid = [size][size]bool

var cells grid
var new_cells grid

func randomizeCells() {
	for y := 0; y < size; y++ {
		for x := 0; x < size; x++ {
			cells[y][x] = rand.Intn(2) == 0
		}
	}
}

func printCells() {
	print("\033[H\033[2J")
	for y := 0; y < size && y < 20; y++ {
		for x := 0; x < size && x < 20; x++ {
			if cells[y][x] {
				print("o")
			} else {
				print(" ")
			}
		}
		println()
	}
}

func nextGeneration() {
	var wg sync.WaitGroup
	wg.Add(threads)

	for t := 0; t < threads; t++ {
		go func(start_col int) {
			for y := start_col; y < start_col+batch; y++ {
				for x := 0; x < size; x++ {
					neighbors := 0

					for dy := -1; dy <= 1; dy++ {
						for dx := -1; dx <= 1; dx++ {
							ny := (size + y + dx) % size
							nx := (size + x + dy) % size
							if (dx != 0 || dy != 0) && cells[ny][nx] {
								neighbors++
							}
						}
					}

					new_cells[y][x] = neighbors == 3 || neighbors == 2 && cells[y][x]
				}
			}
			wg.Done()
		}(t * batch)
	}

	wg.Wait()
	cells, new_cells = new_cells, cells
}

func main() {
	rand.Seed(time.Now().UnixNano())
	randomizeCells()

	start := time.Now()
	for i := 0; i < gens; i++ {
		nextGeneration()
	}
	seconds := time.Since(start).Seconds()

	println("Go Efficiency in cellhz:", float64(gens*size*size)/seconds)
}
