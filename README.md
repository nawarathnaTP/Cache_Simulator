# Cache Simulator

A C++ cache simulator that reads a trace file of memory addresses and simulates cache behavior step-by-step.

### Cache Types
- **Direct Mapped** — each address maps to exactly one cache line
- **N-Way Set Associative** — each address maps to a set with N possible slots
- **Fully Associative** — any address can go in any cache line

### Features
- Trace file input (hex addresses, one per line)
- Step-by-step access log showing hits, cold misses, and conflict evictions
- LRU (Least Recently Used) replacement for set associative and fully associative
- Configurable cache size, block size, and associativity
- Per-run summary with hit rate

## Build & Run

```bash
make
./cache_sim --type <dm|sa|fa> --cache-size <bytes> --block-size <bytes> [--ways <n>] --trace <file>
```

| Flag | Description |
|------|-------------|
| `--type` | `dm` (Direct Mapped), `sa` (Set Associative), `fa` (Fully Associative) |
| `--cache-size` | Total cache size in bytes |
| `--block-size` | Block size in bytes |
| `--ways` | Ways per set (required for `sa`) |
| `--trace` | Trace file path (one hex address per line) |

## Examples

```bash
./cache_sim --type dm --cache-size 256 --block-size 64 --trace sample.trace
./cache_sim --type sa --cache-size 1024 --block-size 64 --ways 4 --trace sample.trace
./cache_sim --type fa --cache-size 256 --block-size 64 --trace sample.trace
```

## Sample Output

```
Fully Associative Cache: 256 bytes, 64 bytes/block, 4 lines
Address breakdown: [tag:58 | offset:6]

#1     addr: 0x0000  tag: 0x0000  -> MISS (cold, placed in line 0)  lru:[0]
#2     addr: 0x0040  tag: 0x0001  -> MISS (cold, placed in line 1)  lru:[1,0]
#3     addr: 0x0000  tag: 0x0000  -> HIT (line 0)  lru:[0,1]
#4     addr: 0x0100  tag: 0x0004  -> MISS (evicted tag 0x0001 from line 1 via LRU)  lru:[1,0]

--- Summary ---
Total accesses: 4
Hits: 1  Misses: 3  Hit Rate: 25.00%
```

## Project Structure

```
Cache_Sim.cpp              # CLI, trace parsing, simulation loop
DirectMapped.h / .cpp      # Direct mapped cache
SetAssociative.h / .cpp    # N-way set associative with LRU
FullyAssociative.h / .cpp  # Fully associative with LRU
```
