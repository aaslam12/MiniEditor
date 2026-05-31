# Benchmark Results

All results below were captured from a Release build (`-O3`) on Linux x86-64.

```bash
python3 build.py --config Release --stress-test
```

Benchmarks below are with Palloc enabled. This workload is slower with Palloc in the treap path, but I kept it anyway because real use can expose allocator issues that are hard to predict while designing the allocator.

---

## Front Insertions & Deletions

Stress-tests the hardest case for most editors: repeated edits at position 0.

| Operation | Count | Total Time | Avg per op |
|-----------|------:|-----------:|-----------:|
| Insert at front | 100,000 | 19.11 ms | 0.191 µs |
| Delete from front | 100,000 | 10.23 ms | 0.102 µs |

---

## Alternating Insert / Delete

Rapid alternation between inserts and deletes at random positions.

| Metric | Result |
|--------|-------:|
| Cycles | 50,000 |
| Total time | 37.97 ms |
| Avg per cycle | 0.759 µs |
| Full string rebuild | 0.009 ms |

---

## Random Edits (1 MB buffer)

500k random insertions and deletions across a 1 million character buffer.

| Metric | Result |
|--------|-------:|
| Operations | 500,000 |
| Total time | 431.8 ms |
| Avg per edit | 0.864 µs |
| Full reconstruction | 2.4 ms |

---

## Tree Insertion Throughput (1M pieces)

Measures raw piece insertion speed building a tree of 1 million nodes.

| Metric | Result |
|--------|-------:|
| Total pieces inserted | 1,000,000 |
| Total time | 0.151 s |
| Avg per insertion | 0.151 µs |
| Throughput | ~75 MB/s |
| Total data | 11.3 MB |
| Peak RAM | ~88.9 MB |

---

## Line Access — `get_line` (O(log n))

Random `get_line` reads across a table built from 10,000 individually inserted pieces.

| Metric | Result |
|--------|-------:|
| Reads | 50,000 |
| Total time | 21.1 ms |
| Avg per read | 0.422 µs |

---

## Line Access — `get_line` on 100k-line file

| Metric | Result |
|--------|-------:|
| Lines in file | 100,000 |
| Random `get_line` reads | 10,000 |
| Avg per `get_line` | 0.772 µs |
| Random `get_index_for_line` lookups | 1,000 |
| Avg per `get_index_for_line` | 0.566 µs |
| Random newline inserts | 1,000 |
| Avg per newline insert | 0.741 µs |

---

## `get_index_for_line` on 10M-piece tree

| Metric | Result |
|--------|-------:|
| Pieces in tree | 10,000,000 |
| Search time (single lookup) | 0.0035 ms |

---

## Flamegraphs

Interactive SVG flamegraphs live in [`flamegraphs/`](../flamegraphs/).

| Benchmark | Flamegraph |
|-----------|------------|
| Alternating insert/delete | [`stress_alternating_ops.svg`](../flamegraphs/stress_alternating_ops.svg) |
| Front insertions/deletions | [`stress_front_ops.svg`](../flamegraphs/stress_front_ops.svg) |
| 10M piece tree insertion | [`stress_get_index.svg`](../flamegraphs/stress_get_index.svg) |
| Random `get_line` (10k pieces) | [`stress_get_line.svg`](../flamegraphs/stress_get_line.svg) |
| 100k line file access | [`stress_newlines.svg`](../flamegraphs/stress_newlines.svg) |
| Random edits (1MB buffer) | [`stress_random_edits.svg`](../flamegraphs/stress_random_edits.svg) |
