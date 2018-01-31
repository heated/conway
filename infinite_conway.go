package main

import (
	"math/rand"
	"time"
)

type point struct {
	x, y int
}
type there struct{}

const size = 100
const gens = 1000

var alive = make(map[point]there)
var changed = make(map[point]there)
var neighbors = make(map[point]int8)
var yes = there{}

func set_alive(p point, b bool) {
	if b {
		alive[p] = yes
	} else {
		delete(alive, p)
	}
}

func update_neighbors(p point) {
	var inc int8

	_, on := alive[p]
	if on {
		inc = 1
	} else {
		inc = -1
	}

	for dx := -1; dx <= 1; dx++ {
		for dy := -1; dy <= 1; dy++ {
			if dx != 0 || dy != 0 {
				np := point{p.x + dx, p.y + dy}
				neighbors[np] += inc
				if neighbors[np] == 0 {
					delete(neighbors, np)
				}
			}
		}
	}
}

func set(p point) {
	alive[p] = yes
	changed[p] = yes
	// to cover solitary points
	changed[point{p.x + 1, p.y}] = yes
	update_neighbors(p)
}

func randomizeCells() {
	for x := 0; x < size; x++ {
		for y := 0; y < size; y++ {
			p := point{x, y}
			if rand.Intn(2) == 0 {
				alive[p] = yes
				changed[p] = yes
				// to cover solitary points
				changed[point{x + 1, y}] = yes
				update_neighbors(p)
			}
		}
	}
}

func printCells() {
	for x := 0; x < size; x++ {
		for y := 0; y < size; y++ {
			_, on := alive[point{x, y}]
			if on {
				print("O")
			} else {
				print(" ")
			}
		}
		println()
	}
	println()
}

func nextGeneration() {
	new_changes := make(map[point]there)

	// Whoops, iterate through neighbors of changes, not all neighbors
	for p := range changed {
		for dx := -1; dx <= 1; dx++ {
			for dy := -1; dy <= 1; dy++ {
				if dx != 0 || dy != 0 {
					np := point{p.x + dx, p.y + dy}
					_, lived := alive[np]
					count := neighbors[np]
					lives := count == 3 || count == 2 && lived
					if lives != lived {
						set_alive(np, lives)
						new_changes[np] = yes
					}
				}
			}
		}
	}

	for p := range new_changes {
		update_neighbors(p)
	}

	changed = new_changes
}

func main() {
	rand.Seed(time.Now().UnixNano())
	randomizeCells()

	start := time.Now()
	for i := 0; i < gens; i++ {
		nextGeneration()
	}
	seconds := time.Now().Sub(start).Seconds()

	println("Final number of recently changed cells:", len(changed))
	println("Final number of cells alive:", len(alive))
	println("Final count of neighbor knowledge:", len(neighbors))
	println("Go Efficiency in cellhz:", gens*size*size/seconds)
}
