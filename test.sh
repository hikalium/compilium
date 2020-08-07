#!/bin/bash -e

function test_result {
  input="$1"
  expected="$2"
  expected_stdout="$3"
  testname="$4"
  printf "$expected_stdout" > expected.stdout
  ./compilium --target-os `uname` <<< "$input" > out.S || { \
    echo "$input" > failcase.c; \
    echo "Compilation failed."; \
    exit 1; }
  gcc out.S
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

# nested func call with args
test_src_result "`cat << EOS
int g() {
  int a;
  a = 3;
  int b;
  b = 5;
  return a + b;
}

int f() {
  int v;
  v = 2;
  int r;
  r = g();
  return v + r;
}

int main() {
  return f();
}
EOS
`" 10 ''

# func args should be visible
test_src_result "`cat << EOS
int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(3, 5);
}
EOS
`" 8 ''

# same symbols in diffrent scope shadows previous one
test_src_result "`cat << EOS
int main() {
  int result;
  result = 1;
  int duplicated_var;
  duplicated_var = 2;
  result = result * duplicated_var;
  if(1) {
    int duplicated_var;
    duplicated_var = 3;
    result = result * duplicated_var;
  }
  result = result * duplicated_var;
  return result;
}
EOS
`" 12 ''

# for stmt
test_src_result "`cat << EOS
int main() {
  int i;
  int sum;
  sum = 0;
  for(i = 0; i <= 10; i = i + 1) {
    sum = sum + i;
  }
  return sum;
}
EOS
`" 55 ''

# return with no expression
test_src_result "`cat << EOS
void func_returns_void() {
  return;
}
int main() {
  func_returns_void();
  return 3;
}
EOS
`" 3 ''

test_src_result "`cat << EOS
int three() {
  return 3;
}
int main() {
  return three();
}
EOS
`" 3 ''

test_src_result "`cat << EOS
int puts(char *s);
int main() {
  puts("Hello, world!");
  return 0;
}
EOS
`" 0 'Hello, world!\n'

test_src_result "`cat << EOS
int putchar(int c);
int main() {
  putchar('C');
  return 0;
}
EOS
`" 0 'C'

test_stmt_result 'int a; int b; int c; a = 3; b = 5; c = 7; return a + b + c;' 15

# Non-printable
test_expr_result ' 0 ' 0

# Unary Prefix
test_expr_result '+ +1' 1
test_expr_result '- -17' 17

echo "All tests passed."
