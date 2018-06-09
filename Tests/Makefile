default: simple_return.test # hello_world.test

.FORCE:

.SECONDARY:

%.clang.bin : %.c Makefile
	@ clang -o $@ $*.c

%.clang.S : %.c Makefile
	@ clang -S -mllvm --x86-asm-syntax=intel -o $@ $*.c

%.compilium.S : %.c Makefile .FORCE
	@ make -q -C ..
	@ ../compilium $*.c $@ > $*.compilium.log

%.bin : %.S Makefile .FORCE
	@ clang -o $@ $*.S

%.test : %.clang.bin %.compilium.bin Makefile
	@ rm $*.clang.bin.stdout $*.compilium.bin.stdout; \
		./$*.clang.bin > $*.clang.bin.stdout; \
		./$*.compilium.bin > $*.compilium.bin.stdout; \
		diff -u $*.clang.bin.stdout $*.compilium.bin.stdout
	@ ./$*.clang.bin; ./check_exit_code.sh ./$*.compilium.bin $$?
