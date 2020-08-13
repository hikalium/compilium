// This is a first line comment
// This is a second line comment
// This is a third line comment
/* This is a block comment */
/*
   This is a block comment 1
 */
/*
   This is a block comment 2
 */
/*
   This is a block comment 3
 */
int puts(char*);
int printf(char*, int);
void exit(int);

void ExpectEq(int actual, int expected, int line) {
  printf("Line %3d: ", line);
  if (actual != expected) {
    puts("FAIL");
    exit(1);
  }
  puts("PASS");
}

int TestIfStmtTrueCase() {
  if (1) return 3;
  return 5;
}

int TestIfStmtFalseCase() {
  if (0) return 3;
  return 5;
}

int TestIfStmtElseCase() {
  if (0) {
    return 2;
  } else {
    return 3;  // expected
  }
  return 5;
}

int TestIfStmtNestedElseCase(int v) {
  int ans;
  ans = 1;
  if (v & 1)
    ans = ans * 2;
  else if (v & 2)
    ans = ans * 3;
  else if (v & 4)
    ans = ans * 5;
  ans = ans * 7;
  return ans;
}

int TestIfStmtWithCompStmt() {
  if (1) {
    return 3;
  }
  return 5;
}

int TestInitializer() {
  int v0 = 3;
  int v1 = v0 * 5 + 7;
  return v0 + v1;  // 25
}

int TestCompAssignPlusEq(int vL, int vR) {
  int v = vL;
  vL += vR;
  return vL;
}

int TestCompAssignMinusEq(int vL, int vR) {
  int v = vL;
  vL -= vR;
  return vL;
}

int TestCompAssignMulEq(int vL, int vR) {
  int v = vL;
  vL *= vR;
  return vL;
}

int TestCompAssignDivEq(int vL, int vR) {
  int v = vL;
  vL /= vR;
  return vL;
}

int TestCompAssignModEq(int vL, int vR) {
  int v = vL;
  vL %= vR;
  return vL;
}

int TestCompAssignLShift(int vL, int vR) {
  int v = vL;
  vL <<= vR;
  return vL;
}

int TestCompAssignRShift(int vL, int vR) {
  int v = vL;
  vL >>= vR;
  return vL;
}

struct Point3D {
  int x;
  int y;
  int z;
};

struct Point2D {
  int x;
  int y;
};

void TestSizeof() {
  char c;
  ExpectEq(sizeof(c), 1, __LINE__);
  int a;
  ExpectEq(sizeof(a), 4, __LINE__);
  int* p;
  ExpectEq(sizeof(p), 8, __LINE__);
  struct IncompleteStruct* incomplete_struct;
  ExpectEq(sizeof(incomplete_struct), 8, __LINE__);
  struct Point2D p2d;
  ExpectEq(sizeof(p2d), 8, __LINE__);
  struct Point3D p3d;
  ExpectEq(sizeof(p3d), 12, __LINE__);
  char s[2 + 3 + 5];
  ExpectEq(sizeof(s), 10, __LINE__);
}

struct Line2D {
  struct Point2D p;
  struct Point2D q;
};

void TestStructVecSum(int x0, int y0, int x1, int y1, int x_expected,
                      int y_expected) {
  struct Point2D v0;
  struct Point2D v1;
  v0.x = x0;
  v0.y = y0;
  v1.x = x1;
  v1.y = y1;
  ExpectEq(v0.x + v1.x, x_expected, __LINE__);
  ExpectEq(v0.y + v1.y, y_expected, __LINE__);
}

void TestStructVecSumRef(int x0, int y0, int x1, int y1, int x_expected,
                         int y_expected) {
  struct Point2D v0;
  struct Point2D v1;
  struct Point2D* vp0 = &v0;
  struct Point2D* vp1 = &v1;
  struct Point2D** vpp0 = &vp0;
  struct Point2D** vpp1 = &vp1;
  v0.x = x0;
  v0.y = y0;
  vp1->x = x1;
  (*vpp1)->y = y1;
  ExpectEq((*vpp0)->x + v1.x, x_expected, __LINE__);
  ExpectEq(vp0->y + v1.y, y_expected, __LINE__);
}

int TestArray(int v0, int v1, int v2, int idx) {
  int a[3];
  a[0] = v0;
  a[1] = v1;
  a[2] = v2;
  return a[idx];
}

