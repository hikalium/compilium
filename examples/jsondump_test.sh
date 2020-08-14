#!/bin/bash -e

function test_stdout {
  input="$1"
  expected_stdout="$2"
  testname="$3"
  printf "$expected_stdout\n" > expected.stdout
  printf "$input\n" > testinput.c
  cat testinput.c | ./jsondump.bin > out.stdout || { \
    echo "$input" > failcase.txt; \
    echo "Run failed."; \
    exit 1; }
  diff -y expected.stdout out.stdout \
    && printf "\nPASS $testname\n" \
    || { printf "\nFAIL $testname: stdout diff\n"; exit 1; }
}

test_stdout \
"`cat << EOS
true
EOS
`" \
"`cat << EOS
 = true
EOS
`" \
'true value'

test_stdout \
"`cat << EOS
false
EOS
`" \
"`cat << EOS
 = false
EOS
`" \
'false value'

test_stdout \
"`cat << EOS
null
EOS
`" \
"`cat << EOS
 = null
EOS
`" \
'null value'

test_stdout \
"`cat << EOS
[null]
EOS
`" \
"`cat << EOS
[0] = null
EOS
`" \
'Array1'

test_stdout \
"`cat << EOS
[null, true]
EOS
`" \
"`cat << EOS
[0] = null
[1] = true
EOS
`" \
'Array2'

test_stdout \
"`cat << EOS
[[null], [true]]
EOS
`" \
"`cat << EOS
[0][0] = null
[1][0] = true
EOS
`" \
'Nested array'
