CFLAGS=-Wall -Wpedantic -Wextra -Wconditional-uninitialized -std=c11
SRCS=ast.c compilium.c parser.c type.c
HEADERS=compilium.h
CC=clang

compilium : $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -o $@ $(SRCS) 

compilium_dbg : $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -g -o $@ $(SRCS)

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

clean:
	-rm compilium
