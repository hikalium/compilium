CFLAGS=-Wall -Wpedantic
SRCS=ast.c error.c generate.c parser.c token.c tokenizer.c
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
	make -C Tests sample.compilium.bin

debug: compilium_dbg
	lldb ./compilium_dbg Tests/test.c

test: compilium
	make -C Tests/

unittest: compilium_unittest
	./compilium_unittest

clean:
	-rm compilium

format:
	clang-format -i *.c *.h
