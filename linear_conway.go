package main

import (
	"math/rand"
	"time"
)

type point struct{ x, y int }
type there struct{}

const target int = 1e8
const size = 100
const gens int = target / size / size
const threads = 4

var alive = make(map[point]there)
var new_changes map[point]bool
var neighbors = make(map[point]int)
var changed []point
var yes = there{}

func bound(n int) int {
	return (size + n) % size
}

func wrap(p point) point {
	p.x = bound(p.x)
	p.y = bound(p.y)
	return p
}

func set_alive(p point, b bool) {
	if b {
		alive[p] = yes
	} else {
		delete(alive, p)
	}
}

func each_neighbor(p point, fn func(point)) {
	for dx := -1; dx <= 1; dx++ {
		for dy := -1; dy <= 1; dy++ {
			if dx != 0 || dy != 0 {
				fn(wrap(point{p.x + dx, p.y + dy}))
			}
		}
	}
}

func update_neighbors(p point) {
	_, on := alive[p]

	each_neighbor(p, func(np point) {
		if on {
			neighbors[np]++
		} else {
			neighbors[np]--
			if neighbors[np] == 0 {
				delete(neighbors, np)
			}
		}
	})
}

func set(p point) {
	alive[p] = yes
	changed = append(changed, p)
	// to cover solitary points
	changed = append(changed, wrap(point{p.x + 1, p.y}))
	update_neighbors(p)
}

func randomize_cells() {
	for x := 0; x < size; x++ {
		for y := 0; y < size; y++ {
			p := point{x, y}
			if rand.Intn(2) == 0 {
				set(p)
			}
		}
	}
}

func print_cells() {
	print("\033[H\033[2J")
	for y := 0; y < size && y < 20; y++ {
		for x := 0; x < size && x < 20; x++ {
			_, on := alive[point{x, y}]
			if on {
				print("o")
			} else {
				print(" ")
			}
		}
		println()
	}
}

func propagate_from(p point) {
	each_neighbor(p, func(np point) {
		_, lived := alive[np]
		count := neighbors[np]
		lives := count == 3 || count == 2 && lived
		if lives != lived {
			new_changes[np] = lives
		}
	})
}

func each_change(fn func(point)) {
	for _, p := range changed {
		fn(p)
	}
}

func implement_new_changes() {
	changed = changed[:0]
	for p, lives := range new_changes {
		set_alive(p, lives)
		changed = append(changed, p)
	}
}

func next_gen() {
	new_changes = make(map[point]bool)
	each_change(propagate_from)
	implement_new_changes()
	each_change(update_neighbors)
}

func main() {
	rand.Seed(time.Now().UnixNano())
	randomize_cells()

	start := time.Now()
	for i := 0; i < gens; i++ {
		next_gen()
	}
	seconds := time.Since(start).Seconds()

	println("Go Efficiency in cellhz:", float64(gens*size*size)/seconds)
}
