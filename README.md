# Malicious-URL-IP-Hash-Filter

A compact, high-performance malicious URL/IP filter implemented in modern C++ using a custom hash map.

This repository demonstrates an implementation of a hash-table based filter optimized for real-world trade-offs between runtime and memory. It's written to be clear, efficient, and easy to reason about.

## Highlights

- Custom `UnorderedMap` implementation (separate chaining) with prime-sized bucket arrays for better distribution.
- Uses the FNV-1A hashing algorithm (implemented as `fnv1a_hash`) for fast, low-collision hashing of strings.
- Targeted load factor ~0.7–0.8 (constructor uses ~0.75) to balance memory use and expected O(1) lookup performance.
- Simple API: insert, find, erase, load factor inspection, iteration.

## Why this approach matters

Hash tables are common — but the details matter for a filter used in high-throughput environments. This project focuses on a few practical engineering choices:

- FNV-1A for speed: FNV-1A provides an excellent speed-to-quality ratio for short strings such as IP addresses and URLs. It's implemented in `src/hash_functions.cpp` as `fnv1a_hash` and used as the default hasher in the filter.
- Separate chaining + prime bucket counts: Buckets are prime-sized (via `primes.h`) to reduce clustering and modulo bias when mapping hash codes to buckets.
- Conservative load factor (~0.75): This keeps most buckets short (1–2 nodes average) so that lookups remain effectively constant time while avoiding excessive memory blow-up from large bucket arrays.

These choices make the code performant for a blocked-IP/URL lookup service while keeping memory usage reasonable — the exact balance that matters in production systems.

## Design & Implementation

- Language: C++17
- Key files:
	- `src/UnorderedMap.h` — custom hash map (separate chaining using singly linked lists). Exposes `insert`, `find`, `erase`, `load_factor`, iteration, and bucket inspection.
	- `src/hash_functions.cpp/.h` — contains `fnv1a_hash` and a polynomial rolling hash (used for experimentation). `fnv1a_hash` is the default used by the filter.
	- `src/malicious_url_filter.h` — small wrapper that loads `resources/block.txt` into the map and provides `is_Malicious_URL()`.
	- `src/main.cpp` — example usage and sanity check.

Core invariants and behavior:

- Bucket count is chosen as the next greater prime of the requested size (see `primes.h`).
- Load factor is computed as `size() / bucket_count()` and the constructor for the filter targets ~0.75 to initialize the bucket array size.
- Collision resolution is handled with chaining: each bucket contains a linked list of entries; insertion prepends to the bucket's list.

Complexity (expected):

- Average-case lookup/insert/erase: O(1) when load factor is kept < 1.0 (practically O(1) near the target 0.7–0.8).
- Worst-case: O(n) for degenerate hash distributions (mitigated by FNV-1A and prime bucket sizing).

## How to build & run

The project is intentionally minimal and uses plain g++ for builds. From the repository root (macOS, zsh):

```
# build
g++ -g -std=c++17 -Wall -Wextra -pedantic-errors -Weffc++ -Wno-unused-parameter -fsanitize=undefined src/*.cpp -o malicious_filter

# run
./malicious_filter
```

Alternatively, open the workspace in VS Code and use the provided build task (label: `C/C++: g++ build active file`) and the `Run` task.

Example run output (from `src/main.cpp`):

```
217.60.239.0/24 not found.
Load factor: 0.75
```

Adjust `resources/block.txt` (the sample block list) to add IPs/URLs for detection.

## Contract

- Input: a string (IP or URL) to check against a pre-loaded block list.
- Output: boolean — `true` if the string exists in the block list, otherwise `false`.
- Error modes: missing or unreadable `resources/block.txt` will result in an empty filter. The implementation is defensive about empty input and exposes `load_factor()` so callers can validate capacity expectations.

## Edge cases considered

- Empty block lists — results in an empty map with safe iteration and lookups.
- Duplicate entries — `insert` returns whether the insert succeeded or if the key already existed (no duplicate keys allowed).
- Strings with unexpected characters — hashing operates on bytes of the string, so valid but unusual strings are supported.

## Benchmarks & notes

This repo contains no heavy benchmarking harness, but the design choices prioritize:

- Low per-lookup latency (short linked lists, fast integer math in FNV-1A).
- Predictable memory usage (prime bucket sizing + controlled load factor).

If you want microbenchmarks, I can add a simple harness using <chrono> to measure thousands of lookups/inserts and produce averages.

## Contributing

This repo is intentionally compact. If you find a bug or want to add benchmarks/tests, open an issue or a PR. I review and respond quickly.

