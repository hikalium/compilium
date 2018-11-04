CFLAGS=-Wall -Wpedantic -std=c11
SRCS=compilium.c

compilium : $(SRCS) Makefile
	cc $(CFLAGS) -o $@ $(SRCS) 

test : compilium
	./test.sh

format:
	clang-format -i $(SRCS)

commit:
	make format
	make test
	git add .
	git commit
