#!/bin/bash -e
echo "building..."
make fib.bin >/dev/null 2>&1
make fib.host.bin fib.host_o3.bin >/dev/null 2>&1
echo "running fib.bin (by compilium)..."
( time ./fib.bin ) 2>&1 | grep real
echo "running fib.host.bin (by host compiler)..."
( time ./fib.host.bin ) 2>&1 | grep real
echo "running fib.host_o3.bin (by host compiler with -O3)..."
( time ./fib.host_o3.bin ) 2>&1 | grep real
