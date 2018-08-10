#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "compilium.h"

void TestASTType() {
  ASTType *type;

  type = AllocAndInitBasicType(kTypeChar);
  assert(GetSizeOfType(type) == 1);

  type = AllocAndInitBasicType(kTypeInt);
  assert(GetSizeOfType(type) == 8);

  type = AllocAndInitASTTypePointerOf(AllocAndInitBasicType(kTypeChar));
  assert(GetSizeOfType(type) == 8);

  type = AllocAndInitASTTypePointerOf(AllocAndInitBasicType(kTypeInt));
  assert(GetSizeOfType(type) == 8);

  type = AllocAndInitASTTypeArrayOf(AllocAndInitBasicType(kTypeChar), 3);
  assert(GetSizeOfType(type) == 3);

  type = AllocAndInitASTTypeArrayOf(AllocAndInitBasicType(kTypeInt), 3);
  assert(GetSizeOfType(type) == 24);

  type = AllocAndInitASTTypePointerOf(
      AllocAndInitASTTypeArrayOf(AllocAndInitBasicType(kTypeChar), 3));
  assert(GetSizeOfType(type) == 8);

  puts("PASS TestASTType");
}

int main(int argc, char *argv[]) {
  TestASTType();
  return 0;
}
