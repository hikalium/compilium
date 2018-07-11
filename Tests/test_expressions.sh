
function test_expression() {
  expression=$1
  expected=$2
	echo "int main(){return $expression;}" | ../compilium - out.S `uname` &> out.compilium.log \
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

test_expression "0" 0
test_expression "1" 1
test_expression "2 + 3" 5
test_expression "7 - 3" 4
test_expression "2 * 3" 6
#test_expression "7 / 2" 3
#test_expression "7 % 2" 1
test_expression "2 + 3 * 5" 17
test_expression "2 * 3 + 5" 11
test_expression "2 * 3 + 5 * 7" 41
test_expression "2, 3" 3
test_expression "2 * 3, 5 + 7" 12

