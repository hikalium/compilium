#!/bin/bash -e

function test_result {
  input=$1
  expected=$2
  ./compilium $input | gcc -xassembler -
  actual=0
  ./a.out || actual=$?
  if [ $expected = $actual ]; then
    echo "PASS $input returns $expected";
  else
    echo "FAIL $input: expected $expected but got $actual"; exit 1; 
  fi
}

test_result '0' 0
test_result '1' 1
test_result '17' 17
