TARGET=calc

default: $(TARGET).compilium.bin

run : $(TARGET).compilium.bin
	./$(TARGET).compilium.bin

verify : $(TARGET).bin
	./$(TARGET).bin
	@echo "OK"

$(TARGET).compilium.bin : $(TARGET).c Makefile
	make -C ../../
	cat $(TARGET).c | ../../compilium > $(TARGET).compilium.S
	gcc -o $(TARGET).compilium.bin $(TARGET).compilium.S

$(TARGET).bin : $(TARGET).c Makefile
	$(CC) -Wall -o $@ $(TARGET).c

