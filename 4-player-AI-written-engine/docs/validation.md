# Validation

Tested on Linux x86-64, CPython 3.12.14, GCC/libstdc++ 13.3.0,
CMake 4.4.3, Ninja 1.13.2, pybind11 3.0.1,
scikit-build-core 0.11.6 and pytest 9.1.1.

- Standalone CMake Release build, without enabling Python: passed.
- Native CTest suite: passed, including 150 randomized complete matches.
- Isolated PEP 517 package build and installation: passed using `CXX=g++`.
- Python tests: 14 passed, including ten full-match action traces compared with
  the standalone native executable, plus ten parameterized invariant checks.
- AddressSanitizer and UndefinedBehaviorSanitizer Debug suite: passed with
  `ASAN_OPTIONS=detect_leaks=0`.
- LeakSanitizer could not execute in this container: failure inspecting `/proc`
  task information. Leak checking is **not verified**. The default sanitizer
  command is retained in README for hosts that support it.
- Native and Python random demonstrations: passed, with matching round outcomes.

The first isolated package attempt inherited a stale `CXX=clang++ -pthread`
setting pointing to an unavailable compiler. Selecting the installed GCC resolved
it; this was not a pybind11/scikit-build-core version incompatibility. Those exact
pinned dependencies were then successfully built together. No compatibility claim
is made for Windows, macOS, other Python releases or alternative dependency pins.
Packaging follows the official [scikit-build-core guide](https://scikit-build-core.readthedocs.io/en/latest/guide/getting_started.html)
and [pybind11 CMake instructions](https://pybind11.readthedocs.io/en/stable/compiling.html).

## Coverage map

| Requirement | Checks |
|---|---|
| Complete unique deck and deal | Ordered fixture, exact Hâkem hand/batch positions, all hand sizes, duplicate rejection, conservation |
| Initial selection | 100 seeded selections; first/second ace recipients, canonical partner mapping, fresh shuffled deal |
| Trump information boundary | Five cards for Hâkem, zero for others, card play rejected before trump |
| Following suit / optional trump | Rules mask for void hand contains trump and nontrump; engine rejects off-suit while holding led suit |
| Trick winner | Led-suit rank, irrelevant off-suit ace, one/multiple trumps |
| Turn order / leader | Scripted seven-trick rounds with every actor checked; winner leads next |
| Ordinary / kot / Hâkem-koti | Direct scoring checks; scripted 7–1, 7–0 wins for each team |
| Immediate stop | Exactly 28 played cards and six cards remaining per player after a sweep; no more actions |
| Hâkem / match lifecycle | Stays/advances, score overshoot to eight, terminal action/advance rejection |
| Invalid atomicity | Malformed/wrong-phase/card legality/fixture/seat errors preserve observations; clone replay also checks subsequent behavior |
| Conservation | Every randomized step: disjoint hands, undealt cards and played history total 52; trick is history suffix |
| Replay / clones | Explicit deal/action replay, independent mutation, equal future shuffled decks after cloning |
| Python agreement | Same native/Python full-match action trace, step count, round count, point checksum |
| Observation isolation | Two valid deals differ only in hidden assignments; identical observer snapshots/masks before and after trump; copied lists and debug snapshots cannot mutate engine |

Tests throw/check explicitly; they do not rely on C++ `assert`, so Release builds
still run every check. Random players are reproducible test drivers. They provide
broad transition coverage, not a proof of correctness or a strategic baseline.
