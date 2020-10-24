#!/bin/bash -e
echo "building..."
make collatz.bin >/dev/null 2>&1
make collatz.host.bin collatz.host_o3.bin >/dev/null 2>&1
echo "running collatz.bin (by compilium)..."
( time ./collatz.bin | tee collatz.stdout.txt ) 2>&1 | grep real
echo "running collatz.host.bin (by host compiler)..."
( time ./collatz.host.bin | tee collatz.host.stdout.txt ) 2>&1 | grep real
echo "running collatz.host_o3.bin (by host compiler with -O3)..."
( time ./collatz.host_o3.bin | tee collatz.host_o3.stdout.txt ) 2>&1 | grep real

diff -u collatz.stdout.txt collatz.host.stdout.txt || { echo "stdout diff"; false; }
diff -u collatz.stdout.txt collatz.host_o3.stdout.txt || { echo "stdout diff"; false; }
echo "OK"

