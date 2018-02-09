package main

import (
	"math/rand"
	"sync"
	"time"
)

const target int = 1e9
const rows = 1 << 13
const cols = rows / 16
const gens int = target / rows / rows
const threads = 4
const batch = rows / threads
const allOn = 0x1111111111111111

var cells [rows][cols]uint64
var neighbors [rows][cols]uint64

func randomizeCells() {
	for x := 0; x < rows; x++ {
		for y := 0; y < cols; y++ {
			cells[x][y] = rand.Uint64() & allOn
		}
	}
}

func printCells() {
	print("\033[H\033[2J")
	for x := 0; x < rows && x < 16; x++ {
		for y := 0; y <= 16; y++ {
			if (cells[x][0]>>uint(60-y*4))&1 == 1 {
				print("o")
			} else {
				print(" ")
			}
		}
		println()
	}
}

func next(alive uint64, n uint64) uint64 {
	b4 := (n & (allOn << 2)) >> 2
	b2 := (n & (allOn << 1)) >> 1
	b1 := n & allOn
	return b2 & (b1 | alive) &^ b4
}

func boundX(x int) int {
	return (rows + x) % rows
}

func boundY(y int) int {
	return (cols + y) % cols
}

func nextGen() {
	var wg sync.WaitGroup
	wg.Add(threads)

	for t := 0; t < threads; t++ {
		go func(start_row int) {
			for dx := -1; dx <= 1; dx++ {
				for dy := -1; dy <= 1; dy++ {
					if dx != 0 || dy != 0 {
						for x := start_row; x < start_row+batch; x++ {
							row := cells[boundX(x+dx)]
							for y := 0; y < cols; y++ {
								var moved uint64
								switch dy {
								case 0:
									moved = row[y]
								case 1:
									most := row[y] << 4
									last := row[boundY(y+1)] >> 60
									moved = (most | last)
								case -1:
									most := row[y] >> 4
									last := row[boundY(y-1)] << 60
									moved = (most | last)
								}
								neighbors[x][y] += moved
							}
						}
					}
				}
			}
			wg.Done()
		}(t * batch)
	}

	wg.Wait()
	wg.Add(threads)

	for t := 0; t < threads; t++ {
		go func(start_row int) {
			for x := start_row; x < start_row+batch; x++ {
				for y := 0; y < cols; y++ {
					cells[x][y] = next(cells[x][y], neighbors[x][y])
					neighbors[x][y] = 0
				}
			}
			wg.Done()
		}(t * batch)
	}

	wg.Wait()
}

func main() {
	rand.Seed(time.Now().UnixNano())
	randomizeCells()

	start := time.Now()
	for i := 0; i < gens; i++ {
		nextGen()
	}
	seconds := time.Since(start).Seconds()

	println("Go Efficiency in cellhz:", float64(gens*rows*rows)/seconds)
}
