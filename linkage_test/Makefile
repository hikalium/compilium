SRCS=linkage_test.c external.c
ASMS=$(addsuffix .S, $(basename $(SRCS)))

default: test

.FORCE : 

test: 
	make validate
	make run

run: linkage_test.bin
	./linkage_test.bin

validate: linkage_test.host.bin
	./linkage_test.host.bin

format:
	clang-format -i *.c

linkage_test.host.bin : $(SRCS) .FORCE
	$(CC) -Wall -pedantic -o $@ ${SRCS}

linkage_test.bin : $(ASMS) .FORCE
	$(CC) -Wall -pedantic -o $@ ${ASMS}

../compilium : .FORCE
	make -C .. compilium

%.host.S : %.c Makefile ../compilium .FORCE
	$(CC) -S -o $@ $*.c

%.S : %.c Makefile ../compilium .FORCE
	../compilium --target-os `uname` -I ../include/ < $*.c > $*.S

clean:
	-rm *.bin
	-rm *.S
