function test_expression() {
  expression=$1
  expected=$2
	echo "int main(){return $expression;}" | ../compilium -o out.S --prefix_type `uname` - &> out.compilium.log \
    || { cat out.compilium.log; echo "FAIL expression $expression: Compilation failed." ; exit 1; }
	gcc -o out.bin out.S
	./out.bin
  actual=$?
  if [ $expected = $actual ]; then
    echo "PASS expression $expression is $expected";
  else
    echo "FAIL expression $expression: expected $expected but got $actual"; exit 1; 
  fi
}

function test_expression_gcc() {
  expression=$1
  expected=$2
	echo "int main(){return $expression;}" | ../compilium -o out.S --prefix_type `uname` - &> out.compilium.log \
    || { cat out.clang.log; echo "FAIL expression $expression: Compilation failed." ; exit 1; }
	gcc -o out.bin out.S
	./out.bin
  actual=$?
  if [ $expected = $actual ]; then
    echo "(gcc)PASS expression $expression is $expected";
  else
    echo "(gcc)FAIL expression $expression: expected $expected but got $actual"; exit 1; 
  fi
}

test_expression "0" 0
test_expression "1" 1

test_expression "2 + 3" 5
test_expression "7 - 3" 4
test_expression "2 * 3" 6
test_expression "7 / 2" 3
test_expression "7 % 2" 1

test_expression "3 << 5" 96
test_expression "5 >> 2" 1

test_expression "3 < 5" 1
test_expression "5 < 3" 0
test_expression "7 < 7" 0

test_expression "3 <= 5" 1
test_expression "5 <= 3" 0
test_expression "7 <= 7" 1

test_expression "3 > 5" 0
test_expression "5 > 3" 1
test_expression "7 > 7" 0

test_expression "3 >= 5" 0
test_expression "5 >= 3" 1
test_expression "7 >= 7" 1

test_expression "2 + 3 * 5" 17
test_expression "2 * 3 + 5" 11
test_expression "2 * 3 + 5 * 7" 41
test_expression "2 * 3 + 5 * 7 / 9 - 11 % 3" 7
test_expression "1 + 2 * 3 / 4 - 5 + 6 * 7 - 8 + 9 + 10 % 11" 50

test_expression "2, 3" 3
test_expression "2 * 3, 5 + 7" 12

