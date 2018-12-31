#!/bin/bash -e

compilium=${COMPILER_BINARY:-./compilium}
host_cc=${HOST_CC:-cc}
target_os=$(uname)
function test_result {
  input="$1"
  expected="$2"
  expected_stdout="$3"
  testname="$4"
  printf "$expected_stdout" > expected.stdout
  ${compilium} --target-os ${target_os} "$input" > out.S || { \
    echo "$input" > failcase.c; \
    echo "Compilation failed."; \
    exit 1; }
  ${host_cc} out.S
  actual=0
  ./a.out > out.stdout || actual=$?
  if [ $expected = $actual ]; then
      diff -u expected.stdout out.stdout \
        && echo "PASS $testname returns $expected" \
        || { echo "FAIL $testname: stdout diff"; exit 1; }
  else
    echo "FAIL $testname: expected $expected but got $actual"; echo $input > failcase.c; exit 1;
  fi
}

function test_expr_result {
  test_result "int main(){return $1;}" "$2" "" "$1"
}

function test_stmt_result {
  test_result "int main(){$1}" "$2" "" "$1"
}

function test_src_result {
  test_result "$1" "$2" "$3" "$1"
}

test_src_result 'int puts(char *s); int main() { puts("Hello, world!"); return 0;}' 0 'Hello, world!\n'
test_src_result "int putchar(int c); int main() { putchar('C'); return 0;}" 0 'C'
test_stmt_result 'return *("compilium" + 1);' 111
test_stmt_result 'return *"compilium";' 99
test_stmt_result "return 'C';" 67
test_stmt_result 'char c; c = 2; c = c + 1; return c;' 3
test_stmt_result 'char c; return sizeof(c);' 1
test_stmt_result 'int a; a = 1; int *p; p = &a; *p = 5; return a;' 5
test_stmt_result 'int a; a = 1; int *p; p = &a; a = 3; return *p;' 3
test_stmt_result 'int a; int *p; p = &a; return p ? 1 : 0;' 1
test_stmt_result 'int a; int b; int c; a = 3; b = 5; c = 7; return a + b + c;' 15
test_stmt_result 'int a; return sizeof(a);' 4
test_stmt_result 'int *a; return sizeof(a);' 8
test_stmt_result 'int *a; return 2;' 2
test_stmt_result 'int a; a = 0; a = 2; return a;' 2
test_stmt_result 'int a; a = 2; a = 0; return a;' 0
test_stmt_result 'int a; a = 2 + 3; return a;' 5
test_stmt_result 'int a; a = 2 + 3; return a + 2;' 7

# Integer literal
test_expr_result '0' 0
test_expr_result '1' 1
test_expr_result '17' 17
test_expr_result '017' 15

# Paren
test_expr_result '(0)' 0
test_expr_result '(1)' 1
test_expr_result '(1 ? 2 : 3)' 2
test_expr_result '(0 ? 1 : 2)' 2
test_expr_result '2 * (3 + 4)' 14

# Non-printable
test_expr_result ' 0 ' 0

# Unary Prefix
test_expr_result '+0' 0
test_expr_result '+ +1' 1
test_expr_result '- -17' 17
test_expr_result '1 - -2' 3

test_expr_result '~10 & 15' 5
test_expr_result '~5 & 15' 10

test_expr_result '!0' 1
test_expr_result '!1' 0
test_expr_result '!2' 0

# Multiplicative
test_expr_result '3 * 4' 12
test_expr_result '3 * 4 * 5' 60
test_expr_result '365 / 7' 52
test_expr_result '365 / 7 / 8' 6
test_expr_result '365 % 7' 1
test_expr_result '365 % 7 % 8' 1

# Additive
test_expr_result '100 + 7' 107
test_expr_result '3 + 5 + 7 + 9' 24
test_expr_result '100 - 7' 93
test_expr_result '1 + 3 - 5 - 7 + 9' 1

# Shift
test_expr_result '3 << 2' 12
test_expr_result '3 >> 2' 0
test_expr_result '17 >> 2' 4
test_expr_result '17 >> 2 >> 1' 2

# Relational
test_expr_result '3 < 5' 1
test_expr_result '5 < 3' 0
test_expr_result '7 < 7' 0

test_expr_result '3 <= 5' 1
test_expr_result '5 <= 3' 0
test_expr_result '7 <= 7' 1

test_expr_result '3 > 5' 0
test_expr_result '5 > 3' 1
test_expr_result '7 > 7' 0

test_expr_result '3 >= 5' 0
test_expr_result '5 >= 3' 1
test_expr_result '7 >= 7' 1

# Equality
test_expr_result '3 == 5' 0
test_expr_result '5 == 3' 0
test_expr_result '7 == 7' 1

test_expr_result '3 != 5' 1
test_expr_result '5 != 3' 1
test_expr_result '7 != 7' 0

# Bitwise logical
test_expr_result '0 & 0' 0
test_expr_result '0 & 1' 0
test_expr_result '1 & 0' 0
test_expr_result '1 & 1' 1

test_expr_result '10 & 6' 2

test_expr_result '0 ^ 0' 0
test_expr_result '0 ^ 1' 1
test_expr_result '1 ^ 0' 1
test_expr_result '1 ^ 1' 0

test_expr_result '10 ^ 6' 12

test_expr_result '0 | 0' 0
test_expr_result '0 | 1' 1
test_expr_result '1 | 0' 1
test_expr_result '1 | 1' 1

test_expr_result '10 | 6' 14

# Boolean logical
test_expr_result '0 && 0' 0
test_expr_result '0 && 1' 0
test_expr_result '1 && 0' 0
test_expr_result '1 && 1' 1

test_expr_result '10 && 6' 1

test_expr_result '0 || 0' 0
test_expr_result '0 || 1' 1
test_expr_result '1 || 0' 1
test_expr_result '1 || 1' 1

test_expr_result '10 || 6' 1

test_expr_result '0 && 0 || 0' 0
test_expr_result '0 && 0 || 1' 1
test_expr_result '0 && 1 || 0' 0
test_expr_result '0 && 1 || 1' 1
test_expr_result '1 && 0 || 0' 0
test_expr_result '1 && 0 || 1' 1
test_expr_result '1 && 1 || 0' 1
test_expr_result '1 && 1 || 1' 1

test_expr_result '0 || 0 && 0' 0
test_expr_result '0 || 0 && 1' 0
test_expr_result '0 || 1 && 0' 0
test_expr_result '0 || 1 && 1' 1
test_expr_result '1 || 0 && 0' 1
test_expr_result '1 || 0 && 1' 1
test_expr_result '1 || 1 && 0' 1
test_expr_result '1 || 1 && 1' 1

# conditional
test_expr_result '0 ? 3 : 5' 5
test_expr_result '1 ? 3 : 5' 3
test_expr_result '2 ? 3 : 5' 3

# comma
test_expr_result '2, 3' 3
test_expr_result '2 * 3, 5 + 7' 12

# Mixed priority
test_expr_result '3 * 4 + 5' 17
test_expr_result '3 + 4 * 5' 23
test_expr_result '3 + 4 * 5 - 9' 14
test_expr_result '3 + 14 / 2' 10
test_expr_result '12 + 17 % 7' 15
test_expr_result '1 + 2 << 3' 24
test_expr_result '-3 * -4 + -5' 7

test_stmt_result '; ; return 0;' 0
test_stmt_result '; return 2; return 0;' 2

echo "All tests passed."
