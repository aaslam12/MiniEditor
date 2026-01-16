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
*   **Intuitive Navigation:** Full cursor support with 1-indexed positioning for consistent line/column reporting
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

The following results are obtained on a standard Linux machine using Release build optimization:

```bash
python3 build.py --config Release --stress-test
```

#### Sequential Operations
| Operation | Performance |
| :--- | :--- |
| Insert 100k characters sequentially | ~0.09 µs per insert |
| Delete 100k characters sequentially | ~0.09 µs per delete |

#### Random Operations (10MB buffer)
| Metric | Result |
| :--- | :--- |
| Total operations | 500,000 random insertions and deletions |
| Average time per edit | ~1.54 µs |
| Index lookup / search time | ~0.007 ms |

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
