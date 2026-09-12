#include "hokm/engine.hpp"
#include <chrono>
#include <iostream>
#include <string>
using namespace hokm;
// Same small test-driver PRNG in C++/Python: identical workload, separate from engine.
struct Driver {
    std::uint32_t x;
    std::uint32_t next() {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return x;
    }
    int index(int n) {
        std::uint32_t r, threshold = std::uint32_t(-std::uint32_t(n)) % n;
        do {
            r = next();
        } while (r < threshold);
        return r % n;
    }
};
int main(int argc, char **argv) {
    try {
        int matches = argc > 1 ? std::stoi(argv[1]) : 1000;
        std::uint64_t seed = argc > 2 ? std::stoull(argv[2]) : 123;
        std::string mode = argc > 3 ? argv[3] : "plain";
        if (matches <= 0 ||
            (mode != "plain" && mode != "observe" && mode != "trace" && mode != "demo"))
            throw std::invalid_argument("usage: hokm_sim MATCHES SEED [plain|observe|trace|demo]");
        bool obs = mode == "observe", trace = mode == "trace", demo = mode == "demo";
        Driver driver{0x12345678};
        std::uint64_t steps = 0, rounds = 0, checksum = 0;
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < matches; ++i) {
            Engine e(seed + i);
            while (e.phase() != Phase::MatchOver) {
                if (e.phase() == Phase::RoundOver) {
                    e.start_next_round();
                    continue;
                }
                if (obs) {
                    auto o = e.observe(e.current_actor());
                    checksum += o.hand.size() + o.history.size();
                }
                auto a = e.legal_actions();
                int id = a[driver.index(static_cast<int>(a.size()))];
                e.apply_id(id);
                ++steps;
                if (trace)
                    std::cout << id << ' ';
                if (e.phase() == Phase::RoundOver || e.phase() == Phase::MatchOver) {
                    ++rounds;
                    auto r = e.round_result();
                    checksum += r.points;
                    if (demo)
                        std::cout << "Round " << rounds << ": team " << r.winner << ", points "
                                  << r.points << '\n';
                }
            }
        }
        double seconds =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        if (trace)
            std::cout << '\n';
        std::cout << "{\"mode\":\"" << mode << "\",\"seed\":" << seed << ",\"matches\":" << matches
                  << ",\"steps\":" << steps << ",\"rounds\":" << rounds
                  << ",\"seconds\":" << seconds << ",\"steps_per_second\":" << steps / seconds
                  << ",\"rounds_per_second\":" << rounds / seconds << ",\"checksum\":" << checksum
                  << "}\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
