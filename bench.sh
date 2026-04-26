#!/usr/bin/env bash
set -e

# Try to set CPU governor to performance (requires sudo, might fail if no permissions, that's OK)
sudo cpupower frequency-set -g performance >/dev/null 2>&1 || true

OUT="bench_$(date +%Y%m%d_%H%M%S).log"
echo "Starting benchmark... writing to $OUT"

{
    echo "## Environment"
    uname -srm
    grep "model name" /proc/cpuinfo | head -1
    grep MemTotal /proc/meminfo
    echo
    echo "## Build Info"
    cmake --build build -- VERBOSE=1 2>&1 | grep -m1 -- '-O3' || echo "WARNING: no -O3 flag found"
    echo
    echo "## Benchmark Results"
    # Run the performance test
    taskset -c 2 ./build/bin/run_tests --gtest_filter='PerformanceTest.*'
} | tee "$OUT"

echo "Benchmark complete!"
