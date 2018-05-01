default: simple_return.test

.FORCE :

%.clang.bin : %.c Makefile
	clang -o $@ $*.c

%.clang.S : %.c Makefile
	clang -S -mllvm --x86-asm-syntax=intel -o $@ $*.c

%.compilium.S : %.c Makefile .FORCE
	../compilium $*.c $@
	cat $@

%.bin : %.S Makefile .FORCE
	clang -o $@ $*.S

%.test : %.clang.bin %.compilium.bin Makefile
	@ ./$*.clang.bin; ./test.sh ./$*.compilium.bin $$?
