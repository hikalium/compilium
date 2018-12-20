CFLAGS=-Wall -Wpedantic -std=c11
SRCS=compilium.c

compilium : $(SRCS) Makefile
	cc $(CFLAGS) -o $@ $(SRCS) 

compilium_dbg : $(SRCS) Makefile
	cc $(CFLAGS) -g -o $@ $(SRCS)

debug : compilium_dbg failcase.c
	lldb -o 'process launch' -- ./compilium_dbg --target-os `uname` "`cat failcase.c`"

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
