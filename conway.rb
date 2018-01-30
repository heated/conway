def game_of_life(board)
    next_gen = Array.new(board.size) { Array.new(board[0].size) }

    each_position(board) do |x, y|
        next_gen[x][y] = survives?(board, x, y) ? 1 : 0
    end

    board = next_gen
end

def each_position(board)
    board.size.times do |x|
        board[0].size.times do |y|
            yield(x, y)
        end
    end
end

def survives?(board, x, y)
    neighbors = count_living_neighbors(board, x, y)

    neighbors == 3 || (neighbors == 2 && alive?(board, x, y))
end

def count_living_neighbors(board, x, y)
    total = 0

    for dx in -1..1
        for dy in -1..1
            next if dx == 0 && dy == 0
            
            total += 1 if alive?(board, x + dx, y + dy)
        end
    end

    total
end

def valid_pos?(board, x, y)
  x >= 0 && x < board.size &&
  y >= 0 && y < board[0].size
end

def alive?(board, x, y)
    valid_pos?(board, x, y) && board[x][y] == 1
end
