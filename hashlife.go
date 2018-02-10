package main

import (
	"math/rand"
	"time"
)

type Node struct {
	nw, ne, sw, se *Node
	alive          bool
	depth          uint
}

func (n *Node) center() *Node { return NodeFrom(n.nw.se, n.ne.sw, n.sw.ne, n.se.nw) }
func (n *Node) north() *Node  { return NodeFrom(n.nw.ne, n.ne.nw, n.nw.se, n.ne.sw) }
func (n *Node) south() *Node  { return NodeFrom(n.sw.ne, n.se.nw, n.sw.se, n.se.sw) }
func (n *Node) west() *Node   { return NodeFrom(n.nw.sw, n.nw.se, n.sw.nw, n.sw.ne) }
func (n *Node) east() *Node   { return NodeFrom(n.ne.sw, n.ne.se, n.se.nw, n.se.ne) }

var DEAD = &Node{}
var LIVE = &Node{alive: true}
var memo = make(map[[4]*Node]*Node)

func NodeFrom(nw, ne, sw, se *Node) *Node {
	nodes := [4]*Node{nw, ne, sw, se}
	result, exists := memo[nodes]
	if exists {
		return result
	} else {
		memo[nodes] = &Node{nw, ne, sw, se, false, nw.depth + 1}
		return memo[nodes]
	}
}

func lives(cells [9]*Node) *Node {
	alive := cells[4].alive
	cells[4] = DEAD
	neighbors := 0
	for _, cell := range cells {
		if cell.alive {
			neighbors++
		}
	}
	if neighbors == 3 || neighbors == 2 && alive {
		return LIVE
	} else {
		return DEAD
	}
}

var nextMemo = make(map[*Node]*Node)

// Calculate the center() 1<<(depth-2) generations into the future!
func (n *Node) nextInner() *Node {
	result, exists := nextMemo[n]
	if exists {
		return result
	} else {
		if n.depth == 2 {
			nextMemo[n] = NodeFrom(
				lives([9]*Node{n.nw.nw, n.nw.ne, n.ne.nw, n.nw.sw, n.nw.se, n.ne.sw, n.sw.nw, n.sw.ne, n.se.nw}),
				lives([9]*Node{n.nw.ne, n.ne.nw, n.ne.ne, n.nw.se, n.ne.sw, n.ne.se, n.sw.ne, n.se.nw, n.se.ne}),
				lives([9]*Node{n.nw.sw, n.nw.se, n.ne.sw, n.sw.nw, n.sw.ne, n.se.nw, n.sw.sw, n.sw.se, n.se.sw}),
				lives([9]*Node{n.nw.se, n.ne.sw, n.ne.se, n.sw.ne, n.se.nw, n.se.ne, n.sw.se, n.se.sw, n.se.se}),
			)
		} else {
			nextMemo[n] = NodeFrom(
				NodeFrom(n.nw.nextInner(), n.north().nextInner(), n.west().nextInner(), n.center().nextInner()).nextInner(),
				NodeFrom(n.north().nextInner(), n.ne.nextInner(), n.center().nextInner(), n.east().nextInner()).nextInner(),
				NodeFrom(n.west().nextInner(), n.center().nextInner(), n.sw.nextInner(), n.south().nextInner()).nextInner(),
				NodeFrom(n.center().nextInner(), n.east().nextInner(), n.south().nextInner(), n.se.nextInner()).nextInner(),
			)
		}
		return nextMemo[n]
	}
}

// Propagate node out by 1<<(depth-1) generations to double the size and thus, overall time!
func (n *Node) expand2x() *Node {
	e := EmptyOfDepth(n.depth)
	return NodeFrom(
		NodeFrom(e, e, e, n).nextInner(),
		NodeFrom(e, e, n, e).nextInner(),
		NodeFrom(e, n, e, e).nextInner(),
		NodeFrom(n, e, e, e).nextInner(),
	)
}

func (n *Node) CellString() string {
	if n.alive {
		return "o"
	} else {
		return " "
	}
}

func (n *Node) RowString(row int) string {
	if n.depth == 0 {
		return n.CellString()
	} else {
		height := 1 << n.depth
		if row < height/2 {
			return n.nw.RowString(row) + n.ne.RowString(row)
		} else {
			return n.sw.RowString(row-height/2) + n.se.RowString(row-height/2)
		}
	}
}

func (n *Node) String() string {
	height := 1 << n.depth
	result := "\033[H\033[2J"
	for i := 0; i < height; i++ {
		result += n.RowString(i) + "\n"
	}
	return result
}

var emptyMemo = make(map[uint]*Node)

func EmptyOfDepth(d uint) *Node {
	result, exists := emptyMemo[d]
	if exists {
		return result
	} else {
		child := EmptyOfDepth(d - 1)
		emptyMemo[d] = NodeFrom(child, child, child, child)
		return emptyMemo[d]
	}
}

func RandomOfDepth(d int) *Node {
	if d == 0 {
		if rand.Intn(2) == 1 {
			return LIVE
		} else {
			return DEAD
		}
	} else {
		return NodeFrom(
			RandomOfDepth(d-1),
			RandomOfDepth(d-1),
			RandomOfDepth(d-1),
			RandomOfDepth(d-1),
		)
	}
}

func volumeOfPyramid(n int) int {
	return 2 * n * (n + 1) * (2*n + 1) / 3
}

func main() {
	emptyMemo[0] = DEAD
	expansions := 10000
	startDepth := 4
	rand.Seed(time.Now().UnixNano())
	board := RandomOfDepth(startDepth)

	start := time.Now()
	for i := 0; i < expansions; i++ {
		board = board.expand2x()
	}
	seconds := time.Since(start).Seconds()

	println("Size of memo:", len(memo))
	println("Size of nextMemo:", len(nextMemo))
	println("Total number of doublings in size:", expansions)
	println("Time elapsed:", seconds, "seconds")
}
