%.clang.bin : %.c Makefile
	clang -o $@ $*.c

%.clang.S : %.c Makefile
	clang -S -mllvm --x86-asm-syntax=intel -o $@ $*.c
