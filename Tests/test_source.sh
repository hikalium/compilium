source test_harness.sh

test_source "void func(){ return; } int main(){ func(); return 0; }" 0 "return expression is optional"
