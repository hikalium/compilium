CFLAGS=-Wall -Wpedantic -std=c11
SRCS=compilium.c

compilium : $(SRCS) Makefile
	cc $(CFLAGS) -o $@ $(SRCS) 

testall : unittest test

test : compilium
	./test.sh

unittest : compilium
	@ ./compilium --run-unittest=List
	@ ./compilium --run-unittest=Type

format:
	clang-format -i $(SRCS)

commit:
	make format
	make testall
	git add .
	git diff HEAD
	git commit
