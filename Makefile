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
	git add .
	git diff HEAD --color=always | less -R
	make testall
	git commit
