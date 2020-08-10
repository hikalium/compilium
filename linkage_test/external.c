int ext_val;

void ExpectEq(int actual, int expected, int line);

void ef2() {
  ExpectEq(ext_val, 1, __LINE__);
  ext_val++;  
  ExpectEq(ext_val, 2, __LINE__);
}
