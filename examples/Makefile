default : hello.bin

../compilium:
	make -C ..

%.S : %.c Makefile ../compilium
	../compilium < $*.c > $*.S

%.bin : %.S Makefile
	clang -o $@ $*.S

clean:
	-rm *.bin
