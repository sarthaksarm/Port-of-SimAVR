#include <avr/io.h>
#include <util/delay.h>

void delay_ms(uint16_t ms) 
{
	
    while ( ms ) 
	{
		_delay_ms(1);
		ms--;
	}
}

int main() 
{a
 
  DDRC |= (1<<PC4);
  DDRC |= (1<<PC3);
  DDRC |= (1<<PC2);
  DDRC |= (1<<PC1);
 
   while(1) {
 
    PORTC |= (1<<PC4);
    delay_ms(100);
    PORTC &= ~(1<<PC4);
    delay_ms(100);
 
    PORTC |= (1<<PC4);
    delay_ms(100);  
    PORTC |= (1<<PC3);
    delay_ms(100);
 
    PORTC &= ~(1<<PC4);
    PORTC &= ~(1<<PC3); 
    delay_ms(100);
 
    PORTC |= (1<<PC2);
    delay_ms(100);
    PORTC &= ~(1<<PC2);
    delay_ms(100);
 
    PORTC |= (1<<PC2);
    delay_ms(100);
    PORTC |= (1<<PC1);
    delay_ms(100);
    PORTC &= ~(1<<PC1);
    PORTC &= ~(1<<PC2);
    delay_ms(100);
  }
 
  return 0;
}