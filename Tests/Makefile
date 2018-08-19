SHELL=/bin/bash
TESTS ?= $(basename $(wildcard *.c))
CFLAGS=-std=c11 -Wall -Wpedantic -I../include/$(shell uname)
COMPILIUM?=../compilium

default:
	@ echo 'Testing $(COMPILIUM)...'
	@ make expr_test
	@ make stmt_test
	@ make src_test
	@ make c_test

FORCE:
.PHONY: FORCE

.SECONDARY:

expr_test : $(COMPILIUM) FORCE
	@./test_expressions.sh $(COMPILIUM)

stmt_test : $(COMPILIUM) FORCE
	@./test_statements.sh $(COMPILIUM)

src_test : $(COMPILIUM) FORCE
	@./test_source.sh $(COMPILIUM)

c_test : $(COMPILIUM) $(addsuffix .test, $(TESTS)) FORCE

%.clang.bin : %.c Makefile
	@ rm $@ $*.compilium.log &> /dev/null; \
		{ $(CC) $(CFLAGS) -o $@ $*.c &> $*.clang.log; } \
		|| { echo "FAIL $@"; cat $*.clang.log; false; }

$(COMPILIUM) : FORCE
	@ make -C .. $(notdir $(COMPILIUM)).target

%.clang.S : %.c Makefile FORCE
	$(CC) $(CFLAGS) -O0 -S -o $*.clang.S $*.c

%.compilium.S : %.c Makefile $(COMPILIUM) FORCE
	@ rm $@ $*.compilium.log &> /dev/null; \
		$(COMPILIUM) -o $@ --prefix_type `uname` -I ../include/`uname` $*.c &> $*.compilium.log \
		|| { echo "FAIL $@"; cat $*.compilium.log; false; }

%.compilium.ast : %.c Makefile $(COMPILIUM) FORCE
	$(COMPILIUM) --parse_only --prefix_type `uname` $*.c &> $*.compilium.ast; cat $*.compilium.ast

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
	@ find . -type f \( \
		-name '*.S' -o \
		-name '*.bin' -o \
		-name '*.bin' -o \
		-name '*.log' -o \
		-name '*.ast' -o \
		-name '*.stdout' \
		\) -exec rm {} +

