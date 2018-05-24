/* This line specifies the frequency your AVR is running at.
   This code supports 20 MHz, 16 MHz and 8MHz */
#define F_CPU 20000000

// These lines specify what pin the LED strip is on.
// You will either need to attach the LED strip's data line to PC0 or change these
// lines to specify a different pin.
#define LED_STRIP_PORT PORTC
#define LED_STRIP_DDR  DDRC
#define LED_STRIP_PIN  0


#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

/** The rgb_color struct represents the color for an 8-bit RGB LED.
    Examples:
      Black:      (rgb_color){ 0, 0, 0 }
      Pure red:   (rgb_color){ 255, 0, 0 }
      Pure green: (rgb_color){ 0, 255, 0 }
      Pure blue:  (rgb_color){ 0, 0, 255 }
      White:      (rgb_color){ 255, 255, 255} */
typedef struct rgb_color
{
  unsigned char red, green, blue;
} rgb_color;

/** led_strip_write sends a series of colors to the LED strip, updating the LEDs.
 The colors parameter should point to an array of rgb_color structs that hold the colors to send.
 The count parameter is the number of colors to send.
 This function takes about 1.1 ms to update 30 LEDs.
 Interrupts must be disabled during that time, so any interrupt-based library
 can be negatively affected by this function.
 Timing details at 20 MHz (the numbers slightly different at 16 MHz and 8MHz):
  0 pulse  = 400 ns
  1 pulse  = 850 ns
  "period" = 1300 ns
 */
void __attribute__((noinline)) led_strip_write(rgb_color * colors, unsigned int count) 
{
  // Set the pin to be an output driving low.
  LED_STRIP_PORT &= ~(1<<LED_STRIP_PIN);
  LED_STRIP_DDR |= (1<<LED_STRIP_PIN);

  cli();   // Disable interrupts temporarily because we don't want our pulse timing to be messed up.
  while(count--)
  {
    // Send a color to the LED strip.
    // The assembly below also increments the 'colors' pointer,
    // it will be pointing to the next color at the end of this loop.
    asm volatile(
        "ld __tmp_reg__, %a0+\n"
        "ld __tmp_reg__, %a0\n"
        "rcall send_led_strip_byte%=\n"  // Send red component.
        "ld __tmp_reg__, -%a0\n"
        "rcall send_led_strip_byte%=\n"  // Send green component.
        "ld __tmp_reg__, %a0+\n"
        "ld __tmp_reg__, %a0+\n"
        "ld __tmp_reg__, %a0+\n"
        "rcall send_led_strip_byte%=\n"  // Send blue component.
        "rjmp led_strip_asm_end%=\n"     // Jump past the assembly subroutines.

        // send_led_strip_byte subroutine:  Sends a byte to the LED strip.
        "send_led_strip_byte%=:\n"
        "rcall send_led_strip_bit%=\n"  // Send most-significant bit (bit 7).
        "rcall send_led_strip_bit%=\n"
        "rcall send_led_strip_bit%=\n"
        "rcall send_led_strip_bit%=\n"
        "rcall send_led_strip_bit%=\n"
        "rcall send_led_strip_bit%=\n"
        "rcall send_led_strip_bit%=\n"
        "rcall send_led_strip_bit%=\n"  // Send least-significant bit (bit 0).
        "ret\n"

        // send_led_strip_bit subroutine:  Sends single bit to the LED strip by driving the data line
        // high for some time.  The amount of time the line is high depends on whether the bit is 0 or 1,
        // but this function always takes the same time (2 us).
        "send_led_strip_bit%=:\n"
#if F_CPU == 8000000
        "rol __tmp_reg__\n"                      // Rotate left through carry.
#endif
        "sbi %2, %3\n"                           // Drive the line high.

#if F_CPU != 8000000
        "rol __tmp_reg__\n"                      // Rotate left through carry.
#endif

#if F_CPU == 16000000
        "nop\n" "nop\n"
#elif F_CPU == 20000000
        "nop\n" "nop\n" "nop\n" "nop\n"
#elif F_CPU != 8000000
#error "Unsupported F_CPU"
#endif

        "brcs .+2\n" "cbi %2, %3\n"              // If the bit to send is 0, drive the line low now.

#if F_CPU == 8000000
        "nop\n" "nop\n"
#elif F_CPU == 16000000
        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
#elif F_CPU == 20000000
        "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
        "nop\n" "nop\n"
#endif

        "brcc .+2\n" "cbi %2, %3\n"              // If the bit to send is 1, drive the line low now.

        "ret\n"
        "led_strip_asm_end%=: "
        : "=b" (colors)
        : "0" (colors),         // %a0 points to the next color to display
          "I" (_SFR_IO_ADDR(LED_STRIP_PORT)),   // %2 is the port register (e.g. PORTC)
          "I" (LED_STRIP_PIN)     // %3 is the pin number (0-8)
    );

    // Uncomment the line below to temporarily enable interrupts between each color.
    //sei(); asm volatile("nop\n"); cli();
  }
  sei();          // Re-enable interrupts now that we are done.
  _delay_us(80);  // Send the reset signal.
}

