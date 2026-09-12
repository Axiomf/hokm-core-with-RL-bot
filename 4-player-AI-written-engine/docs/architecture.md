# Architecture and API

## Responsibilities

| Location | Responsibility |
|---|---|
| `cpp/include/hokm/types.hpp` | Cards, tagged actions, phases, public and privileged value types |
| `cpp/include/hokm/rules.hpp`, `cpp/src/rules.cpp` | Follow-suit legality, trick winner, official scoring, action encoding |
| `cpp/include/hokm/engine.hpp`, `cpp/src/engine.cpp` | Owned RNG, dealing, transitions, lifecycle, observation snapshots, fixture validation |
| `cpp/bindings/module.cpp` | Thin pybind11 conversion and method exposure |
| `python/hokm/__init__.py` | Public Python exports; no duplicated rules |
| `tests/` | Native checks, Python checks, cross-interface replay |
| `scripts/` | Random test drivers, demo, reproducible benchmark |

Hands are 52-bit sets in `uint64_t`. State uses fixed arrays: at most 52 public
plays and four cards in the current trick. A transition allocates no heap memory;
legal-action vectors and observations do allocate. Engine copies are value copies,
including the `mt19937_64` RNG. No global RNG, shared mutable state, Python callbacks,
or I/O occurs in the core. There is no reward function. Points are official scores.

## Rules and explicit conventions

