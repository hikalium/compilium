SHELL=/bin/bash
TESTS = simple_return \
		printf \
		hello_world \
		local_variable_def_assign_ref \
		tokenize_tab

default: expr_test $(addsuffix .test, $(TESTS))

FORCE:
.PHONY: FORCE

.SECONDARY:

expr_test : FORCE
	@./test_expressions.sh

%.clang.bin : %.c Makefile
	@ rm $@ $*.compilium.log &> /dev/null; \
		{ gcc -o $@ $*.c &> $*.clang.log; } \
		|| { echo "FAIL $@"; cat $*.clang.log; false; }

../compilium : FORCE
	@ make -C ..

%.clang.S : %.c Makefile FORCE
	clang -O0 -S -o $*.clang.S $*.c

%.compilium.S : %.c Makefile ../compilium FORCE
	@ rm $@ $*.compilium.log &> /dev/null; \
		../compilium -o $@ --prefix_type `uname` $*.c &> $*.compilium.log \
		|| { echo "FAIL $@"; cat $*.compilium.log; false; }

%.bin : %.S Makefile FORCE
	@ gcc -o $@ $*.S

%.test : %.clang.bin %.compilium.bin Makefile FORCE
	@ rm $*.clang.bin.stdout $*.compilium.bin.stdout &> /dev/null; true
	@ ./$*.clang.bin > $*.clang.bin.stdout; expected=$$?; \
		./$*.compilium.bin > $*.compilium.bin.stdout; actual=$$?; \
		if [ $$expected = $$actual ]; then \
		{ diff -u $*.clang.bin.stdout $*.compilium.bin.stdout \
		&& echo "PASS $*" \
		|| echo "FAIL $*: text diff";\
		} else \
		echo "FAIL $*: expected $$expected but got $$actual"; \
		fi

clean:
	-rm *.bin *.stdout *.S *.s *.log

