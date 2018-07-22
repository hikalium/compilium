function test_source() {
  source=$1
  expected=$2
  test_name=$3
	echo "$source" | ../compilium -o out.S --prefix_type `uname` - &> out.compilium.log \
    || { cat out.compilium.log; echo "FAIL $test_name: Compilation failed." ; exit 1; }
	gcc -o out.bin out.S
	./out.bin
  actual=$?
  if [ $expected = $actual ]; then
    echo "PASS $test_name";
  else
    echo "FAIL $test_name: expected $expected but got $actual"; exit 1; 
  fi
}
