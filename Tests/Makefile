default: test.compilium.S preprocess.compilium.S

%.clang.bin : %.c Makefile
	clang -o $@ $*.c

%.clang.S : %.c Makefile
	clang -S -mllvm --x86-asm-syntax=intel -o $@ $*.c

%.compilium.S : %.c Makefile
	../compilium $*.c
