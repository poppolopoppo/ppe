#!/bin/sh

go tool pprof -raw -output=cpu.txt cpu.pprof && \
    ../FlameGraph/stackcollapse-go.pl cpu.txt | ../FlameGraph/flamegraph.pl > cpu.svg
