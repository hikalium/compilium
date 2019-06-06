default : hello.bin

%.S : %.c Makefile ../compilium
	../compilium < $*.c > $*.S

%.bin : %.S Makefile
	clang -o $@ $*.S
