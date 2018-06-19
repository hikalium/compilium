TESTS = simple_return \
		simple_return_with_bin_op_plus \
		simple_return_with_bin_op_minus \
		simple_return_with_bin_op_mul \
		simple_return_with_bin_op_mixed_priority \
		simple_return_with_comma_op \
		hello_world

default: $(addsuffix .test, $(TESTS))

FORCE:
.PHONY: FORCE

.SECONDARY:

%.clang.bin : %.c Makefile
	@ rm $@ $*.compilium.log &> /dev/null; \
		{ clang -o $@ $*.c &> $*.clang.log; } \
		|| { echo "FAIL $@"; cat $*.clang.log; false; }

../compilium: FORCE
	@ make -C ..

%.compilium.S : %.c Makefile ../compilium FORCE
	@ rm $@ $*.compilium.log &> /dev/null; \
		../compilium $*.c $@ &> $*.compilium.log \
		|| { echo "FAIL $@"; cat $*.compilium.log; false; }

%.bin : %.S Makefile FORCE
	@ clang -o $@ $*.S

%.test : %.clang.bin %.compilium.bin Makefile FORCE
	@ rm $*.clang.bin.stdout $*.compilium.bin.stdout &> /dev/null; \
		./$*.clang.bin > $*.clang.bin.stdout; expected=$$?; \
		./$*.compilium.bin > $*.compilium.bin.stdout; actual=$$?; \
		if [ $$expected = $$actual ]; then \
		{ diff -u $*.clang.bin.stdout $*.compilium.bin.stdout \
		&& echo "PASS $*" \
		|| echo "FAIL $*: text diff";\
		} else \
		echo "FAIL $*: expected $$expected but got $$actual"; \
		fi

