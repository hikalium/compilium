#!/bin/bash -e

function test_stdout {
  input="$1"
  expected_stdout="$2"
  testname="$3"
  printf "%s" "$expected_stdout" > expected.stdout
  printf "%s" "$input" > testinput.c
  cat testinput.c | ./compilium -E --target-os `uname` > out.stdout || { \
    echo "$input" > failcase.txt; \
    echo "Compilation failed."; \
    exit 1; }
  diff -y expected.stdout out.stdout \
    && printf "\nPASS $testname\n" \
    || { printf "\nFAIL $testname: stdout diff\n"; exit 1; }
}

test_stdout \
"`cat << EOS
#define MACRO_DEFINED
#ifdef MACRO_DEFINED
int this_should_be_visible_1;
#else
int this_should_not_be_visible_1;
#endif
#ifdef MACRO_NOT_DEFINED
int this_should_not_be_visible_2;
#else
int this_should_be_visible_2;
#endif
int always_visible;
EOS
`" \
"`cat << EOS

int this_should_be_visible_1;


int this_should_be_visible_2;

int always_visible;

EOS
`" \
'ifdef nested case'

test_stdout \
"`cat << EOS
#define MACRO_DEFINED
#ifdef MACRO_DEFINED
int this_should_be_visible;
#ifdef MACRO_DEFINED
#ifdef MACRO_NOT_DEFINED
int this_is_not_visible;
#endif
int this_is_also_visible;
#endif
#endif
int always_visible;
EOS
`" \
"`cat << EOS

int this_should_be_visible;


int this_is_also_visible;


int always_visible;

EOS
`" \
'ifdef nested case'

test_stdout \
"`cat << EOS
#ifdef MACRO_NOT_DEFINED
int this_is_not_visible;
#endif
int always_visible;
EOS
`" \
"`cat << EOS

int always_visible;

EOS
`" \
'ifdef not defined case'

test_stdout \
"`cat << EOS
#define MACRO_DEFINED
#ifdef MACRO_DEFINED
int this_should_be_visible;
#endif
int always_visible;
EOS
`" \
"`cat << EOS

int this_should_be_visible;

int always_visible;

EOS
`" \
'ifdef defined case'

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
#define EOF (-1)
EOF;
EOS
`" \
"`cat << EOS
printf("Hello, world!");
(-1);
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

test_stdout \
"`cat << EOS
#define f0() printf("Zero")
#define f1(a) printf("One %d", a)
#define f2(a, b) printf("Two %d %d", a, b)
f0();
f1(1 + 1);
f2(1 + 1, 3);
EOS
`" \
"`cat << EOS
printf("Zero");
printf("One %d", 1 + 1);
printf("Two %d %d", 1 + 1, 3);
EOS
`" \
'Function-like macros'

test_stdout \
"`cat << EOS
#define f1(a) printf("One %s", #a)
#define f2(a, b) printf("Two %s %d", #a, b)
f1(1 + 1);
f2(1 + 1, 3);
EOS
`" \
"`cat << EOS
printf("One %s", "1 + 1");
printf("Two %s %d", "1 + 1", 3);
EOS
`" \
'Function-like macros with #expr macro'
