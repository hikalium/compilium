CFLAGS=-Wall -Wpedantic
SRCS=compilium.c error.c token.c

default: compilium

compilium: $(SRCS) Makefile
	$(CC) $(CFLAGS) -o $@ $(SRCS)

compilium_dbg: $(SRCS) Makefile
	$(CC) $(CFLAGS) -g -o $@ $(SRCS)

run: compilium
	make -C Tests test.compilium.S

debug: compilium_dbg
	lldb ./compilium_dbg Tests/test.c

clean:
	-rm compilium
