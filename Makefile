CFLAGS=-Wall -Wpedantic -std=c11 -Wno-extra-semi
SRCS=ast.c error.c generate.c il.c parser.c token.c tokenizer.c
MAIN_SRCS=compilium.c
HEADERS=compilium.h

default: compilium

compilium: $(MAIN_SRCS) $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -o $@ $(MAIN_SRCS) $(SRCS)

compilium_dbg: $(MAIN_SRCS) $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -g -o $@ $(MAIN_SRCS) $(SRCS)

compilium_unittest: unittest.c $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -o $@ unittest.c $(SRCS)

run: compilium
	make -C Tests sample.compilium.bin; \
	cat Tests/sample.compilium.log

debug: compilium_dbg
	lldb ./compilium_dbg Tests/sample.c Tests/sample.compilium.S

test: compilium
	make -C Tests/

unittest: compilium_unittest
	./compilium_unittest

clean:
	-rm compilium

format:
	clang-format -i *.c *.h
