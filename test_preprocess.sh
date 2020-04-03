#!/bin/bash -e

function test_stdout {
  input="$1"
  expected_stdout="$2"
  testname="$3"
  printf "$expected_stdout" > expected.stdout
  ./compilium -E --target-os `uname` <<< "$input" > out.stdout || { \
    echo "$input" > failcase.txt; \
    echo "Compilation failed."; \
    exit 1; }
  diff -u expected.stdout out.stdout \
    && echo "PASS $testname" \
    || { echo "FAIL $testname: stdout diff"; exit 1; }
}

test_stdout \
"`cat << EOS
EOS
`" \
"`cat << EOS
EOS
`" \
'empty input becomes empty'

