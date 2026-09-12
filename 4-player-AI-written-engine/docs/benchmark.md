# Measured benchmark

Run on 2026-09-12. These are measurements in this hosted container, not estimates
for the user's laptop. Raw runs are in [benchmark-results.json](benchmark-results.json).

- CPU: Intel Xeon Platinum 8573C, x86-64 KVM virtual machine, nine visible CPUs.
  Each simulation runs on one thread; no batching or multithreading.
- Linux, CPython 3.12.14, GCC/libstdc++ 13.3.0, CMake 4.4.3.
- Native core: Release, `-O3 -DNDEBUG`, C++17, no CPU-specific flags.
- Python extension: scikit-build-core 0.11.6 + pybind11 3.0.1, Release,
  normal pybind11 module optimization defaults; same GCC and core sources.
- Workload: 5,000 complete matches per run, seed 123 (match i uses 123+i),
  independent random-driver state 0x12345678. Three repeats per mode.
- Every run: **2,406,127 decision steps; 53,567 completed rounds**.
  A decision is either trump selection or playing one card.
- Table uses median elapsed time and derives throughput from it. Ranges show
  all three runs; no concurrent build/test jobs were run in the final measurement.
  This is a shared virtual host, without CPU affinity or isolation guarantees.

| Workload | Median seconds | Decisions/s | Rounds/s | Elapsed range (s) |
|---|---:|---:|---:|---:|
| Native C++, plain | 0.559953 | 4,297,016 | 95,663 | 0.551950–0.578319 |
| Native C++, with observation | 1.255260 | 1,916,836 | 42,674 | 1.221060–1.316190 |
| Python loop + bindings, plain | 16.692341 | 144,146 | 3,209 | 13.029207–19.616133 |
| Python loop + bindings, with observation | 36.486698 | 65,945 | 1,468 | 33.734805–43.743270 |

The incremental measured cost per decision when enabling observations was
**0.289 µs native** and
**8.227 µs through Python**.
These are differences of whole-loop medians, not isolated function microbenchmarks.

## Timing boundaries

Both plain modes include engine construction/reset, ace selection, all shuffles,
dealing, legal-action list construction, random legal choice, checked action
application, phase checks, scoring, round advancement and outcome bookkeeping.
They exclude process startup, imports, warm-up and final JSON/file output. Engine
and driver seeds are identical; the runner verifies matching step/round counts
and point checksums between native and Python. A separate correctness test compares
entire action traces. The test-driver PRNG is shared in design across languages;
it is not a strategic policy or part of the engine RNG.

Observation modes additionally construct the current actor's complete observation
at every decision and consume hand/history lengths in a checksum. Python also
wraps the returned value and converts accessed hand/history vectors to Python
lists. Fields not accessed by Python remain in the owned C++ snapshot until read.
No invariant checker, logging, file I/O, Python callback or policy inference runs
inside the measured core simulation. Demo and trace modes are not benchmarks.

The Python difference includes interpreter loops, random-driver operations,
property access and list conversions, not only the binding call overhead. These
results do not establish a general C++ speedup, predict learning throughput, or
justify adding batching yet. Rerun on the intended machine and workload first.

## Reproduce

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
python -m pip install .
python -m pip install pybind11==3.0.1 scikit-build-core==0.11.6
python scripts/benchmark.py --matches 5000 --seed 123 --repeats 3
```

Use `--native build/Release/hokm_sim.exe` for a multi-configuration Windows build.
Build native and Python artifacts with the same toolchain to retain seed-based
workload agreement. Original per-run settings are retained in the raw JSON.
