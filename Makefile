CFLAGS=-Wall -Wpedantic -std=c11 -Wno-extra-semi
SRCS=ast.c error.c generate.c il.c parser.c token.c tokenizer.c
MAIN_SRCS=compilium.c
HEADERS=compilium.h
RUN_TARGET ?= Tests/hello_world
UNIT_TESTS ?= ast token

UNIT_TEST_TARGETS = $(addsuffix .unittest, $(UNIT_TESTS))

.PHONY: FORCE default

default: compilium
	@true

FORCE:

compilium: $(MAIN_SRCS) $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -o $@ $(MAIN_SRCS) $(SRCS)

compilium_dbg: $(MAIN_SRCS) $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -g -o $@ $(MAIN_SRCS) $(SRCS)

%.unittest : %_test.c $(SRCS) $(HEADERS) Makefile FORCE
	@ $(CC) $(CFLAGS) -o $@ $(SRCS) $*_test.c
	@ ./$@

run: compilium
	make -C $(dir $(RUN_TARGET)) $(notdir $(RUN_TARGET)).compilium.bin; \
	cat $(RUN_TARGET).compilium.log
	./$(RUN_TARGET).compilium.bin

debug: compilium_dbg
	lldb -- ./compilium_dbg -o $(RUN_TARGET).compilium.S $(RUN_TARGET).c

test: compilium
	@ make unittest
	@ make -C Tests/ | tee test_result.txt && ! grep FAIL test_result.txt  && echo "All tests passed"

unittest: $(UNIT_TEST_TARGETS)

clean:
	-rm compilium

format:
	clang-format -i *.c *.h
