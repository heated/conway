use crossbeam::thread;
use rand::Rng;
use std::mem;
use std::sync::{Barrier, Mutex, RwLock};
use std::time::Instant;

const ROWS: usize = 1 << 12;
const COLS: usize = ROWS / 128;
const TOTAL_CELLS: u64 = ROWS.pow(2) as u64;
const TARGET_LOAD: u64 = 5_000_000_000;

// want at least 1 but std::cmp::max isn't a const fn
const GENS: u64 = 1 + TARGET_LOAD / TOTAL_CELLS;
const LOAD: u64 = GENS * TOTAL_CELLS;
const NUM_WORKERS: usize = 2;
const WORKER_ROWS: usize = ROWS / NUM_WORKERS;

type Chunk = [[u128; COLS]; WORKER_ROWS];

fn next_generation(chunks: &RwLock<Vec<&mut Chunk>>,
                   buf_chunk: &Mutex<&mut Chunk>,
                   chunk_i: usize) {
    let chunks_r = chunks.read().unwrap();
    let mut buf_chunk_w = buf_chunk.lock().unwrap();

    for y in 0..WORKER_ROWS {
        let glob_y = WORKER_ROWS*chunk_i + y;

        for x in 0..COLS {
            let (mut b1, mut b2, mut b4) = (0u128, 0u128, 0u128);

            for dy in 0..3 {
                for dx in 0..3 {
                    if dy != 1 || dx != 1 {
                        let ny = (ROWS + glob_y+dy - 1) % ROWS;
                        let nz = ny / WORKER_ROWS;
                        let nly = ny % WORKER_ROWS;

                        let nx = (COLS + x+dx - 1) % COLS;
                        let last = chunks_r[nz][nly][nx];
                        let mut n_alive = chunks_r[nz][nly][x];

                        match dx {
                            2 => {
                                n_alive <<= 1;
                                n_alive |= last >> 127;
                            },
                            0 => {
                                n_alive >>= 1;
                                n_alive |= last << 127;
                            },
                            _ => {}
                        }
                        
                        let c2 = n_alive & b1;
                        let c4 = c2 & b2;
                        b1 ^= n_alive;
                        b2 ^= c2;
                        b4 |= c4;
                    }
                }
            }

            let alive = chunks_r[chunk_i][y][x];
            buf_chunk_w[y][x] = b2 & (b1 | alive) & !b4;
        }
    }
}

fn randomize_chunks(chunks: &RwLock<Vec<&mut Chunk>>) {
    let mut rng = rand::thread_rng();
    let mut chunks_w = chunks.write().unwrap();

    for z in 0..NUM_WORKERS {
        for y in 0..WORKER_ROWS {
            for x in 0..COLS {
                chunks_w[z][y][x] = rng.gen();
            }
        }
    }
}

#[allow(dead_code)]
fn print_cells(chunks: &RwLock<Vec<&mut Chunk>>) {
    let chunks_r = chunks.read().unwrap();

    for y in 0..16 {
        for x in 0..32 {
            let on = ((chunks_r[0][y][0] >> (127-x))&1) == 1;
            print!("{}", if on { "o" } else { " " });
        }
        println!();
    }
}

fn main() {
    // heap allocate contiguously
    let mut raw_chunks = vec![[[0; COLS]; WORKER_ROWS]; NUM_WORKERS].into_boxed_slice();
    let mut raw_buf_chunks = raw_chunks.clone();

    let mut chunks = RwLock::new(
        raw_chunks.chunks_exact_mut(1)
        .map(|c| &mut c[0])
        .collect());
    let buf_chunks: Vec<Mutex<&mut Chunk>> = raw_buf_chunks
        .chunks_exact_mut(1)
        .map(|c| Mutex::new(&mut c[0]))
        .collect();

    randomize_chunks(&mut chunks);
    let barrier = Barrier::new(1 + NUM_WORKERS);

    thread::scope(|s| {
        for worker_i in 0..NUM_WORKERS {
            let barrier_ref = &barrier;
            let chunks_ref = &chunks;
            let buf_chunk_ref = &buf_chunks[worker_i];

            s.spawn(move |_| {
                for _ in 0..GENS {
                    barrier_ref.wait();
                    next_generation(chunks_ref, buf_chunk_ref, worker_i);
                    barrier_ref.wait();
                }
            });
        }

        let start = Instant::now();

        for _ in 0..GENS {
            barrier.wait();
            barrier.wait();

            let mut chunks_w = chunks.write().unwrap();
            for i in 0..NUM_WORKERS {
                let chunk_ref = &mut chunks_w[i];
                let buf_chunk_ref = &mut *buf_chunks[i].lock().unwrap();
                mem::swap(chunk_ref, buf_chunk_ref);
            }
        }

        let duration = Instant::now() - start;
        let cellghz = LOAD as f32 / duration.as_nanos() as f32;
        println!("{:.1} cellghz", cellghz);
    }).unwrap();

    // print_cells(&chunks);
}
