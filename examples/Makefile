default : hello.bin

run_ctests : ctests.bin ../compilium
	./ctests.bin

run_ctests_hostcc : ctests.host.bin
	./ctests.host.bin

../compilium:
	make -C ..

%.host.bin : %.c Makefile
	$(CC) -o $*.host.bin $*.c

%.S : %.c Makefile ../compilium
	../compilium --target-os `uname` < $*.c > $*.S

%.bin : %.S Makefile
	$(CC) -o $@ $*.S

clean:
	-rm *.bin
	-rm *.S
