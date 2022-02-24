use rand::Rng;
use std::mem;
use std::time::Instant;

const ROWS: usize = 1 << 8;
const COLS: usize = ROWS / 128;
const TOTAL_CELLS: u64 = ROWS.pow(2) as u64;
const TARGET_LOAD: u64 = 3_000_000_000;

// want at least 1 but std::cmp::max isn't a const fn
const GENS: u64 = 1 + TARGET_LOAD / TOTAL_CELLS;
const TOTAL_CELL_UPDATES: u64 = GENS * TOTAL_CELLS;

type Grid = [[u128; COLS]; ROWS];

fn next_generation(cells: &mut Grid, buffer: &mut Grid) {
    for y in 0..ROWS {
        for x in 0..COLS {
            let (mut b1, mut b2, mut b4) = (0u128, 0u128, 0u128);

            for dy in 0..3 {
                for dx in 0..3 {
                    if dy != 1 || dx != 1 {
                        let ny = (ROWS + y+dy - 1) % ROWS;
                        let nx = (COLS + x+dx - 1) % COLS;
                        let last = cells[ny][nx];
                        let mut alive = cells[ny][x];

                        match dx {
                            2 => {
                                alive <<= 1;
                                alive |= last >> 127;
                            },
                            0 => {
                                alive >>= 1;
                                alive |= last << 127;
                            }
                            _ => {}
                        }
                        
                        let c2 = alive & b1;
                        let c4 = c2 & b2;
                        b1 ^= alive;
                        b2 ^= c2;
                        b4 |= c4;
                    }
                }
            }

            buffer[y][x] = b2 & (b1 | cells[y][x]) & !b4;
        }
    }
}

fn randomize_cells(cells: &mut Grid) {
    let mut rng = rand::thread_rng();

    for y in 0..ROWS {
        for x in 0..COLS {
            cells[y][x] = rng.gen();
        }
    }
}

#[allow(dead_code)]
fn print_cells(cells: &Grid) {
    for y in 0..16 {
        for x in 0..32 {
            let on = ((cells[y][0] >> (127-x))&1) == 1;
            print!("{}", if on { "o" } else { " " });
        }
        println!();
    }
}

fn main() {
    // heap allocate
    let mut cells_box = Vec::from([[[0; COLS]; ROWS]; 1]).into_boxed_slice();
    let mut buf_box = cells_box.clone();
    let mut cells = &mut cells_box[0];
    let mut buffer = &mut buf_box[0];

    randomize_cells(cells);
    let start = Instant::now();

    for _ in 0..GENS {
        next_generation(cells, buffer);

        // just swap refs, not data
        mem::swap(&mut cells, &mut buffer);
    }

    let duration = Instant::now() - start;
    let cellghz = TOTAL_CELL_UPDATES as f32 / duration.as_nanos() as f32;
    // print_cells(&cells);
    println!("{:.1} cellghz", cellghz);
}
