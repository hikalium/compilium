CFLAGS=-Wall -Wpedantic -std=c11
SRCS=ast.c compilium.c parser.c
HEADERS=compilium.h

compilium : $(SRCS) $(HEADERS) Makefile
	cc $(CFLAGS) -o $@ $(SRCS) 

compilium_dbg : $(SRCS) $(HEADERS) Makefile
	cc $(CFLAGS) -g -o $@ $(SRCS)

debug : compilium_dbg failcase.c
	lldb \
		-o 'settings set interpreter.prompt-on-quit false' \
		-o 'b __assert' \
		-o 'process launch' \
		-- ./compilium_dbg --target-os `uname` "`cat failcase.c`"

testall : unittest test

test : compilium
	./test.sh

unittest : compilium
	@ ./compilium --run-unittest=List
	@ ./compilium --run-unittest=Type

format:
	clang-format -i $(SRCS) $(HEADERS)

commit:
	make format
	git add .
	git diff HEAD --color=always | less -R
	make testall
	git commit
