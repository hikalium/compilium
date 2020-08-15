CFLAGS=-Wall -Wpedantic -Wextra -Werror -Wconditional-uninitialized -std=c11
SRCS=analyzer.c ast.c compilium.c generator.c \
		 parser.c preprocessor.c struct.c symbol.c token.c tokenizer.c type.c
HEADERS=compilium.h
CC=clang
FAILCASE_FILE:=failcase.c
LLDB_ARGS = -o 'settings set interpreter.prompt-on-quit false' \
			-o 'b __assert' \
			-o 'process launch'

.FORCE :

%.compilium.S : compilium %.c .FORCE
	cat $*.c | \
		./compilium -I include/ --target-os `uname` > $*.compilium.S

compilium : $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -o $@ $(SRCS) 

compilium_dbg : $(SRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -g -o $@ $(SRCS)

debug : compilium_dbg failcase.c
	lldb \
		-o 'settings set target.input-path ${FAILCASE_FILE}' $(LLDB_ARGS) \
		-- ./compilium_dbg -I include/ --target-os `uname`

testall : unittest ctest test linkage_test
	make -C examples

test_preprocess : compilium
	./test_preprocess.sh

test : compilium
	time ./test.sh

ctest : compilium
	make -C examples run_ctests

linkage_test : compilium
	make -C linkage_test test

unittest : run_unittest_List run_unittest_Type

run_unittest_% : compilium
	@ ./compilium --run-unittest=$* || { echo "FAIL unittest.$*: Run 'make dbg_unittest_$*' to rerun this testcase with debugger"; exit 1; }

dbg_unittest_% : compilium_dbg
	lldb $(LLDB_ARGS)\
		-- ./compilium_dbg --run-unittest=$*

format:
	clang-format -i $(SRCS) $(HEADERS)
	make -C examples format

commit:
	make format
	git add .
	git diff HEAD --color=always | less -R
	make testall
	git commit

clean:
	-rm -r compilium compilium_dbg
