"""Random legal demo / benchmark. No game rules live here."""
import argparse
import json
import time
from hokm import Engine, Phase

class Driver:
    def __init__(self):
        self.x = 0x12345678
    def next(self):
        x = self.x
        x ^= (x << 13) & 0xffffffff
        x ^= x >> 17
        x ^= (x << 5) & 0xffffffff
        self.x = x
        return x
    def index(self, n):
        threshold = ((-n) & 0xffffffff) % n
        while True:
            r = self.next()
            if r >= threshold:
                return r % n

def run(matches=1000, seed=123, mode="plain"):
    driver = Driver()
    steps = rounds = checksum = 0
    trace = []
    start = time.perf_counter()
    for i in range(matches):
        e = Engine(seed + i)
        while e.phase != Phase.MATCH_OVER:
            if e.phase == Phase.ROUND_OVER:
                e.start_next_round()
                continue
            if mode == "observe":
                o = e.observe(e.current_actor)
                checksum += len(o.hand) + len(o.history)
            actions = e.legal_actions()
            action = actions[driver.index(len(actions))]
            e.apply_id(action)
            steps += 1
            if mode == "trace":
                trace.append(action)
            if e.phase in (Phase.ROUND_OVER, Phase.MATCH_OVER):
                rounds += 1
                checksum += e.round_result.points
                if mode == "demo":
                    print(f"Round {rounds}: team {e.round_result.winner}, points {e.round_result.points}")
    seconds = time.perf_counter() - start
    result = dict(mode=mode, seed=seed, matches=matches, steps=steps, rounds=rounds,
                  seconds=seconds, steps_per_second=steps/seconds,
                  rounds_per_second=rounds/seconds, checksum=checksum)
    return result, trace

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--matches", type=int, default=1000)
    p.add_argument("--seed", type=int, default=123)
    p.add_argument("--mode", choices=["plain", "observe", "trace", "demo"], default="plain")
    args = p.parse_args()
    if args.matches <= 0:
        p.error("matches must be positive")
    result, trace = run(args.matches, args.seed, args.mode)
    if args.mode == "trace":
        print(*trace)
    print(json.dumps(result))
