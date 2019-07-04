#include <avr/io.h>
#include <util/delay.h>

void delay_ms(uint16_t ms) {
	
    while ( ms ) 
	{
		_delay_ms(1);
		ms--;
	}
}

int main (void)
{
//Set all Ports high
DDRA  = 0XFF;
DDRB = 0XFF;
DDRC = 0xFF;
DDRD = 0XFF;
DDRE = 0XFF;
DDRF = 0xFF;
DDRG = 0XFF;
DDRH = 0XFF;
DDRJ = 0xFF;
DDRK = 0XFF;
DDRL = 0XFF;

delay_ms(250);
//create an infinite loop
while(1) {

PORTA = 0XFF;
PORTB = 0XFF;
PORTC = 0XFF;
PORTD = 0XFF;
PORTE = 0XFF;
PORTF = 0XFF;
PORTG = 0XFF;
PORTH = 0XFF;
PORTJ = 0XFF;
PORTK = 0XFF;
PORTL = 0XFF;

delay_ms(250);

//turn LOW with 250ms delay.

PORTA = 0x00;
PORTB = 0x00;
PORTC = 0X00;
PORTD = 0X00;
PORTE = 0X00;
PORTF = 0X00;
PORTG = 0X00;
PORTH = 0X00;
PORTJ = 0X00;
PORTK = 0X00;
PORTL = 0X00;

//PAUSE 250 miliseconds
delay_ms(250);
}

}