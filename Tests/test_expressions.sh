
function test_expression() {
  expected=$2
	echo "int main(){return $1;}" | ../compilium - out.S &> out.compilium.log \
    || { cat out.compilium.log; echo "FAIL expression $1: Compilation failed." ; exit 1; }
	gcc -o out.bin out.S
	./out.bin
  actual=$?
  if [ $expected = $actual ]; then
    echo "PASS expression $1 is $expected";
  else
    echo "FAIL expression $1: expected $expected but got $actual"; exit 1; 
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

