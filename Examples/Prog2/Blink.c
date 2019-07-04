#include <avr/io.h>
#include <avr/delay.h>

void delay_ms(uint16_t ms) {
	
    while ( ms ) 
	{
		_delay_ms(1);
		ms--;
	}
}

int main (void)
{
//Set PORTC to all outputs
DDRC = 0xFF;
//create an infinite loop
while(1) {

PORTC |=(1<<0);
//PAUSE 250 miliseconds
delay_ms(250);
//turns C0 LOW
PORTC &= ~(1 << 0);
//PAUSE 250 miliseconds
delay_ms(250);
}

}