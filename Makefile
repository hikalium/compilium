CFLAGS=-Wall -Wpedantic
SRCS=compilium.c

default: compilium

compilium: $(SRCS) Makefile
	$(CC) $(CFLAGS) -o $@ $(SRCS)

compilium_dbg: $(SRCS) Makefile
	$(CC) $(CFLAGS) -g -o $@ $(SRCS)

run: compilium
	./compilium test.c

debug: compilium_dbg
	lldb ./compilium_dbg test.c
