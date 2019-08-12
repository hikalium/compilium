// This is a first line comment
// This is a second line comment
// This is a third line comment
int puts(char *);
int printf(char *, int);
void exit(int);

void ExpectEq(int actual, int expected, int line) {
  printf("Line %3d: ", line);
  if (actual != expected) {
    puts("FAIL");
    exit(1);
  }
  puts("PASS");
}

int main(int argc, char **argv) {
  ExpectEq(0, 0, __LINE__);
  ExpectEq(1, 1, __LINE__);

  ExpectEq(17, 17, __LINE__);
  ExpectEq(017, 15, __LINE__);

  ExpectEq((0), 0, __LINE__);
  ExpectEq((1), 1, __LINE__);
  ExpectEq((1 ? 2 : 3), 2, __LINE__);
  ExpectEq((0 ? 1 : 2), 2, __LINE__);
  ExpectEq(2 * (3 + 4), 14, __LINE__);

  ExpectEq(~10 & 15, 5, __LINE__);
  ExpectEq(~5 & 15, 10, __LINE__);

  ExpectEq(!0, 1, __LINE__);
  ExpectEq(!1, 0, __LINE__);
  ExpectEq(!2, 0, __LINE__);

  ExpectEq(3 * 4, 12, __LINE__);
  ExpectEq(3 * 4 * 5, 60, __LINE__);
  ExpectEq(365 / 7, 52, __LINE__);
  ExpectEq(365 / 7 / 8, 6, __LINE__);
  ExpectEq(365 % 7, 1, __LINE__);
  ExpectEq(365 % 7 % 8, 1, __LINE__);

  ExpectEq(100 + 7, 107, __LINE__);
  ExpectEq(3 + 5 + 7 + 9, 24, __LINE__);
  ExpectEq(100 - 7, 93, __LINE__);
  ExpectEq(1 + 3 - 5 - 7 + 9, 1, __LINE__);

  ExpectEq(3 << 2, 12, __LINE__);
  ExpectEq(3 >> 2, 0, __LINE__);
  ExpectEq(17 >> 2, 4, __LINE__);
  ExpectEq(17 >> 2 >> 1, 2, __LINE__);

  ExpectEq(3 < 5, 1, __LINE__);
  ExpectEq(5 < 3, 0, __LINE__);
  ExpectEq(7 < 7, 0, __LINE__);

  ExpectEq(3 <= 5, 1, __LINE__);
  ExpectEq(5 <= 3, 0, __LINE__);
  ExpectEq(7 <= 7, 1, __LINE__);

  ExpectEq(3 > 5, 0, __LINE__);
  ExpectEq(5 > 3, 1, __LINE__);
  ExpectEq(7 > 7, 0, __LINE__);

  ExpectEq(3 >= 5, 0, __LINE__);
  ExpectEq(5 >= 3, 1, __LINE__);
  ExpectEq(7 >= 7, 1, __LINE__);

  ExpectEq(3 == 5, 0, __LINE__);
  ExpectEq(5 == 3, 0, __LINE__);
  ExpectEq(7 == 7, 1, __LINE__);

  ExpectEq(3 != 5, 1, __LINE__);
  ExpectEq(5 != 3, 1, __LINE__);
  ExpectEq(7 != 7, 0, __LINE__);

  ExpectEq(0 & 0, 0, __LINE__);
  ExpectEq(0 & 1, 0, __LINE__);
  ExpectEq(1 & 0, 0, __LINE__);
  ExpectEq(1 & 1, 1, __LINE__);

  ExpectEq(10 & 6, 2, __LINE__);

  ExpectEq(0 ^ 0, 0, __LINE__);
  ExpectEq(0 ^ 1, 1, __LINE__);
  ExpectEq(1 ^ 0, 1, __LINE__);
  ExpectEq(1 ^ 1, 0, __LINE__);

  ExpectEq(10 ^ 6, 12, __LINE__);

  ExpectEq(0 | 0, 0, __LINE__);
  ExpectEq(0 | 1, 1, __LINE__);
  ExpectEq(1 | 0, 1, __LINE__);
  ExpectEq(1 | 1, 1, __LINE__);

  ExpectEq(10 | 6, 14, __LINE__);

  ExpectEq(0 && 0, 0, __LINE__);
  ExpectEq(0 && 1, 0, __LINE__);
  ExpectEq(1 && 0, 0, __LINE__);
  ExpectEq(1 && 1, 1, __LINE__);

  ExpectEq(10 && 6, 1, __LINE__);

  ExpectEq(0 || 0, 0, __LINE__);
  ExpectEq(0 || 1, 1, __LINE__);
  ExpectEq(1 || 0, 1, __LINE__);
  ExpectEq(1 || 1, 1, __LINE__);

  ExpectEq(10 || 6, 1, __LINE__);

  ExpectEq(0 && 0 || 0, 0, __LINE__);
  ExpectEq(0 && 0 || 1, 1, __LINE__);
  ExpectEq(0 && 1 || 0, 0, __LINE__);
  ExpectEq(0 && 1 || 1, 1, __LINE__);
  ExpectEq(1 && 0 || 0, 0, __LINE__);
  ExpectEq(1 && 0 || 1, 1, __LINE__);
  ExpectEq(1 && 1 || 0, 1, __LINE__);
  ExpectEq(1 && 1 || 1, 1, __LINE__);

  ExpectEq(0 || 0 && 0, 0, __LINE__);
  ExpectEq(0 || 0 && 1, 0, __LINE__);
  ExpectEq(0 || 1 && 0, 0, __LINE__);
  ExpectEq(0 || 1 && 1, 1, __LINE__);
  ExpectEq(1 || 0 && 0, 1, __LINE__);
  ExpectEq(1 || 0 && 1, 1, __LINE__);
  ExpectEq(1 || 1 && 0, 1, __LINE__);
  ExpectEq(1 || 1 && 1, 1, __LINE__);

  ExpectEq(0 ? 3 : 5, 5, __LINE__);
  ExpectEq(1 ? 3 : 5, 3, __LINE__);
  ExpectEq(2 ? 3 : 5, 3, __LINE__);

  ExpectEq((2, 3), 3, __LINE__);
  ExpectEq((2 * 3, 5 + 7), 12, __LINE__);

  ExpectEq(3 * 4 + 5, 17, __LINE__);
  ExpectEq(3 + 4 * 5, 23, __LINE__);
  ExpectEq(3 + 4 * 5 - 9, 14, __LINE__);
  ExpectEq(3 + 14 / 2, 10, __LINE__);
  ExpectEq(12 + 17 % 7, 15, __LINE__);
  ExpectEq(1 + 2 << 3, 24, __LINE__);
  ExpectEq(-3 * -4 + -5, 7, __LINE__);

  puts("PASS all stmt tests");
  return 0;
}
