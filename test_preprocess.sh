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

# 6.10.8.1 Mandatory macros - 1 
# 5.1.1.2 Translation phases - 2
# 
# $ printf 'one\\\n __LINE__ two' | clang -E -
# one 2 two
# $ printf 'one\n __LINE__ two' | clang -E -
# one
#  2 two

test_stdout \
"`cat << EOS
one\\\\
 __LINE__ two
EOS
`" \
"`cat << EOS
one 2 two
EOS
`" \
'__LINE__ macro shows physical source line, not logical one.'

test_stdout \
"`cat << EOS
one
 __LINE__ two
three__LINE__
four __LINE__ __LINE__
EOS
`" \
"`cat << EOS
one
 2 two
three__LINE__
four 4 4
EOS
`" \
'__LINE__ macro'

test_stdout \
"`cat << EOS
#define hello "Hello, world!"
printf(hello);
EOS
`" \
"`cat << EOS
printf("Hello, world!");
EOS
`" \
'Simple macro replacement'

test_stdout \
"`cat << EOS
#define hello printf("Hello, world!")
hello;
EOS
`" \
"`cat << EOS
printf("Hello, world!");
EOS
`" \
'Simple macro replacement with multiple tokens'

test_stdout \
"`cat << EOS
#define hello
hello;
EOS
`" \
"`cat << EOS
;
EOS
`" \
'Simple macro replacement with zero tokens'
