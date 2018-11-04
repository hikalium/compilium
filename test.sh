#!/bin/bash -e

function test_result {
  input=$1
  expected=$2
  ./compilium $input > out.S
  gcc out.S
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
test_result '017' 15

test_result '100+7' 107
test_result '3+5+7+9' 24