int TestWhileLoop(int idx_begin, int idx_end, int idx_inc, int value_inc) {
  int v = 0;
  int i = idx_begin;
  while (i < idx_end) {
    v += value_inc;
    i += idx_inc;
  }
  return v;
}

int TestForLoop(int idx_begin, int idx_end, int idx_inc, int value_inc) {
  int v = 0;
  for (int i = idx_begin; i < idx_end; i += idx_inc) {
    v += value_inc;
  }
  return v;
}

void TestInc() {
  int v = 2;
  ExpectEq(v++, 2, __LINE__);
  ExpectEq(v, 3, __LINE__);
  ExpectEq(v++, 3, __LINE__);
  ExpectEq(v, 4, __LINE__);
  v = 2;
  ExpectEq(v, 2, __LINE__);
  ExpectEq(++v, 3, __LINE__);
  ExpectEq(v, 3, __LINE__);
  ExpectEq(++v, 4, __LINE__);
}

void TestDec() {
  int v = 2;
  ExpectEq(v--, 2, __LINE__);
  ExpectEq(v, 1, __LINE__);
  ExpectEq(v--, 1, __LINE__);
  ExpectEq(v, 0, __LINE__);
  v = 2;
  ExpectEq(v, 2, __LINE__);
  ExpectEq(--v, 1, __LINE__);
  ExpectEq(v, 1, __LINE__);
  ExpectEq(--v, 0, __LINE__);
}

int TestCharLiteralAccess() {
  ExpectEq(*"compilium", 'c', __LINE__);
  ExpectEq(*("compilium" + 0), 'c', __LINE__);
  ExpectEq(*("compilium" + 1), 'o', __LINE__);
  ExpectEq(*("compilium" + 2), 'm', __LINE__);
  ExpectEq(*("compilium" + 3), 'p', __LINE__);
  ExpectEq(*("compilium" + 4), 'i', __LINE__);
  ExpectEq(*("compilium" + 5), 'l', __LINE__);
  ExpectEq(*("compilium" + 6), 'i', __LINE__);
  ExpectEq(*("compilium" + 7), 'u', __LINE__);
  ExpectEq(*("compilium" + 8), 'm', __LINE__);
  ExpectEq(*("compilium" + 9), 0, __LINE__);
}

void TestReassign() {
  char c = 2;
  c = c + 1;
  ExpectEq(c, 3, __LINE__);
  c = 0;
  ExpectEq(c, 0, __LINE__);
  c = 2;
  ExpectEq(c, 2, __LINE__);
  c = 0;
  ExpectEq(c, 0, __LINE__);
  c = 2 + 3;
  ExpectEq(c, 5, __LINE__);
  ExpectEq(c + 2, 7, __LINE__);
}

int UnreachableReturn() {
  ;
  return 2;
  return 0;
}

void TestPtrOfVar() {
  int a;
  int* p;
  a = 1;
  p = &a;
  *p = 5;
  ExpectEq(a, 5, __LINE__);
  a = 1;
  p = &a;
  a = 3;
  ExpectEq(*p, 3, __LINE__);
  p = &a;
  ExpectEq(p ? 1 : 0, 1, __LINE__);
}

void TestConstTypeSpec() { const int a = 0; }

void TestBreak() {
  int v;
  v = 1;
  for (;;) {
    for (;;) {
      v *= 2;
      break;
    }
    v *= 3;
    break;
  }
  ExpectEq(v, 6, __LINE__);

  v = 1;
  while (1) {
    while (1) {
      v *= 3;
      break;
    }
    v *= 5;
    break;
  }
  ExpectEq(v, 15, __LINE__);
}

void TestShortCircuitEval() {
  int v = 1;
  v++ && 0 && v++;
  ExpectEq(v, 2, __LINE__);
  v = 1;
  1 || v++;
  ExpectEq(v, 1, __LINE__);
}

