all: neopixelUsart.hex

neopixelUsart.hex: neopixelUsart.elf
	avr-objcopy  -j .text -j .data -O ihex $^ $@
	avr-size neopixelUsart.elf

neopixelUsart.elf: neopixelUsart.c
	avr-g++ -mmcu=atmega324p -DF_CPU=16000000 -Os -Wall -o $@ $^

clean:
	rm -rf neopixelUsart.elf neopixelUsart.hex
