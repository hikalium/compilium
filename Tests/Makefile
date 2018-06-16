TESTS = simple_return \
		simple_return_with_bin_op_plus \
		simple_return_with_bin_op_minus \
		simple_return_with_bin_op_mul \
		simple_return_with_bin_op_mixed_priority

default: $(addsuffix .test, $(TESTS))

FORCE:
.PHONY: FORCE

.SECONDARY:

%.clang.bin : %.c Makefile
	@ clang -o $@ $*.c

%.clang.S : %.c Makefile
	@ clang -S -mllvm --x86-asm-syntax=intel -o $@ $*.c

../compilium: FORCE
	@ make -C ..

%.compilium.S : %.c Makefile ../compilium FORCE
	@ rm $@ $*.compilium.log; \
		../compilium $*.c $@ > $*.compilium.log

%.bin : %.S Makefile FORCE
	@ clang -o $@ $*.S

%.test : %.clang.bin %.compilium.bin Makefile FORCE
	@ rm $*.clang.bin.stdout $*.compilium.bin.stdout; \
		./$*.clang.bin > $*.clang.bin.stdout; \
		./$*.compilium.bin > $*.compilium.bin.stdout; \
		diff -u $*.clang.bin.stdout $*.compilium.bin.stdout
	@ ./$*.clang.bin; ./check_exit_code.sh ./$*.compilium.bin $$?