int main(int argc, char** argv) {
  TestShortCircuitEval();
  TestBreak();
  TestConstTypeSpec();
  TestPtrOfVar();
  TestReassign();
  TestCharLiteralAccess();
  TestInc();

  ExpectEq('C', 67, __LINE__);

  ExpectEq(UnreachableReturn(), 2, __LINE__);

  ExpectEq(+0, 0, __LINE__);
  ExpectEq(1 - -2, 3, __LINE__);

  ExpectEq(0, 0, __LINE__);
  ExpectEq(1, 1, __LINE__);

  ExpectEq(0xF, 15, __LINE__);
  ExpectEq(0xFF, 255, __LINE__);
  ExpectEq(0x010, 16, __LINE__);
  ExpectEq(0x100, 256, __LINE__);

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

  ExpectEq(TestIfStmtTrueCase(), 3, __LINE__);
  ExpectEq(TestIfStmtFalseCase(), 5, __LINE__);
  ExpectEq(TestIfStmtWithCompStmt(), 3, __LINE__);
  ExpectEq(TestIfStmtElseCase(), 3, __LINE__);

  ExpectEq(TestIfStmtNestedElseCase(0), 7, __LINE__);
  ExpectEq(TestIfStmtNestedElseCase(1), 14, __LINE__);
  ExpectEq(TestIfStmtNestedElseCase(2), 21, __LINE__);
  ExpectEq(TestIfStmtNestedElseCase(3), 14, __LINE__);
  ExpectEq(TestIfStmtNestedElseCase(4), 35, __LINE__);
  ExpectEq(TestIfStmtNestedElseCase(5), 14, __LINE__);
  ExpectEq(TestIfStmtNestedElseCase(6), 21, __LINE__);
  ExpectEq(TestIfStmtNestedElseCase(7), 14, __LINE__);

  ExpectEq(TestInitializer(), 25, __LINE__);

  ExpectEq(TestCompAssignPlusEq(3, 5), 8, __LINE__);
  ExpectEq(TestCompAssignPlusEq(-5, 5), 0, __LINE__);

  ExpectEq(TestCompAssignMinusEq(3, 5), -2, __LINE__);
  ExpectEq(TestCompAssignMinusEq(-5, 5), -10, __LINE__);

  ExpectEq(TestCompAssignMulEq(3, 0), 0, __LINE__);
  ExpectEq(TestCompAssignMulEq(0, 5), 0, __LINE__);
  ExpectEq(TestCompAssignMulEq(3, 5), 15, __LINE__);
  ExpectEq(TestCompAssignMulEq(5, 3), 15, __LINE__);

  ExpectEq(TestCompAssignDivEq(8, 2), 4, __LINE__);
  ExpectEq(TestCompAssignDivEq(13, 5), 2, __LINE__);

  ExpectEq(TestCompAssignModEq(8, 2), 0, __LINE__);
  ExpectEq(TestCompAssignModEq(13, 5), 3, __LINE__);

  ExpectEq(TestCompAssignModEq(13, 5), 3, __LINE__);

  TestSizeof();

  TestStructVecSum(1, 2, 3, 4, 4, 6);
  TestStructVecSum(2, 3, 5, 7, 7, 10);

  TestStructVecSumRef(1, 2, 3, 4, 4, 6);
  TestStructVecSumRef(2, 3, 5, 7, 7, 10);

  ExpectEq(TestArray(2, 3, 5, 0), 2, __LINE__);
  ExpectEq(TestArray(2, 3, 5, 1), 3, __LINE__);
  ExpectEq(TestArray(2, 3, 5, 2), 5, __LINE__);

  ExpectEq(TestArray(7, 11, 13, 0), 7, __LINE__);
  ExpectEq(TestArray(7, 11, 13, 1), 11, __LINE__);
  ExpectEq(TestArray(7, 11, 13, 2), 13, __LINE__);

  ExpectEq(TestWhileLoop(7, 11, 2, 5), 10, __LINE__);
  ExpectEq(TestWhileLoop(11, 7, 2, 5), 0, __LINE__);
  ExpectEq(TestWhileLoop(7, 11, 1, 5), 20, __LINE__);

  ExpectEq(TestForLoop(7, 11, 2, 5), 10, __LINE__);
  ExpectEq(TestForLoop(11, 7, 2, 5), 0, __LINE__);
  ExpectEq(TestForLoop(7, 11, 1, 5), 20, __LINE__);

  ExpectEq(TestCompAssignLShift(1, 0), 1, __LINE__);
  ExpectEq(TestCompAssignLShift(1, 3), 8, __LINE__);
  ExpectEq(TestCompAssignLShift(5, 0), 5, __LINE__);
  ExpectEq(TestCompAssignLShift(5, 3), 40, __LINE__);

  ExpectEq(TestCompAssignRShift(32, 0), 32, __LINE__);
  ExpectEq(TestCompAssignRShift(32, 3), 4, __LINE__);
  ExpectEq(TestCompAssignRShift(32, 6), 0, __LINE__);
  ExpectEq(TestCompAssignRShift(7, 2), 1, __LINE__);

  puts("PASS all stmt tests");
  return 0;
}
