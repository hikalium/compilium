#!/bin/bash -e
echo "building..."
make constsum.bin >/dev/null 2>&1
make constsum.host.bin constsum.host_o3.bin >/dev/null 2>&1
echo "running constsum.bin (by compilium)..."
( time ./constsum.bin ) 2>&1 | grep real
echo "running constsum.host.bin (by host compiler)..."
( time ./constsum.host.bin ) 2>&1 | grep real
echo "running constsum.host_o3.bin (by host compiler with -O3)..."
( time ./constsum.host_o3.bin ) 2>&1 | grep real
