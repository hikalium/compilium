source test_harness.sh

function test_expression() {
  expression=$1
  expected=$2
	test_source "int main(){return $expression;}" $expected "expression $expression"
}

test_expression "0" 0
test_expression "1" 1

test_expression "- -3" 3
test_expression "+ +5" 5

test_expression "2 + 3" 5
test_expression "7 - 3" 4
test_expression "2 * 3" 6
test_expression "7 / 2" 3
test_expression "7 % 2" 1

test_expression "7 + +3" 10
test_expression "7 + -3" 4
test_expression "7 - +3" 4
test_expression "7 - -3" 10

test_expression "-7 + +3 + 10" 6
test_expression "-7 + -3 + 10" 0
test_expression "-7 - +3 + 10" 0
test_expression "-7 - -3 + 10" 6

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

test_expression "3 == 5" 0
test_expression "5 == 3" 0
test_expression "7 == 7" 1

test_expression "3 != 5" 1
test_expression "5 != 3" 1
test_expression "7 != 7" 0

test_expression "0 & 0" 0
test_expression "0 & 1" 0
test_expression "1 & 0" 0
test_expression "1 & 1" 1

test_expression "10 & 6" 2

test_expression "0 ^ 0" 0
test_expression "0 ^ 1" 1
test_expression "1 ^ 0" 1
test_expression "1 ^ 1" 0

test_expression "10 ^ 6" 12

test_expression "0 | 0" 0
test_expression "0 | 1" 1
test_expression "1 | 0" 1
test_expression "1 | 1" 1

test_expression "10 | 6" 14

test_expression "0 && 0" 0
test_expression "0 && 1" 0
test_expression "1 && 0" 0
test_expression "1 && 1" 1

test_expression "10 && 6" 1

test_expression "0 || 0" 0
test_expression "0 || 1" 1
test_expression "1 || 0" 1
test_expression "1 || 1" 1

test_expression "10 || 6" 1

test_expression "2 + 3 * 5" 17
test_expression "2 * 3 + 5" 11
test_expression "2 * 3 + 5 * 7" 41
test_expression "2 * 3 + 5 * 7 / 9 - 11 % 3" 7
test_expression "1 + 2 * 3 / 4 - 5 + 6 * 7 - 8 + 9 + 10 % 11" 50

test_expression "2, 3" 3
test_expression "2 * 3, 5 + 7" 12
