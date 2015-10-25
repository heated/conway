class Board:
	'''if life is a game, this is a playing field'''
	def __init__(self, living_cell_positions = []):
		self.living_cells = {Cell(self, Position(x, y)) for x, y in living_cell_positions}

	def cells_that_could_be_alive_soon(self):
		all_neighbors = {cell for living_cell in self.living_cells for cell in living_cell.neighbors()}
		return list(all_neighbors.union(self.living_cells))

	def next_generation(self):
		return {cell for cell in self.cells_that_could_be_alive_soon() if cell.will_survive()}

	def step(self):
		self.living_cells = self.next_generation()
		return self.living_cells

	def __str__(self):
		return str(self.living_cells)

class Cell:
	'''a single cell in the game of life'''
	def __init__(self, board, pos):
		self.board = board
		self.pos = pos

	def is_alive(self):
		return self in self.board.living_cells

	def neighbors(self):
		return [Cell(self.board, pos) for pos in self.pos.neighbors()]

	def number_of_living_neighbors(self):
		return len([cell for cell in self.neighbors() if cell.is_alive()])

	def will_survive(self):
		if self.number_of_living_neighbors() == 3:
			return True
		elif self.number_of_living_neighbors() != 2:
			return False
		else:
			return self.is_alive()

	def __eq__(self, other):
		return isinstance(other, Cell) and self.pos == other.pos

	def __hash__(self):
		return hash(self.pos)

	def __str__(self):
		return str(self.pos)

	def __repr__(self):
		return self.__str__()

class Position:
	'''a position on a grid'''
	def __init__(self, x, y):
		self.x = x
		self.y = y

	def north(self):
		return Position(self.x, self.y - 1)

	def east(self):
		return Position(self.x + 1, self.y)

	def south(self):
		return Position(self.x, self.y + 1)

	def west(self):
		return Position(self.x - 1, self.y)

	def neighbors(self):
		return [
			self.north(),
			self.east(),
			self.south(),
			self.west(),
			self.north().east(),
			self.north().west(),
			self.south().east(),
			self.south().west()
		]

	def __eq__(self, other):
		return self.x == other.x and self.y == other.y

	def __hash__(self):
		return hash((self.x, self.y))

	def __str__(self):
		return str((self.x, self.y))

	def __repr__(self):
		return self.__str__()

sample_board = Board([(0, 0), (0, 1), (0, 2)])
for i in range(10):
	print sample_board.step()
