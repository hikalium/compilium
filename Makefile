CFLAGS=-Wall -Wpedantic -std=c11 -Wno-extra-semi -Iinclude/$(shell uname)
SRCS=analyzer.c ast.c context.c error.c generate.c il.c parser.c static.c token.c tokenizer.c type.c
MAIN_SRCS=compilium.c
HEADERS=compilium.h
RUN_TARGET ?= Tests/hello_world
UNIT_TESTS ?= ast token type

UNIT_TEST_TARGETS = $(addsuffix .unittest, $(UNIT_TESTS))

.PHONY: FORCE default

.PRECIOUS: %_test

default: compilium
	@true

FORCE:

compilium: $(MAIN_SRCS) $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -o $@ $(MAIN_SRCS) $(SRCS)

compilium_dbg: $(MAIN_SRCS) $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -g -o $@ $(MAIN_SRCS) $(SRCS)

%_test : %_test.c $(SRCS) $(HEADERS) Makefile FORCE
	@ $(CC) $(CFLAGS) -g -o $@ $(SRCS) $*_test.c

%.unittest : %_test Makefile FORCE
	@ ./$*_test

%.self.c : %.c $(HEADERS) Makefile FORCE
	@ $(CC) $(CFLAGS) -P -E $*.c > $@

%.self.S : %.self.c $(HEADERS) Makefile compilium FORCE
	@ ./compilium -o $@ --prefix_type `uname` $*.self.c

run: compilium
	make -C $(dir $(RUN_TARGET)) $(notdir $(RUN_TARGET)).compilium.bin; \
	cat $(RUN_TARGET).compilium.log
	./$(RUN_TARGET).compilium.bin

debug: compilium_dbg
	lldb -- ./compilium_dbg -o $(RUN_TARGET).compilium.S $(RUN_TARGET).c

test: compilium
	@ make unittest
	@ make -C Tests/ | tee test_result.txt && ! grep FAIL test_result.txt && echo "All tests passed" && rm test_result.txt

unittest: $(UNIT_TEST_TARGETS)

clean:
	-rm compilium

format:
	clang-format -i *.c *.h

commit:
	make format
	make test
	git add .
	git commit