Source: [Jahanshiri, four-player Hokm](https://www.jahanshiri.ir/cardgames/en/hokm).
The user's explicit five-card pause is authoritative: no other player receives
cards before trump selection. The reference's physical cutting step is absorbed
into the uniform shuffle; there is no cut action. No other house rules are added.

Seats increase anticlockwise: 0, 1, 2, 3, 0. Team = seat modulo 2.
Dealer = `(hakem + 3) % 4`, the seat immediately before Hâkem in dealing order.

On normal reset, shuffle a selection deck, expose cards anticlockwise beginning
with provisional participant 0, mark the first ace recipient Hâkem, and skip that
recipient thereafter until a different participant receives an ace. Rotate Hâkem
to canonical seat 0 and swap the second ace recipient into seat 2 if needed.
`debug_state().initial_seating[seat]` maps canonical seats to original participant
labels. The other participants form the other partnership. Provisional seat 0 is
an arbitrary fixed starting point because the reference does not specify one.
Collect all selection cards and shuffle a fresh complete deck for actual dealing.
An explicit initial Hâkem (0–3) skips selection and reseating entirely; it preserves
the caller's fixed seats and teams. Later rounds never reselect partnerships.

Deal batches: Hâkem five; pause; next three seats five each; four each beginning
with Hâkem; another four each beginning with Hâkem. Hâkem leads. Follow suit if
possible; otherwise any held card is legal, including either trump or nontrump.
Highest trump wins, else highest led-suit card. Trick winner leads next.

Stop immediately at seven tricks. Score 1 point, except a 7–0 win scores 2 for
Hâkem's team or 3 for the other team. Stop the match at >=7 points; overshoot is
retained. Hâkem stays after their team's win, otherwise advances one seat
anticlockwise. There are no bonuses or decisions after round termination.

## Cards and actions

Suits: clubs=0, diamonds=1, hearts=2, spades=3. Card ID is
`13 * suit + rank_offset`; offsets 0–12 mean 2,3,...,10,J,Q,K,A.
`card_rank()` returns the offset, not the printed rank.

| Integer ID | Typed C++ / Python action |
|---|---|
| 0–51 | `PlayCard(card_id)` |
| 52–55 | `ChooseTrump(Suit(id - 52))` |

C++ uses `std::variant<ChooseTrump, PlayCard>`. `encode` / `decode` validate the
stable encoding. Legal actions are sorted by ID. Masks are 56 booleans indexed by
ID. An observation mask is all false unless the observer is the current actor.
Engine masks refer to the current actor. An action has no claimed player field:
`apply` always acts for `current_actor`; callers route that seat's decision.

## Lifecycle

| Phase | Actor | Legal actions | Exit |
|---|---|---|---|
| `CHOOSE_TRUMP` | Hâkem | Four suits | `apply` completes deal, enters PLAY |
| `PLAY` | Seat to play | Legal held cards | Fourth card resolves trick; seventh trick win ends round |
| `ROUND_OVER` | -1 | None | Explicit `start_next_round()` |
| `MATCH_OVER` | -1 | None | Explicit `reset()` or privileged `load_deal()` |

`round_result` has winner=-1 until available, then winner team, points, trick totals,
and the completed round's Hâkem. `match_winner` is -1 until terminal. Scores are
updated before either completed phase is exposed. The terminal round uses
MATCH_OVER directly, with its round result still available. Calling next-round
from any other phase throws. Starting the next round clears hands, play history,
tricks and round result, retains match score, advances Hâkem if required, shuffles,
and pauses after five cards. History covers the current/just-completed round;
experiment runners should retain snapshots for earlier rounds if needed.

Illegal card, revoke, malformed action, wrong-phase action, invalid seat, and
invalid fixture inputs throw `std::invalid_argument` (`ValueError` in Python).
Validation precedes state/RNG mutation. Terminal legal actions are empty.
No automatic transition erases a result. `check_invariants` throws `logic_error`
if an internal invariant fails; it is an explicit test/debug operation, not part
of the simulation hot path.

## Engine API

| C++ | Python | Meaning |
|---|---|---|
| `Engine(seed, initial_hakem=-1)` | same | Reset on construction |
| `reset(seed, initial_hakem=-1)` | same | New match; -1 selects by aces |
| `phase()`, `current_actor()` | `.phase`, `.current_actor` | Current lifecycle and actor |
| `legal_actions()` / `legal_action_mask()` | same | Integer actions / mask |
| `apply(Action)` / `apply_id(int)` | same | Checked transition |
| `observe(seat)` | same | Independent player snapshot |
| `round_result()` / `match_winner()` | `.round_result` / `.match_winner` | Outcomes |
| `start_next_round()` | same | Explicit nonterminal round advancement |
| `clone()` | same | Independent full engine/RNG copy |
| `load_deal(deck, hakem, score={0,0}, future_seed=0)` | same | Privileged explicit starting fixture |
| `debug_state()` / `check_invariants()` | same | Privileged copied state / validation |

C++ headers document parameter types. Python arguments accept standard integer
lists for the 52-card deck and two-element score array. A valid fixture is a
permutation of all 52 cards in dealing order, a Hâkem in 0–3, and scores in 0–6.
It replaces the match context and begins at CHOOSE_TRUMP with five dealt cards.
Use actions to reach any desired intermediate state; arbitrary inconsistent
mid-round state injection is intentionally unsupported. Full-deal replay is exact
through that round. For an entire match independent of the engine's shuffling,
record each round's deal plus Hâkem, pre-round scores, and action sequence, and
load each round fixture explicitly. `future_seed` controls subsequent shuffles,
but is never player metadata.

## Information boundaries

An observation contains only seat/team, own cards, Hâkem/dealer, selected trump
(-1 before selection), phase, actor, trick leader, ordered current trick,
ordered public history, public hand sizes, trick counts, match scores and outcomes.
History includes the current trick as its suffix. Completed tricks are consecutive
groups of four; their winning seats can be recovered from public cards and trump.
After a trick resolves, `trick` is empty and `leader` is that trick's winner, even
if the round has ended. No selected trump is inferred or supplied beforehand.

Observation has no seed, other hands, undealt deck, RNG, privileged pointer, or
reference to engine memory. Python fields are read-only; converted lists are
copies. C++ snapshots can be edited without affecting the engine. Hidden deck
storage in `DebugState` includes the original full deck; only indices `cursor..51`
represent currently undealt cards. Hands plus undealt cards plus public history
partition the deck. Do not count the current trick a second time.

**Pass observations, not engines, to your agents.** Debug APIs intentionally expose
all cards for tests and experiments. This separation prevents accidental leaks;
it is not a security sandbox against code given access to the simulator object.

## Reproducibility and limitations

Seed replay is guaranteed only for the same build/toolchain and standard library:
`std::shuffle` is not specified to use an identical algorithm across implementations.
Explicit ordered-deal/action replay does not depend on shuffle and is portable.
Clones preserve all RNG state, including future round shuffles.

No batching, threading, GIL release, agent policies beyond random test drivers,
search, inferred features, learning algorithms, or reward choices. Thread safety
requires callers not to mutate the same engine concurrently. C++17 is the target;
Linux CPython 3.12 is tested here. Windows/MSVC and other Python versions require
local validation. Dependency pins describe the tested combination, not a promise
about untested platforms or future dependency releases.
