default : calc.bin ctests.bin gameoflife.bin hello.bin pi.bin fib.bin

.FORCE : 

run_ctests : ctests.bin ../compilium
	make run_ctests_hostcc
	./ctests.bin

run_ctests_hostcc : ctests.host.bin
	./ctests.host.bin

validate : run_ctests_hostcc

../compilium : .FORCE
	make -C .. compilium

../compilium_dbg : .FORCE
	make -C .. compilium_dbg

constsum.c : gen_constsum.js
	node gen_constsum.js > $@

%.host.bin : %.c Makefile
	$(CC) -o $*.host.bin $*.c

%.host_o3.bin : %.c Makefile
	$(CC) -O3 -o $*.host_o3.bin $*.c

LLDB_ARGS = -o 'settings set interpreter.prompt-on-quit false' \
			-o 'b __assert' \
			-o 'b ErrorWithToken' \
			-o 'process launch'

debug_% : ../compilium_dbg %.c
	lldb \
		-o 'settings set target.input-path $*.c' $(LLDB_ARGS) \
		-- ../compilium_dbg --target-os `uname`

%.host.S : %.c Makefile ../compilium .FORCE
	$(CC) -S -o $@ $*.c

%.S : %.c Makefile ../compilium .FORCE
	../compilium --target-os `uname` -I ../include/ < $*.c > $*.S

format:
	clang-format -i *.c

%.bin : %.S Makefile
	$(CC) -o $@ $*.S

clean:
	-rm *.bin
	-rm *.S
