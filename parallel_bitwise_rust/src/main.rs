use rand::Rng;
use std::mem;
use std::sync::{Arc, Barrier, RwLock, RwLockReadGuard};
use std::thread;
use std::time::Instant;

const ROWS: usize = 1 << 12;
const COLS: usize = ROWS / 128;
const TOTAL_CELLS: u64 = ROWS.pow(2) as u64;
const TARGET_LOAD: u64 = 5_000_000_000;
const GENS: u64 = TARGET_LOAD / TOTAL_CELLS;
const LOAD: u64 = GENS * TOTAL_CELLS;
const NUM_WORKERS: usize = 2;
const WORKER_ROWS: usize = ROWS / NUM_WORKERS;

type Chunk = [[u128; COLS]; WORKER_ROWS];
type Grid = [Chunk; NUM_WORKERS];

fn next_generation(chunks: Vec<RwLockReadGuard<Chunk>>, 
                   buf_chunk: &mut Chunk, 
                   chunk_i: usize) {
    for y in 0..WORKER_ROWS {
        let glob_y = WORKER_ROWS*chunk_i + y;

        for x in 0..COLS {
            let (mut b1, mut b2, mut b4) = (0u128, 0u128, 0u128);

            for dy in 0..3 {
                for dx in 0..3 {
                    if dy == 1 && dx == 1 {
                        continue;
                    }

                    let ny = (ROWS + glob_y+dy - 1) % ROWS;
                    let nz = ny / WORKER_ROWS;
                    let nly = ny % WORKER_ROWS;

                    let nx = (COLS + x+dx - 1) % COLS;
                    let last = chunks[nz][nly][nx];
                    let mut alive = chunks[nz][nly][x];

                    match dx {
                        2 => {
                            alive <<= 1;
                            alive |= last >> 127;
                        },
                        0 => {
                            alive >>= 1;
                            alive |= last << 127;
                        },
                        _ => {}
                    }
                    
                    let c2 = alive & b1;
                    let c4 = c2 & b2;
                    b1 ^= alive;
                    b2 ^= c2;
                    b4 |= c4;
                }
            }

            let alive = chunks[chunk_i][y][x];
            buf_chunk[y][x] = b2 & (b1 | alive) & !b4;
        }
    }
}

fn randomize_cells(cells: &mut Grid) {
    let mut rng = rand::thread_rng();

    for z in 0..NUM_WORKERS {
        for y in 0..WORKER_ROWS {
            for x in 0..COLS {
                cells[z][y][x] = rng.gen();
            }
        }
    }
}

#[allow(dead_code)]
fn print_cells(chunks: &Vec<RwLockReadGuard<Chunk>>) {
    for y in 0..16 {
        for x in 0..32 {
            let on = ((chunks[0][y][0] >> (127-x))&1) == 1;
            print!("{}", if on { "o" } else { " " });
        }
        println!();
    }
}

fn main() {
    let mut cells = [[[0; COLS]; WORKER_ROWS]; NUM_WORKERS];
    let buffer = cells;

    randomize_cells(&mut cells);
    let mut     chunks = Vec::with_capacity(NUM_WORKERS);
    let mut buf_chunks = Vec::with_capacity(NUM_WORKERS);

    for i in 0..NUM_WORKERS {
            chunks.push(Arc::new(RwLock::new(cells[i])));
        buf_chunks.push(Arc::new(RwLock::new(buffer[i])));
    }

    let mut handles = Vec::with_capacity(NUM_WORKERS);
    let barrier = Arc::new(Barrier::new(1 + NUM_WORKERS));

    for worker_i in 0..NUM_WORKERS {
        let barrier_c = Arc::clone(&barrier);
        let chunks_c: Vec<Arc<RwLock<Chunk>>> = chunks.iter()
            .map(|c| c.clone()).collect();
        let buf_chunk_c = Arc::clone(&buf_chunks[worker_i]);

        handles.push(thread::spawn(move || {
            barrier_c.wait();

            for _ in 0..GENS {
                {
                    let chunks = chunks_c.iter()
                        .map(|c| c.read().unwrap())
                        .collect();
                    let mut buf_chunk = buf_chunk_c.write().unwrap();
                    next_generation(chunks, &mut buf_chunk, worker_i);
                }

                barrier_c.wait();
                barrier_c.wait();
            }
        }));
    }

    let start = Instant::now();
    barrier.wait();

    for _ in 0..GENS {
        barrier.wait();
        for i in 0..NUM_WORKERS {
            let     chunk = &mut     *chunks[i].write().unwrap();
            let buf_chunk = &mut *buf_chunks[i].write().unwrap();
            mem::swap(chunk, buf_chunk);
        }
        barrier.wait();
    }

    let duration = Instant::now() - start;
    let cellghz = LOAD as f32 / 1000.0 / duration.as_micros() as f32;
    println!("{:.1} cellghz", cellghz);

    for handle in handles {
        handle.join().unwrap();
    }

    // let unpacked = chunks.iter()
    //     .map(|c| c.read().unwrap())
    //     .collect();
    // print_cells(&unpacked);
}
