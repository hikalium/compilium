#!/bin/bash -e

function test_stdout {
  input="$1"
  expected_stdout="$2"
  testname="$3"
  printf "$expected_stdout" > expected.stdout
  printf "$input" > testinput.c
  cat testinput.c | ./compilium -E --target-os `uname` > out.stdout || { \
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

test_stdout \
"`cat << EOS
int   one;

int   two;
int three;
EOS
`" \
"`cat << EOS
int   one;

int   two;
int three;
EOS
`" \
'keep white spaces and new lines'

