# Hokm engine

A standalone C++17 four-player, two-versus-two Hokm simulator, with thin Python
bindings. Mechanics only: no learning, search, inference, or strategic players.

## Build and install

Run commands from this directory. Requires a C++17 compiler. On Windows, use
WSL2 with GCC, or install Visual Studio C++ build tools for native builds.

```sh
python -m venv .venv
# Linux / WSL:
source .venv/bin/activate
# PowerShell alternative: .venv\Scripts\Activate.ps1
python -m pip install cmake ninja
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
python -m pip install ".[test]"
python -m pytest -q
```

The native build does not find or require Python or pybind11. With a system CMake,
the three `cmake`/`ctest` commands alone suffice. Python installation uses an
isolated scikit-build-core build with pinned pybind11. A normal non-editable
installation must be reinstalled after changes to C++ source.

For MSVC multi-configuration builds the tool path is `build/Release/hokm_sim.exe`.
Set `HOKM_NATIVE_SIM` to that path before pytest; supply it with `--native` to the
benchmark. For a stale compiler environment, select a valid compiler before pip,
for example `CXX=g++ python -m pip install ".[test]"` on Linux. Do not use that
GCC setting with MSVC.

## Demo, tests and benchmark

```sh
./build/hokm_sim 1 123 demo
python scripts/simulate.py --matches 1 --seed 123 --mode demo
ctest --test-dir build --output-on-failure
python -m pytest -q

./build/hokm_sim 1000 123 plain
./build/hokm_sim 1000 123 observe
python scripts/simulate.py --matches 1000 --seed 123 --mode plain
python scripts/simulate.py --matches 1000 --seed 123 --mode observe

# Installs metadata dependencies for the benchmark report (isolated pip builds
# do not install their build dependencies into your active environment).
python -m pip install pybind11==3.0.1 scikit-build-core==0.11.6
python scripts/benchmark.py --matches 1000 --seed 123 --repeats 3
```

[Measured results](docs/benchmark.md) explain workload, build settings, timing
boundaries and limitations. The benchmark creates `docs/benchmark-results.json`.
Its seeded test driver is deliberately the same in C++ and Python, enabling exact
cross-interface action-trace comparison. Demo/trace printing is disabled during
benchmark runs. Do not compare Debug timings with Release results.

Optional Linux memory/undefined-behavior check:

```sh
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
cmake --build build-sanitize --parallel 2
ctest --test-dir build-sanitize --output-on-failure
```

## Play a random match in Python

```python
import random
from hokm import Engine, Phase

game = Engine(seed=123)
player_rng = random.Random(456)  # independent of the engine RNG
while game.phase != Phase.MATCH_OVER:
    if game.phase == Phase.ROUND_OVER:
        print("Round winner:", game.round_result.winner)
        game.start_next_round()
        continue
    observation = game.observe(game.current_actor)
    legal = [i for i, allowed in enumerate(observation.legal_mask) if allowed]
    game.apply_id(player_rng.choice(legal))
print("Final round:", game.round_result.winner, game.round_result.points)
print("Match winner:", game.match_winner)
```

Only pass the observation to an agent. The reset seed, `Engine`, `debug_state()`
and `load_deal()` belong to the experiment controller. Observations contain no
other hands, undealt deck, RNG state or seed.

## Repository map

| Directory / file | Contents |
|---|---|
| `cpp/include/hokm/` | `types.hpp`, `rules.hpp`, `engine.hpp` |
| `cpp/src/` | `rules.cpp`, `engine.cpp` |
| `cpp/bindings/` | `module.cpp` |
| `python/hokm/` | Python exports |
| `tests/` | Native and Python correctness tests |
| `scripts/` | Native/Python random demos and benchmark runner |
| `docs/` | Architecture/API, benchmark report and raw results |
| `CMakeLists.txt`, `pyproject.toml` | Independent native build and Python packaging |

[Architecture and API](docs/architecture.md) documents action IDs, all observation
fields, fixture format, ace-selection seating, phases, scoring, cloning, and
reproducibility. [Validation](docs/validation.md) records checks and tested tools.

Main lifecycle: choose trump → play → completed round → explicit next-round call.
A completed match remains terminal until reset. Official scores are separate from
any reward you later define. History is preserved until the next round starts.
