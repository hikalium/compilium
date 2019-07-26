default : hello.bin

run_stmt_tests : stmt_tests.bin
	./stmt_tests.bin

run_stmt_tests_hostcc : stmt_tests.host.bin
	./stmt_tests.host.bin

../compilium:
	make -C ..

%.host.bin : %.c Makefile
	$(CC) -o $*.host.bin $*.c

%.S : %.c Makefile ../compilium
	../compilium < $*.c > $*.S

%.bin : %.S Makefile
	clang -o $@ $*.S

clean:
	-rm *.bin
