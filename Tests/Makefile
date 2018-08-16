SHELL=/bin/bash
TESTS ?= $(basename $(wildcard *.c))
CFLAGS=-Wall -Wpedantic -I../include/$(shell uname)

default: expr_test stmt_test src_test c_test

FORCE:
.PHONY: FORCE

.SECONDARY:

expr_test : ../compilium FORCE
	@./test_expressions.sh

stmt_test : ../compilium FORCE
	@./test_statements.sh

src_test : ../compilium FORCE
	@./test_source.sh

c_test : ../compilium $(addsuffix .test, $(TESTS)) FORCE

%.clang.bin : %.c Makefile
	@ rm $@ $*.compilium.log &> /dev/null; \
		{ clang $(CFLAGS) -o $@ $*.c &> $*.clang.log; } \
		|| { echo "FAIL $@"; cat $*.clang.log; false; }

../compilium : FORCE
	@ make -C ..

%.clang.S : %.c Makefile FORCE
	clang $(CFLAGS) -O0 -S -o $*.clang.S $*.c

%.compilium.S : %.c Makefile ../compilium FORCE
	@ rm $@ $*.compilium.log &> /dev/null; \
		../compilium -o $@ --prefix_type `uname` -I ../include/`uname` $*.c &> $*.compilium.log \
		|| { echo "FAIL $@"; cat $*.compilium.log; false; }

%.compilium.ast : %.c Makefile ../compilium FORCE
	../compilium --parse_only --prefix_type `uname` $*.c &> $*.compilium.ast; cat $*.compilium.ast

%.bin : %.S Makefile FORCE
	@ gcc -o $@ $*.S

%.test : %.clang.bin %.compilium.bin Makefile FORCE
	@ rm $*.clang.bin.stdout $*.compilium.bin.stdout &> /dev/null; true
	@ ./$*.clang.bin > $*.clang.bin.stdout; expected=$$?; \
		./$*.compilium.bin > $*.compilium.bin.stdout; actual=$$?; \
		if [ $$expected = $$actual ]; then \
		{ diff -u $*.clang.bin.stdout $*.compilium.bin.stdout \
		&& echo "PASS $*.c" \
		|| echo "FAIL $*.c: text diff";\
		} else \
		echo "FAIL $*.c: expected $$expected but got $$actual"; \
		fi

clean:
	-rm *.bin *.stdout *.S *.s *.log

