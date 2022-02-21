use rand::Rng;
use std::mem;
use std::time::Instant;

const ROWS: usize = 1 << 8;
const COLS: usize = ROWS / 128;
const TOTAL_CELLS: u64 = ROWS.pow(2) as u64;
const TARGET_LOAD: u64 = 1_000_000_000;
const GENS: u64 = TARGET_LOAD / TOTAL_CELLS;
const LOAD: u64 = GENS * TOTAL_CELLS;

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

    mem::swap(cells, buffer);
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
    let mut cells = [[0; COLS]; ROWS];
    let mut buffer = [[0; COLS]; ROWS];
    randomize_cells(&mut cells);
    let start = Instant::now();

    for _ in 0..GENS {
        next_generation(&mut cells, &mut buffer);
    }

    let duration = Instant::now() - start;
    let cellghz = LOAD as f32 / 1000.0 / duration.as_micros() as f32;
    // print_cells(&cells);
    println!("{:.1} cellghz", cellghz);
}
