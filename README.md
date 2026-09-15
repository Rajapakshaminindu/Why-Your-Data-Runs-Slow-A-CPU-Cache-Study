# ⚡ Why Your Data Runs Slow — A CPU Cache Study

> A hands-on computer architecture experiment connecting **CPU cache theory** with **real data science performance**. Written in C to benchmark memory access patterns, with results analyzed in Python using Pandas and Matplotlib.

[![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/Rajapakshaminindu/Why-Your-Data-Runs-Slow-A-CPU-Cache-Study/blob/main/analysis.ipynb)

---

## 📋 Table of Contents

- [Project Overview](#-project-overview)
- [The Core Idea](#-the-core-idea)
- [Project Structure](#-project-structure)
- [The 4 Experiments](#-the-4-experiments)
- [Benchmark Results](#-benchmark-results)
- [Performance Summary](#-performance-summary)
- [Analysis Charts](#-analysis-charts)
- [Tech Stack](#-tech-stack)
- [How to Run](#-how-to-run)
- [Real-World Connection](#-real-world-connection)
- [Key Takeaways](#-key-takeaways)
- [Author](#-author)

---

## 🔍 Project Overview

This project was built to answer a deceptively simple question:

> **Why does the same computation run at completely different speeds — on the same machine, with the same data?**

The answer lies in **CPU cache behavior**. Using concepts from a **Computer Architecture module at the University of Kelaniya (UOK)**, 4 C programs were written to benchmark different memory access patterns. The raw timing data was then analyzed in Python to visualize and quantify the performance gap.

**The headline result:**
- 🚀 Cache-friendly code: **~2 ms**
- 🐢 Cache-unfriendly code (random access): **~200 ms**
- That's a **~20x speedup** just by changing the order of memory reads — **same data, same machine, same math.**

---

## 💡 The Core Idea

Modern CPUs are **much faster than RAM**. To bridge this gap, CPUs use a small, extremely fast memory called **cache** (L1, L2, L3). When the CPU reads a value from RAM, it also pre-fetches a nearby block of data (a **cache line**, typically 64 bytes) into cache.

- If the **next value you need is already in cache** → **cache HIT** → nearly instant
- If the **next value is NOT in cache** → **cache MISS** → must wait for slow RAM

The key insight:
```
Cache-friendly code  = reads memory in order   → lots of hits  → FAST
Cache-unfriendly code = jumps randomly in memory → lots of misses → SLOW
```

This directly explains why libraries like **NumPy** and **Pandas** are built the way they are — their internal memory layout is specifically designed to maximize cache hits.

---

## 📁 Project Structure

```
Why-Your-Data-Runs-Slow-A-CPU-Cache-Study/
│
├── experiments/                  # C source files for each benchmark
│   ├── exp_a.c                   # Experiment A: Row-major matrix traversal (cache-friendly)
│   ├── exp_b.c                   # Experiment B: Column-major matrix traversal (cache-unfriendly)
│   ├── exp_c.c                   # Experiment C: Sequential 1D array access (cache-friendly)
│   ├── exp_d.c                   # Experiment D: Random 1D array access (cache-unfriendly)
│   └── timing.h                  # Shared high-resolution timing utility
│
├── data/
│   ├── results.csv               # Raw benchmark timing data (32 measurements)
│   └── sample_results.csv        # Sample data for reference
│
├── analysis.ipynb                # Python notebook: loads data, produces 4 charts
└── README.md                     # This file
```

---

## 🧪 The 4 Experiments

All experiments were run **8 times each** to produce stable, reproducible measurements. The timing utility (`timing.h`) uses `clock_gettime(CLOCK_MONOTONIC)` for nanosecond precision, compatible with Linux and WSL2.

---

### Experiment A — Row-Major Matrix Traversal ✅ Cache-Friendly

**File:** [`experiments/exp_a.c`](experiments/exp_a.c)

**Data structure:** 2048 × 2048 integer matrix = **16 MB** (larger than L2 cache)

**Access pattern:** Outer loop over rows, inner loop over columns
```c
for (int i = 0; i < ROWS; i++)       // row by row
    for (int j = 0; j < COLS; j++)
        sum += matrix[i][j];          // reads memory left-to-right
```

**Why it's fast:**
In C, 2D arrays are stored in **row-major order** — each row is one contiguous block in memory. When the CPU fetches `matrix[0][0]`, it automatically pulls the next 64 bytes (the whole cache line) into cache — including `matrix[0][1]`, `matrix[0][2]`, etc. The next ~15 reads are **free** (already in cache). Cache hit rate is very high.

**Average time: ~2.08 ms**

---

### Experiment B — Column-Major Matrix Traversal ❌ Cache-Unfriendly

**File:** [`experiments/exp_b.c`](experiments/exp_b.c)

**Data structure:** Same 2048 × 2048 matrix = **16 MB**

**Access pattern:** Outer loop over columns, inner loop over rows
```c
for (int j = 0; j < COLS; j++)       // column by column
    for (int i = 0; i < ROWS; i++)
        sum += matrix[i][j];          // jumps 8 KB between reads
```

**Why it's slow:**
Column elements are stored 2048 integers apart in memory (2048 × 4 bytes = **8 KB apart**). Each access jumps to a completely different memory location. Every cache line fetched contains exactly one useful value — the rest is wasted. **Every read is a cache miss.**

> ⚠️ **Only the loop order changed** from Experiment A. The matrix, the data, and the work are identical. Yet the speed difference is enormous.

**Average time: ~35.3 ms → 17× slower than Exp A**

---

### Experiment C — Sequential 1D Array Access ✅ Cache-Friendly

**File:** [`experiments/exp_c.c`](experiments/exp_c.c)

**Data structure:** 1D array of **16 million integers = 64 MB**

**Access pattern:** Linear scan from index 0 to end
```c
for (int i = 0; i < SIZE; i++)
    sum += arr[i];                    // perfectly sequential
```

**Why it's fast:**
This is the most cache-friendly access pattern possible. The CPU's **hardware prefetcher** detects the sequential access pattern and begins loading the next cache line into cache **before it's even requested**. Almost every access is a hit, with near-zero wait time for memory.

> 💡 This is also how **NumPy** works internally — contiguous memory blocks read in order.

**Average time: ~9.94 ms**

---

### Experiment D — Random 1D Array Access ❌ Cache-Unfriendly

**File:** [`experiments/exp_d.c`](experiments/exp_d.c)

**Data structure:** Same 1D array of **16 million integers = 64 MB** + shuffled index array

**Access pattern:** Access elements via a Fisher-Yates shuffled index array
```c
// indices[] is a random permutation of 0..SIZE-1
for (int i = 0; i < SIZE; i++)
    sum += arr[indices[i]];           // random jump in memory every time
```

**Why it's slow:**
The CPU's hardware prefetcher is completely blind — it cannot predict a random access pattern. Every read goes to a random memory location. **Every single access is a cache miss**, forcing the CPU to wait for RAM on each iteration. This is similar to what happens in Pandas when using unsorted indices or scattered `df.loc[]` access patterns.

**Average time: ~200 ms → 20× slower than Exp C**

---

## 📊 Benchmark Results

Raw data from `data/results.csv` — 8 runs per experiment:

| Experiment | Run 1 | Run 2 | Run 3 | Run 4 | Run 5 | Run 6 | Run 7 | Run 8 |
|------------|-------|-------|-------|-------|-------|-------|-------|-------|
| **A — Row-major** | 2.337 | 2.269 | 2.067 | 2.096 | 1.919 | 1.948 | 2.108 | 1.905 |
| **B — Col-major** | 34.448 | 34.350 | 38.803 | 35.850 | 34.723 | 34.275 | 35.338 | 34.672 |
| **C — Sequential** | 10.148 | 10.357 | 10.040 | 9.862 | 9.883 | 9.714 | 9.640 | 9.877 |
| **D — Random** | 208.864 | 202.438 | 197.327 | 196.660 | 197.311 | 200.575 | 198.775 | 197.993 |

*(All values in milliseconds)*

---

## 📐 Performance Summary

Summary statistics computed in Python:

| Experiment | Pattern | Mean (ms) | Std (ms) | Median (ms) | Min (ms) | Max (ms) |
|---|---|---|---|---|---|---|
| **A — Row-major** | Cache-friendly | **2.081** | 0.159 | 2.082 | 1.905 | 2.337 |
| **B — Col-major** | Cache-unfriendly | **35.307** | 1.511 | 34.698 | 34.275 | 38.803 |
| **C — Sequential** | Cache-friendly | **9.940** | 0.233 | 9.880 | 9.640 | 10.357 |
| **D — Random** | Cache-unfriendly | **199.993** | 4.069 | 198.384 | 196.660 | 208.864 |

### Speedup Ratios

| Comparison | Speedup |
|---|---|
| A (row-major) vs B (col-major) | **B is 17× slower than A** |
| C (sequential) vs D (random) | **D is 20× slower than C** |

> 🔑 Same data. Same machine. Same computation. Just a **different order of memory access**.

---

## 📈 Analysis Charts

The Python notebook (`analysis.ipynb`) produces **4 charts**:

| Chart | Description |
|---|---|
| **Chart 1** — Average time per experiment | Bar chart comparing mean execution time across all 4 experiments |
| **Chart 2** — All 8 runs per experiment | Box + strip plot showing variability across runs |
| **Chart 3** — Speedup ratios | Bar chart showing how much faster cache-friendly experiments are (17× and 20×) |
| **Chart 4** — Theory check: implied hit rate | Validates results against the Lecture 02 cache hit rate formula from Computer Architecture module |

---

## 🛠️ Tech Stack

| Tool | Purpose |
|---|---|
| **C (C99)** | Writing and running the timing experiments |
| **GCC** | Compiling the C experiments |
| **`clock_gettime(CLOCK_MONOTONIC)`** | High-precision wall-clock timing (nanosecond resolution) |
| **Python 3** | Data analysis and visualization |
| **Pandas** | Loading and summarizing `results.csv` |
| **NumPy** | Numerical calculations (speedup ratios, hit rate estimates) |
| **Matplotlib** | Producing all 4 benchmark charts |
| **Seaborn** | Box + strip plot for run variability |
| **Google Colab** | Running the analysis notebook in the cloud |
| **Linux / WSL2** | Required to compile and run the C experiments |

---

## ▶️ How to Run

### Step 1 — Compile and Run the C Experiments (Linux / WSL2)

```bash
# Clone the repo
git clone https://github.com/Rajapakshaminindu/Why-Your-Data-Runs-Slow-A-CPU-Cache-Study.git
cd Why-Your-Data-Runs-Slow-A-CPU-Cache-Study/experiments

# Compile each experiment
gcc -O0 exp_a.c -o exp_a   # -O0 disables optimizations for honest measurements
gcc -O0 exp_b.c -o exp_b
gcc -O0 exp_c.c -o exp_c
gcc -O0 exp_d.c -o exp_d

# Run all 4 and combine into results.csv
{ ./exp_a; ./exp_b | tail -n +2; ./exp_c | tail -n +2; ./exp_d | tail -n +2; } > ../data/results.csv
```

> **Note:** Use `-O0` to prevent the compiler from optimizing away the loops. With `-O2` or `-O3`, the compiler may restructure the loops automatically, masking the cache effect.

### Step 2 — Analyze Results in Python (Google Colab or Locally)

**Option A: Google Colab**
1. Click the **Open in Colab** badge at the top of this README
2. Upload `data/results.csv` to the Colab session
3. Run all cells

**Option B: Local Python**
```bash
# Install dependencies
pip install pandas numpy matplotlib seaborn

# Run the analysis notebook
jupyter notebook analysis.ipynb
```

---

## 🌍 Real-World Connection

This experiment directly explains performance behaviour in popular data science tools:

| Library / Tool | Cache-Friendly Behaviour |
|---|---|
| **NumPy** | Stores arrays in contiguous C-order memory, reads sequentially → near-zero cache misses |
| **Pandas** | Column-oriented storage means reading one column is sequential; row-wise operations (`.iterrows()`) can be slow due to scattered access |
| **Pandas `df.loc[]`** | Non-sorted / non-contiguous index access causes random memory jumps → cache misses → slow |
| **Sorting before merge/join** | Pre-sorting aligns memory access patterns → cache-friendly → faster merge |
| **NumPy `reshape()` vs `transpose()`** | `transpose()` changes logical view without moving data — subsequent access can become column-major (cache-unfriendly) |

> 💡 This is why data engineers often say: **"Sort your data before joining"** and why **columnar formats** like Parquet are faster for analytical queries — they exploit cache-friendly sequential reads.

---

## 🎯 Key Takeaways

1. **Memory access order matters as much as algorithm complexity.** Two loops with identical Big-O can differ by 20× in real runtime due to cache behaviour.

2. **Cache misses are expensive.** A cache miss forces the CPU to wait ~100–200 CPU cycles for RAM, while a cache hit takes ~4 cycles. At scale, this dominates performance.

3. **The CPU hardware prefetcher is powerful — but predictable patterns are required.** Sequential access patterns let the prefetcher work perfectly; random access makes it useless.

4. **NumPy is fast because it's cache-aware.** Its contiguous memory layout and sequential access patterns are designed specifically around CPU cache behaviour.

5. **You don't need to change your algorithm to get a speedup.** Sometimes just reordering loops or restructuring data access is enough to get a 10–20× improvement.

---

## 👤 Author

**Rajapakshaminindu**
- 📂 GitHub: [@Rajapakshaminindu](https://github.com/Rajapakshaminindu)
- 🎓 University of Kelaniya (UOK) — Computer Architecture Module

---

> 🚀 *This project bridges computer architecture theory with practical data science performance — showing that low-level hardware behaviour has a direct, measurable impact on everyday Python code.*
