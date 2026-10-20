# DuplicateFinder

A Qt6/C++17 desktop utility that scans a directory tree and finds files with identical content using a three-stage hashing pipeline running on a thread pool.

> **Note:** Build and runtime have been **tested on Linux only**.

---

## Features

- Recursive directory scan with symlink skipping
- Three-stage pipeline to minimize disk I/O:
  1. **Group by file size** — files with unique sizes are filtered out immediately.
  2. **Mini-hash** the first 4 KiB of each candidate with SHA-256.
  3. **Full-hash** only files whose mini-hash matched.
- Parallel hashing via `QThreadPool` (`HashWorker` uses `QRunnable`)
- Live progress reporting driven by a mutex-guarded pending-task counter
- Cancellable at any stage
- Tree view of duplicate groups with per-group wasted-space column
- Human-readable size formatting

---

## Requirements

- CMake ≥ 3.20
- C++17 compiler (GCC ≥ 9, Clang ≥ 10)
- Qt 6 (Core, Gui, Widgets, Concurrent)

---
## TODO

- Add a sorting files by size, name.
- Add a "Delete selected duplicates" action with confirmation.
- Verify the Windows build.
- Add drag & drop
