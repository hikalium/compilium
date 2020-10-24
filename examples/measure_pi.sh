#!/bin/bash -e
echo "building..."
make pi.bin >/dev/null 2>&1
make pi.host.bin pi.host_o3.bin >/dev/null 2>&1
echo "running pi.bin (by compilium)..."
( time ./pi.bin ) 2>&1 | grep real
echo "running pi.host.bin (by host compiler)..."
( time ./pi.host.bin ) 2>&1 | grep real
echo "running pi.host_o3.bin (by host compiler with -O3)..."
( time ./pi.host_o3.bin ) 2>&1 | grep real
