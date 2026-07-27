# Cache Simulator

A command-line cache simulator written in C++ that models three cache mapping strategies: **Direct Mapped**, **Set Associative**, and **Fully Associative**. It reads a trace file of memory addresses, simulates cache behavior, and outputs a detailed step-by-step log showing how each access results in a hit or miss, including eviction decisions via LRU replacement.

## Cache Types

### Direct Mapped
Each memory address maps to exactly one cache line. Simple and fast, but suffers from conflict misses when multiple addresses compete for the same line.

```
Address: [tag | index | offset]
```

- **index** selects the cache line
- **tag** is compared against the stored tag to determine hit/miss
- On a miss, the existing line is always overwritten (no replacement policy needed)

### Set Associative (N-Way)
Each address maps to a **set**, which contains N slots (ways). Reduces conflict misses compared to direct mapped since each address has N possible locations.

```
Address: [tag | index | offset]
```

- **index** selects the set
- All N ways within the set are checked for a tag match
- On a miss with a full set, the **Least Recently Used (LRU)** way is evicted

### Fully Associative
Any address can go in any cache line. Eliminates conflict misses entirely, but requires checking every line on each access.

```
Address: [tag | offset]
```

- No index bits — the entire cache is one large set
- On a miss with a full cache, the **LRU** line is evicted

## Project Structure

```
Cache_Simulator/
├── Cache_Sim.cpp            # Main program: CLI parsing, trace reading, simulation loop
├── DirectMapped.h / .cpp    # Direct mapped cache implementation
├── SetAssociative.h / .cpp  # N-way set associative cache with LRU
├── FullyAssociative.h / .cpp# Fully associative cache with LRU
├── Makefile                 # Build configuration
└── sample.trace             # Example trace file
```

## Building

```bash
make
```

To clean:

```bash
make clean
```

## Usage

```
./cache_sim --type <dm|sa|fa> --cache-size <bytes> --block-size <bytes> [--ways <n>] --trace <file>
```

### Parameters

| Flag | Description |
|------|-------------|
| `--type` | Cache type: `dm` (Direct Mapped), `sa` (Set Associative), `fa` (Fully Associative) |
| `--cache-size` | Total cache size in bytes (must be a power of 2) |
| `--block-size` | Block/line size in bytes (must be a power of 2) |
| `--ways` | Number of ways per set (required only for `sa`) |
| `--trace` | Path to the trace file |

### Examples

```bash
# Direct Mapped: 256-byte cache, 64-byte blocks
./cache_sim --type dm --cache-size 256 --block-size 64 --trace sample.trace

# 4-Way Set Associative: 1024-byte cache, 64-byte blocks
./cache_sim --type sa --cache-size 1024 --block-size 64 --ways 4 --trace sample.trace

# Fully Associative: 256-byte cache, 64-byte blocks
./cache_sim --type fa --cache-size 256 --block-size 64 --trace sample.trace
```

## Trace File Format

One hex address per line (with or without the `0x` prefix). Empty lines are skipped.

```
0x0000
0x0040
0x0080
0x00C0
0x0000
0x0100
```

## Output

The simulator prints a detailed log for each access, followed by a summary.

### Direct Mapped Output

```
Direct Mapped Cache: 256 bytes, 64 bytes/block, 4 lines
Address breakdown: [tag:56 | index:2 | offset:6]

#1     addr: 0x0000  tag: 0x0000  index: 0    -> MISS (cold, line was empty)
#2     addr: 0x0040  tag: 0x0000  index: 1    -> MISS (cold, line was empty)
#3     addr: 0x0000  tag: 0x0000  index: 0    -> HIT
#4     addr: 0x0100  tag: 0x0001  index: 0    -> MISS (conflict, evicted tag 0x0000)

--- Summary ---
Total accesses: 4
Hits: 1  Misses: 3  Hit Rate: 25.00%
```

Each access shows:
- **HIT** when the tag matches and the line is valid
- **MISS (cold)** when the line was empty (compulsory miss)
- **MISS (conflict, evicted tag ...)** when an existing block was replaced

### Set Associative Output

```
2-Way Set Associative Cache: 256 bytes, 64 bytes/block, 2 sets
Address breakdown: [tag:57 | index:1 | offset:6]

#1     addr: 0x0000  tag: 0x0000  set: 0    -> MISS (cold, placed in way 0)  lru:[0]
#2     addr: 0x0080  tag: 0x0001  set: 0    -> MISS (cold, placed in way 1)  lru:[1,0]
#3     addr: 0x0000  tag: 0x0000  set: 0    -> HIT (way 0)  lru:[0,1]
#4     addr: 0x0100  tag: 0x0002  set: 0    -> MISS (evicted tag 0x0001 from way 1 via LRU)  lru:[1,0]
```

Additional details:
- **Which way** the block was placed in or found in
- **LRU order** after each access (most recently used first)
- **Which tag was evicted** and from which way when LRU replacement occurs

### Fully Associative Output

```
Fully Associative Cache: 256 bytes, 64 bytes/block, 4 lines
Address breakdown: [tag:58 | offset:6]

#1     addr: 0x0000  tag: 0x0000  -> MISS (cold, placed in line 0)  lru:[0]
#2     addr: 0x0040  tag: 0x0001  -> MISS (cold, placed in line 1)  lru:[1,0]
#3     addr: 0x0000  tag: 0x0000  -> HIT (line 0)  lru:[0,1]
#4     addr: 0x0100  tag: 0x0004  -> MISS (evicted tag 0x0001 from line 1 via LRU)  lru:[1,0]
```

Same detail as set associative, but with no index field (any block can go anywhere).

## How It Works

### Address Decomposition

Given a 64-bit memory address, the simulator splits it into fields using bit manipulation:

```
| tag (upper bits) | index (middle bits) | offset (lower bits) |
```

- **Offset bits** = log2(block size) — selects a byte within a block
- **Index bits** = log2(number of sets/lines) — selects which set/line to check
- **Tag bits** = remaining upper bits — identifies which memory block is stored

### Hit/Miss Detection

1. Extract the index and tag from the address
2. Look up the cache line(s) at that index
3. If any line has `valid == true` and a matching tag, it's a **hit**
4. Otherwise, it's a **miss** — the block is loaded and may evict an existing entry

### LRU Replacement

For set associative and fully associative caches, when all ways/lines are occupied and a miss occurs:

1. The **least recently used** entry (back of the LRU list) is selected as the victim
2. The victim's tag is overwritten with the new block's tag
3. The new entry moves to the **front** of the LRU list (most recently used)

On a hit, the accessed entry also moves to the front of the LRU list.

### Hit Rate Calculation

```
Hit Rate = Hits / (Hits + Misses) * 100%
```
