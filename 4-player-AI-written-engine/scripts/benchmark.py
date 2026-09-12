"""Sequential benchmark runs; JSON output includes raw timings and machine details."""
import argparse
import importlib.metadata
import json
import platform
from pathlib import Path
import statistics
import subprocess
import sys
from simulate import run

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--native", default="build/hokm_sim")
    p.add_argument("--matches", type=int, default=1000)
    p.add_argument("--seed", type=int, default=123)
    p.add_argument("--repeats", type=int, default=3)
    p.add_argument("--output", default="docs/benchmark-results.json")
    args = p.parse_args()
    if args.matches < 1 or args.repeats < 1:
        p.error("matches and repeats must be positive")
    native = str(Path(args.native).resolve())
    cpu = platform.processor()
    if Path("/proc/cpuinfo").exists():
        cpu = next(line.split(":",1)[1].strip() for line in Path("/proc/cpuinfo").read_text().splitlines() if line.startswith("model name"))
    cache = Path(native).parent / "CMakeCache.txt"
    cmake_settings = [line for line in cache.read_text().splitlines() if line.startswith(("CMAKE_BUILD_TYPE:","CMAKE_CXX_COMPILER:","CMAKE_CXX_FLAGS:","CMAKE_CXX_FLAGS_RELEASE:"))] if cache.exists() else []
    data = {"machine": platform.platform(), "cpu": cpu, "python": sys.version,
            "cmake_settings": cmake_settings,
            "dependencies": {name: importlib.metadata.version(name) for name in ["hokm-engine","pybind11","scikit-build-core"]},
            "seed": args.seed, "matches": args.matches, "repeats": args.repeats,
            "driver_seed": "0x12345678", "runs": {}, "summary": {}}
    # Warm up each execution path; not included in reported timings.
    subprocess.check_output([native,"10",str(args.seed),"plain"],text=True)
    run(10,args.seed,"observe")
    for interface in ("native","python"):
        for mode in ("plain","observe"):
            key = f"{interface}_{mode}"
            runs = []
            for _ in range(args.repeats):
                if interface == "native":
                    r = json.loads(subprocess.check_output([native,str(args.matches),str(args.seed),mode],text=True))
                else:
                    r, _ = run(args.matches,args.seed,mode)
                runs.append(r)
            data["runs"][key] = runs
            median = statistics.median(r["seconds"] for r in runs)
            data["summary"][key] = dict(seconds=median, steps=runs[0]["steps"],rounds=runs[0]["rounds"],
                                         steps_per_second=runs[0]["steps"]/median,rounds_per_second=runs[0]["rounds"]/median)
    for mode in ("plain","observe"):
        a=data["runs"][f"native_{mode}"][0]; b=data["runs"][f"python_{mode}"][0]
        for key in ("steps","rounds","checksum"):
            if a[key] != b[key]:
                raise RuntimeError(f"Native/Python workload mismatch: {key}")
    data["observation_increment_us_per_step"] = {}
    for interface in ("native","python"):
        a=data["summary"][f"{interface}_plain"]; b=data["summary"][f"{interface}_observe"]
        data["observation_increment_us_per_step"][interface] = (b["seconds"]-a["seconds"])/a["steps"]*1e6
    Path(args.output).write_text(json.dumps(data,indent=2)+"\n")
    print(json.dumps({"summary":data["summary"],"observation_increment_us_per_step":data["observation_increment_us_per_step"]},indent=2))

if __name__ == "__main__":
    main()
