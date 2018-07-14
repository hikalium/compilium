source test_harness.sh

function test_statements() {
  statements=$1
  expected=$2
	test_source "int main(){$statements}" $expected "statements $statements"
}

test_statements "return 0;" 0
test_statements "return 5;" 5

test_statements "int foo; foo = 5; return foo;" 5
test_statements "int foo; foo = 5; return foo * foo;" 25
test_statements "int foo; int bar; foo = 3; bar = 7; return foo * bar;" 21
test_statements "int foo; foo = 7; foo = foo + 5; foo = foo + foo; foo = 3 + foo; return foo;" 27
test_statements "int foo; foo = 7; int bar; bar = 3; foo = bar; return bar;" 3
test_statements "int foo; int bar; foo = bar = 3; bar = bar + 1; return foo * bar;" 12
