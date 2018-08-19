CFLAGS=-Wall -Wpedantic -std=c11 -Wno-extra-semi -Iinclude/$(shell uname)
SRCS=analyzer.c ast.c context.c error.c generate.c il.c misc.c parser.c reg.c static.c token.c tokenizer.c type.c
MAIN_SRCS=compilium.c
HEADERS=compilium.h
RUN_TARGET ?= Tests/hello_world
UNIT_TESTS ?= ast token type

SELF_HOST_OBJS = analyzer.o ast.o context.o error.o generate.o il.o misc.self.o parser.o reg.o static.self.o token.o tokenizer.o type.self.o compilium.self.o

UNIT_TEST_TARGETS = $(addsuffix .unittest, $(UNIT_TESTS))

.PHONY: FORCE default

.PRECIOUS: %_test %.self.c

compilium.target : compilium
	@true

compilium_self.target : compilium
	@true

FORCE:

compilium: $(MAIN_SRCS) $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -o $@ $(MAIN_SRCS) $(SRCS)

compilium_dbg: $(MAIN_SRCS) $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -g -o $@ $(MAIN_SRCS) $(SRCS)

compilium_self: $(SELF_HOST_OBJS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -g -o $@ $(SELF_HOST_OBJS)

%_test : %_test.c $(SRCS) $(HEADERS) Makefile FORCE
	@ $(CC) $(CFLAGS) -g -o $@ $(SRCS) $*_test.c

%.unittest : %_test Makefile FORCE
	@ ./$*_test

%.preprocess.c : %.c $(HEADERS) Makefile
	@ $(CC) $(CFLAGS) -P -E $*.c > $@

%.self.S : %.preprocess.c $(HEADERS) Makefile compilium
	./compilium -o $@ --prefix_type `uname` $*.preprocess.c > $*.self.log

%.self.o : %.self.S Makefile
	@ as -o $@ $*.self.S

run: compilium
	make -C $(dir $(RUN_TARGET)) $(notdir $(RUN_TARGET)).compilium.bin; \
	cat $(RUN_TARGET).compilium.log
	./$(RUN_TARGET).compilium.bin

debug: compilium_dbg
	lldb -- ./compilium_dbg -o $(RUN_TARGET).compilium.S $(RUN_TARGET).c

test_all: test test_self

test: compilium
	@ make unittest
	@ make -C Tests/ | tee test_result.txt && ! grep FAIL test_result.txt && echo "All tests passed" && rm test_result.txt

test_self: compilium_self
	@ COMPILIUM=../compilium_self \
		make -C Tests/ | tee test_result.txt && ! grep FAIL test_result.txt && echo "All tests passed" && rm test_result.txt

unittest: $(UNIT_TEST_TARGETS)

clean:
	@ rm compilium *.S *.preprocess.c *.o *.log ; true

format:
	clang-format -i *.c *.h

commit:
	make format
	make test_all
	git add .
	git commit
