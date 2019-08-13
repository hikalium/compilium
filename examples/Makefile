default : hello.bin

.FORCE : 

run_ctests : ctests.bin ../compilium
	./ctests.bin

run_ctests_hostcc : ctests.host.bin
	./ctests.host.bin

validate : run_ctests_hostcc

../compilium : .FORCE
	make -C ..

%.host.bin : %.c Makefile
	$(CC) -o $*.host.bin $*.c

%.S : %.c Makefile ../compilium .FORCE
	../compilium --target-os `uname` < $*.c > $*.S

%.bin : %.S Makefile
	$(CC) -o $@ $*.S

clean:
	-rm *.bin
	-rm *.S
