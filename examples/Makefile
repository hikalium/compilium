default : hello.bin

.FORCE : 

run_ctests : ctests.bin ../compilium
	./ctests.bin

run_ctests_hostcc : ctests.host.bin
	./ctests.host.bin

validate : run_ctests_hostcc

../compilium : .FORCE
	make -C .. compilium

../compilium_dbg : .FORCE
	make -C .. compilium_dbg

%.host.bin : %.c Makefile
	$(CC) -o $*.host.bin $*.c

LLDB_ARGS = -o 'settings set interpreter.prompt-on-quit false' \
			-o 'b __assert' \
			-o 'process launch'

debug_ctests : ../compilium_dbg ctests.c
	lldb \
		-o 'settings set target.input-path ctests.c' $(LLDB_ARGS) \
		-- ../compilium_dbg --target-os `uname`

%.S : %.c Makefile ../compilium .FORCE
	../compilium --target-os `uname` < $*.c > $*.S

%.bin : %.S Makefile
	$(CC) -o $@ $*.S

clean:
	-rm *.bin
	-rm *.S
