source test_harness.sh

function test_statements() {
  statements=$1
  expected=$2
	test_source "int main(){$statements}" $expected "statements $statements"
}

test_statements "return 0;" 0
test_statements "return 5;" 5
test_statements "return 3; return 5;" 3

test_statements ";return 0;" 0

test_statements "{}return 0;" 0

test_statements "int foo; foo = 7; ++foo; ++foo; return ++foo;" 10
test_statements "int foo; foo = 7; ++foo; ++foo; return foo;" 9
test_statements "int foo; foo = 7; --foo; --foo; return --foo;" 4
test_statements "int foo; foo = 7; --foo; --foo; return foo;" 5

test_statements "int foo; foo = 7; foo++; foo++; return foo++;" 9
test_statements "int foo; foo = 7; foo++; foo++; return foo;" 9
test_statements "int foo; foo = 7; foo--; foo--; return foo--;" 5
test_statements "int foo; foo = 7; foo--; foo--; return foo;" 5

test_statements "int foo; foo = 7; foo *= 3; return foo;" 21
test_statements "int foo; foo = 7; foo /= 3; return foo;" 2
test_statements "int foo; foo = 7; foo %= 3; return foo;" 1
test_statements "int foo; foo = 7; foo += 3; return foo;" 10
test_statements "int foo; foo = 7; foo += foo + 3; return foo;" 17
test_statements "int foo; foo = 7; foo -= 3; return foo;" 4
test_statements "int foo; foo = 7; foo <<= 3; return foo;" 56
test_statements "int foo; foo = 9; foo >>= 3; return foo;" 1
test_statements "int foo; foo = 10; foo &= 6; return foo;" 2
test_statements "int foo; foo = 10; foo ^= 6; return foo;" 12
test_statements "int foo; foo = 10; foo |= 6; return foo;" 14

test_statements "int foo; foo = 5; return foo;" 5
test_statements "int foo; foo = 5; return foo * foo;" 25
test_statements "int foo; int bar; foo = 3; bar = 7; return foo * bar;" 21
test_statements "int foo; foo = 7; foo = foo + 5; foo = foo + foo; foo = 3 + foo; return foo;" 27
test_statements "int foo; foo = 7; int bar; bar = 3; foo = bar; return bar;" 3
test_statements "int foo; int bar; foo = bar = 3; bar = bar + 1; return foo * bar;" 12

test_statements "int foo; foo = 0; if(foo) return 3; return 5;" 5
test_statements "int foo; foo = 1; if(foo) return 3; return 5;" 3
test_statements "int foo; foo = 0; if(foo) foo = 2; return foo;" 0
test_statements "int foo; foo = 1; if(foo) foo = 2; return foo;" 2
test_statements "int foo; int bar; foo = 1; bar = foo + 1; bar = foo + 2; return foo;" 1
test_statements "int foo; foo = 1; if(foo == 1){ foo = foo + 1; foo = foo + 1; } if(foo == 1){ foo = foo + 1; foo = foo + 1; } return foo;" 3
test_statements "int foo; foo = 1; if(foo == 1){ foo = 7; } else if(foo == 2){ foo = 11; } else{ foo = 13; } return foo;" 7
test_statements "int foo; foo = 2; if(foo == 1){ foo = 7; } else if(foo == 2){ foo = 11; } else{ foo = 13; } return foo;" 11
test_statements "int foo; foo = 3; if(foo == 1){ foo = 7; } else if(foo == 2){ foo = 11; } else{ foo = 13; } return foo;" 13

test_statements "int i; i = 0; while(i < 5) ++i; return i;" 5
test_statements "int i; i = 0; while(i < 5){ ++i; ++i; } return i;" 6
test_statements "int i; i = 5; while(0) ++i; return i;" 5
test_statements "int i; i = 0; while(i++ < 10){ if(i == 5) break; }; return i;" 5

test_statements "int i; for(i = 0; i < 5; ++i){} return i;" 5
test_statements "int i; int n; n = 0; for(i = 2; i < 5; ++i){ ++n; } return n;" 3
test_statements "int i; for(i = 0; i < 10; ++i){ if(i == 5) break; } return i;" 5