#define LED_COUNT 38
rgb_color colors[LED_COUNT];

/*
 * Functie de initializare a controllerului USART
 */
void USART0_init()
{
    /* seteaza baud rate la 9600 */
    UBRR0H = 0;
    UBRR0L = 103;

    /* porneste transmitatorul */
    UCSR0B = (1<<TXEN0) | (1<<RXEN0);

    /* seteaza formatul frame-ului: 8 biti de date, 1 biti de stop, paritate para */
    UCSR0C &= ~(1<<USBS0);
    UCSR0C |= (2<<UCSZ00);
    UCSR0C |= (1<<UPM01);
}

/*
 * Functie ce transmite un caracter prin USART
 *
 * @param data - caracterul de transmis
 */
void USART0_transmit(char data)
{
    /* asteapta pana bufferul e gol */
    while(!(UCSR0A & (1<<UDRE0)));

    /* pune datele in buffer; transmisia va porni automat in urma scrierii */
    UDR0 = data;
}

/*
 * Functie ce primeste un caracter prin USART
 *
 * @return - caracterul primit
 */
char USART0_receive()
{
    /* asteapta cat timp bufferul e gol */
    while(!(UCSR0A & (1<<RXC0)));

    /* returneaza datele din buffer */
    return UDR0;
}

/*
 * Functie ce transmite un sir de caractere prin USART
 *
 * @param data - sirul (terminat cu '\0') de transmis
 */
void USART0_print(const char *data)
{
    while(*data != '\0')
        USART0_transmit(*data++);
}

/*
 * Functie ce transmite un caracter prin USART
 *
 * @param data - caracterul de transmis
 * @param stream - nefolosit
 *
 * @return - intotdeauna returneaza 0
 */
int USART0_printf(char data, FILE *stream)
{
    /* asteapta pana bufferul e gol */
    while(!(UCSR0A & (1<<UDRE0)));

    /* pune datele in buffer; transmisia va porni automat in urma scrierii */
    UDR0 = data;
    return 0;
}

int main()
{
    DDRB &= ~(1 << PB7);
    //Pin PB7 pus ca intrare pentru senzorul de lumina
    unsigned char c1,c2,c3;
    int i;
	char str[6];
	str[0] = '\n';
    while(1){
    
        USART0_init();
        if(PINB & (1 << PB7)){
            // USART0_print("Intuneriic!\n");
			for(i = 0; i < LED_COUNT; i++){
				c1 = USART0_receive();
                c2 = USART0_receive();
                c3 = USART0_receive();
				// str[0] = c1 + 5;
				// str[1] = c2 +5;
				// str[2] = c3 + 5;
				// str[3] = '\0';
				// USART0_print(str);
				str[0] = '\0';
                colors[i] = (rgb_color){ c1, c2, c3};
            }
		}
        else{
            // USART0_print("Luminaa!\n");
            for(i = 0; i < LED_COUNT; i++){
            

                colors[i] = (rgb_color){ 0, 0, 0};
            }
        }
        led_strip_write(colors, LED_COUNT);
        _delay_ms(100);
    }
    return 0;
}