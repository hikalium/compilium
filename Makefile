CFLAGS=-Wall -Wpedantic
SRCS=compilium.c

default: compilium

compilium: $(SRCS) Makefile
	$(CC) $(CFLAGS) -o $@ $(SRCS)

run: compilium
	./compilium compilium.c
