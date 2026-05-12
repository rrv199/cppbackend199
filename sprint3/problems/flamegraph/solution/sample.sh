#!/bin/bash

# Poor Man's Profiler
# Usage: ./sample.sh PID [iterations] [sleep_seconds]

if [ $# -lt 1 ]; then
    echo "Usage: $0 <PID> [iterations] [sleep_seconds]"
    exit 1
fi

PID=$1
ITERATIONS=${2:-100}
SLEEP=${3:-0.1}
OUTPUT=${4:-stacks.txt}

echo "Sampling process $PID for $ITERATIONS iterations (sleep $SLEEP sec)"
echo "Writing to $OUTPUT"

# Clear output file
> $OUTPUT

for i in $(seq 1 $ITERATIONS); do
    echo "Sample $i" >> $OUTPUT
    # Attach with gdb, get backtrace, detach
    gdb -batch -p $PID \
        -ex "thread apply all bt" \
        -ex "detach" \
        -ex "quit" >> $OUTPUT 2>&1
    sleep $SLEEP
    echo "" >> $OUTPUT
done

echo "Done. Collected $(wc -l < $OUTPUT) lines"
