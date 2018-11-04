#!/bin/bash -e

function test_result {
  input="$1"
  expected="$2"
  ./compilium --target_os `uname` "$input" > out.S
  gcc out.S
  actual=0
  ./a.out || actual=$?
  if [ $expected = $actual ]; then
    echo "PASS $input returns $expected";
  else
    echo "FAIL $input: expected $expected but got $actual"; exit 1; 
  fi
}

# Integer literal
test_result '0' 0
test_result '1' 1
test_result '17' 17
test_result '017' 15

# Non-printable
test_result ' 0 ' 0

# Unary Prefix
test_result '+0' 0
test_result '+ +1' 1
test_result '- -17' 17
test_result '1 - -2' 3

test_result '~10 & 15' 5
test_result '~5 & 15' 10

test_result '!0' 1
test_result '!1' 0
test_result '!2' 0

# Multiplicative
test_result '3 * 4' 12
test_result '3 * 4 * 5' 60
test_result '365 / 7' 52
test_result '365 / 7 / 8' 6
test_result '365 % 7' 1
test_result '365 % 7 % 8' 1

# Additive
test_result '100 + 7' 107
test_result '3 + 5 + 7 + 9' 24
test_result '100 - 7' 93
test_result '1 + 3 - 5 - 7 + 9' 1

# Shift
test_result '3 << 2' 12
test_result '3 >> 2' 0
test_result '17 >> 2' 4
test_result '17 >> 2 >> 1' 2

# Relational
test_result '3 < 5' 1
test_result '5 < 3' 0
test_result '7 < 7' 0

test_result '3 <= 5' 1
test_result '5 <= 3' 0
test_result '7 <= 7' 1

test_result '3 > 5' 0
test_result '5 > 3' 1
test_result '7 > 7' 0

test_result '3 >= 5' 0
test_result '5 >= 3' 1
test_result '7 >= 7' 1

# Equality
test_result '3 == 5' 0
test_result '5 == 3' 0
test_result '7 == 7' 1

test_result '3 != 5' 1
test_result '5 != 3' 1
test_result '7 != 7' 0

# Bitwise logical
test_result '0 & 0' 0
test_result '0 & 1' 0
test_result '1 & 0' 0
test_result '1 & 1' 1

test_result '10 & 6' 2

test_result '0 ^ 0' 0
test_result '0 ^ 1' 1
test_result '1 ^ 0' 1
test_result '1 ^ 1' 0

test_result '10 ^ 6' 12

test_result '0 | 0' 0
test_result '0 | 1' 1
test_result '1 | 0' 1
test_result '1 | 1' 1

test_result '10 | 6' 14

# Boolean logical
test_result '0 && 0' 0
test_result '0 && 1' 0
test_result '1 && 0' 0
test_result '1 && 1' 1

test_result '10 && 6' 1

test_result '0 || 0' 0
test_result '0 || 1' 1
test_result '1 || 0' 1
test_result '1 || 1' 1

test_result '10 || 6' 1

test_result '0 && 0 || 0' 0
test_result '0 && 0 || 1' 1
test_result '0 && 1 || 0' 0
test_result '0 && 1 || 1' 1
test_result '1 && 0 || 0' 0
test_result '1 && 0 || 1' 1
test_result '1 && 1 || 0' 1
test_result '1 && 1 || 1' 1

test_result '0 || 0 && 0' 0
test_result '0 || 0 && 1' 0
test_result '0 || 1 && 0' 0
test_result '0 || 1 && 1' 1
test_result '1 || 0 && 0' 1
test_result '1 || 0 && 1' 1
test_result '1 || 1 && 0' 1
test_result '1 || 1 && 1' 1

# conditional
test_result '0 ? 3 : 5' 5
test_result '1 ? 3 : 5' 3
test_result '2 ? 3 : 5' 3

# comma
test_result '2, 3' 3
test_result '2 * 3, 5 + 7' 12

# Mixed priority
test_result '3 * 4 + 5' 17
test_result '3 + 4 * 5' 23
test_result '3 + 4 * 5 - 9' 14
test_result '3 + 14 / 2' 10
test_result '12 + 17 % 7' 15
test_result '1 + 2 << 3' 24
test_result '-3 * -4 + -5' 7
