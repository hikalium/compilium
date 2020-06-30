TARGET=calc

default: $(TARGET).compilium.bin

compilium:
	make -C ../../

run : $(TARGET).compilium.bin
	./$(TARGET).compilium.bin

verify : $(TARGET).bin
	./$(TARGET).bin
	@echo "OK"

$(TARGET).compilium.bin : $(TARGET).c Makefile compilium
	cat $(TARGET).c | ../../compilium -I ./ > $(TARGET).compilium.S
	gcc -o $(TARGET).compilium.bin $(TARGET).compilium.S

$(TARGET).bin : $(TARGET).c Makefile
	$(CC) -Wall -o $@ $(TARGET).c

