# MiniEditor

A C++20 terminal text editor built around a piece table and an implicit treap. It stays fast on large files, keeps the UI minimal, and uses Palloc-backed treap nodes for allocator exploration.

`C++20` · `CMake` · `Catch2` · `PDCurses` · `Linux` · `Palloc`

---

## Highlights

- Piece table + implicit treap core for O(log n) edits.
- Batched insertions up to 512 bytes.
- Terminal UI with cursor movement, scrolling, save, and quit.
- Debug unit tests and Release stress tests via `build.py`.
- Full benchmark tables live in [`docs/BENCHMARKS.md`](docs/BENCHMARKS.md).

---

## At a Glance

| Area | Details |
|------|---------|
| Build | `python3 build.py` |
| Release stress tests | `python3 build.py --config Release --stress-test` |
| Benchmarks | [`docs/BENCHMARKS.md`](docs/BENCHMARKS.md) |
| Allocator | Palloc-backed treap nodes; system allocator elsewhere |
| Tests | 29 test cases, 199 assertions |

---

## Usage

### Open a file

```bash
./build/Release/minieditor /path/to/file.txt
```

### Key bindings

| Key | Action |
|-----|--------|
| Arrow keys | Move the cursor |
| Backspace | Delete the character before the cursor |
| Enter | Insert a new line |
| `]` | Save the current file |
| `[` | Quit the editor |
| Typing | Insert ASCII characters (32-126) |

---

## Performance

Headline numbers and methodology are in [`docs/BENCHMARKS.md`](docs/BENCHMARKS.md). The benchmark suite is captured with Palloc enabled, even though this workload runs slower with Palloc in the treap path; I kept it anyway so real use can surface allocator issues that design-time reasoning misses.

---

## Engineering Decisions

**1. Piece table + implicit treap.**
MiniEditor stores the file as original and add buffers, then uses an implicit treap to keep insert, delete, and line lookup operations fast without rebuilding the whole document.

**2. Batched insertions.**
Typed input is buffered up to 512 bytes before flushing into the piece table, which reduces tree updates for normal typing.

**3. Newline-aware metadata and caching.**
Each piece tracks newline counts, subtree metadata powers line-based navigation, and `piece_table` caches reconstructed text so repeated reads do not rebuild the document every time.

**4. Palloc only for treap nodes.**
Known-size treap nodes use Palloc, while higher-level containers and strings stay on the system allocator. This workload is still slower with Palloc, but it remains in the tree for allocator exploration rather than raw throughput.

---

## Getting Started

### Requirements

- C++20 compiler
- CMake 3.10+
- Python 3
- Ninja or Make

### Build

```bash
python3 build.py
python3 build.py --config Release --stress-test
```

### Repository Layout

| Path | Purpose |
|------|---------|
| `include/` | Public headers |
| `src/` | Implementation files |
| `tests/` | Unit tests |
| `stress_tests/` | Performance benchmarks |
| `docs/` | Benchmark and profiling docs |
| `flamegraphs/` | Generated flamegraphs |

---

## Author

Built by Altamash Aslam.
