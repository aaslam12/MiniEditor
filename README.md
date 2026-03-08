# MiniEditor

MiniEditor is a high-performance, cross-platform, terminal-based text editor built from scratch in C++20. It is designed to be efficient and simple while handling large files with ease. The editor features a responsive TUI (Text User Interface) backed by advanced data structures (Piece Table with Implicit Treap) that ensure operations remain fast regardless of file size.

## Project Overview

*   **Language:** C++20
*   **Build System:** CMake (wrapped in a Python script)
*   **Key Data Structures:** Piece Table, Implicit Treap
*   **Libraries:** PDCurses (for TUI), Catch2 (for Unit Testing)

## Key Features

*   **High Performance:** Optimized O(log n) operations using an Implicit Treap-backed Piece Table
*   **Large File Support:** Efficiently handle files of any size without memory bloat
*   **Responsive Editing:** Batched insertions (up to 512 bytes) with real-time display during batching
*   **Intuitive Navigation:** Full cursor support with horizontal/vertical scrolling for long lines
*   **Proper Viewport Management:** Automatic scrolling keeps cursor visible with efficient rendering
*   **Command-line Integration:** Open files directly from the command line
*   **Persistent Storage:** Save changes with visual confirmation in the status bar
*   **Clean TUI:** Dark terminal-friendly interface with line numbers and status bar
*   **Comprehensive Testing:** 29 unit tests covering core functionality (199 assertions)

## Requirements

To build MiniEditor, you need the following:

*   **Compiler:** A C++ compiler supporting C++20
*   **Build Tools:**
    *   CMake (3.10 or newer)
    *   Python 3 (for the build script)
    *   Ninja (recommended) or Make

## Build

The project includes a `build.py` helper script to streamline the build process.

Builds the project in Debug mode and runs unit tests.
```bash
python build.py
```
*Note: The executable will be located at `build/Debug/minieditor`*

### Build Options

| Flag | Description |
| :--- | :--- |
| `--config <type>` | Set build configuration: `Debug` (default) or `Release`. |
| `--clean` | Removes the `build` directory to start fresh. |
| `--build-only` | Only compiles the project and skips running tests or the executable. |
| `--no-tests` | Skips building and running unit tests (default in Release mode). |
| `--stress-test` | Builds and runs the performance stress tests. Should be paired with the Release configuration |
| `--static` | Links standard libraries (`libgcc`, `libstdc++`) statically. Useful for portability. |

## Usage

### Opening the Editor

```bash
# Open an existing file
./build/Release/minieditor /path/to/file.txt

# Start with an empty buffer (requires specifying a file path to save later)
./build/Release/minieditor
```

### Key Bindings

| Key | Action |
| :--- | :--- |
| **Arrow Keys** | Move cursor (Up, Down, Left, Right) |
| **Backspace** | Remove character before cursor |
| **Enter** | Insert a new line |
| **`]`** | Save the current file |
| **`[`** | Quit the editor |
| **Typing** | Insert characters at cursor position (ASCII 32-126) |

### Status Bar Information

The status bar at the bottom displays:
*   **Filename** - Name of the currently open file
*   **Position** - Current cursor line and column (1-indexed)
*   **Dirty Indicator** - `[modified]` when file has unsaved changes
*   **Status Message** - Temporary messages like "File saved!" or error messages

## Architecture & Implementation


### Benchmark Results

All results are from a Release build (`-O3`) on a standard Linux x86-64 machine.

```bash
python3 build.py --config Release --stress-test
```

#### Front Insertions & Deletions
Stress-tests the hardest case for most editors: repeated edits at position 0.

| Operation | Count | Total Time | Avg per op |
| :--- | ---: | ---: | ---: |
| Insert at front | 100,000 | 9.56 ms | **0.096 µs** |
| Delete from front | 100,000 | 9.09 ms | **0.091 µs** |

#### Alternating Insert / Delete
Rapid alternation between inserts and deletes at random positions.

| Metric | Result |
| :--- | ---: |
| Cycles | 50,000 |
| Total time | 31.7 ms |
| **Avg per cycle** | **0.63 µs** |
| Full string rebuild | 0.008 ms |

#### Random Edits (10 MB buffer)
500k random insertions and deletions across a 10 million character buffer.

| Metric | Result |
| :--- | ---: |
| Operations | 500,000 |
| Total time | 674.7 ms |
| **Avg per edit** | **1.35 µs** |
| Full reconstruction | 33.4 ms |

#### Tree Insertion Throughput (10M pieces)
Measures raw piece insertion speed building a tree of 10 million nodes.

| Metric | Result |
| :--- | ---: |
| Total pieces inserted | 10,000,000 |
| Total time | 1.49 s |
| **Avg per insertion** | **0.149 µs** |
| **Throughput** | **~82 MB/s** |
| Total data | 122.9 MB |
| Peak RAM | ~986 MB |

#### Line Access — `get_line` (O(log n))
Random `get_line` reads across a table built from 10,000 individually inserted pieces.

| Metric | Result |
| :--- | ---: |
| Reads | 50,000 |
| Total time | 16.5 ms |
| **Avg per read** | **0.33 µs** |

#### Line Access — `get_line` on 100k-line file

| Metric | Result |
| :--- | ---: |
| Lines in file | 100,000 |
| Random `get_line` reads | 10,000 |
| **Avg per `get_line`** | **0.79 µs** |
| Random `get_index_for_line` lookups | 1,000 |
| **Avg per `get_index_for_line`** | **0.50 µs** |
| Random newline inserts | 1,000 |
| Avg per newline insert | 0.65 µs |

#### `get_index_for_line` on 10M-piece tree

| Metric | Result |
| :--- | ---: |
| Pieces in tree | 10,000,000 |
| **Search time (single lookup)** | **0.005 ms** |

### Flamegraphs

Interactive SVG flamegraphs are in the [`flamegraphs/`](flamegraphs/) directory, generated with `perf record -F 999 --call-graph dwarf` on each stress test.

| Benchmark | Flamegraph |
| :--- | :--- |
| Alternating insert/delete | [stress_alternating_ops.svg](flamegraphs/stress_alternating_ops.svg) |
| Front insertions/deletions | [stress_front_ops.svg](flamegraphs/stress_front_ops.svg) |
| 10M piece tree insertion | [stress_get_index.svg](flamegraphs/stress_get_index.svg) |
| Random `get_line` (10k pieces) | [stress_get_line.svg](flamegraphs/stress_get_line.svg) |
| 100k line file access | [stress_newlines.svg](flamegraphs/stress_newlines.svg) |
| Random edits (10MB buffer) | [stress_random_edits.svg](flamegraphs/stress_random_edits.svg) |

## Testing

The project includes comprehensive unit tests:

```bash
# Run unit tests (default, included in debug build)
python3 build.py

# Run only with no tests
python3 build.py --no-tests

# Run stress tests for performance analysis
python3 build.py --config Release --stress-test
```

**Test Coverage:** 29 test cases, 199 assertions covering:
*   Editor operations (cursor movement, text insertion/deletion, file I/O)
*   Piece table functionality (insertion, deletion, line access)
*   Data structure integrity (Implicit Treap operations)

## Development Conventions

*   **Namespace:** All code is contained within the `AL` namespace
*   **Formatting:** Code follows `.clang-format` configuration for consistency
*   **Testing:** The macro `MINIEDITOR_TESTING` exposes private members for white-box testing
*   **Code Style:** Clean, readable C++ with minimal comments (only where necessary)

## Limitations (for now)

*   ASCII input only (characters 32-126)
*   No undo/redo functionality
*   No multi-file support
*   No syntax highlighting
*   No search/replace functionality
