#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
bool send(char);
int main(){
	DDRB 	= 0b00000111;
	DDRF 	= 0XFF;
	_delay_ms(100);
	PORTF 	= 0b11111111;
	PORTB   = 0b11111111; 
	bool looping = true;
	SPCR 	= 0b01010010;
	SPSR 	= (1<<7);
	char a = 'a';
		//send(a);
		_delay_ms(100);
		PORTB = 0x00;
		SPDR = a;
		while(1){
		if (bit_is_set(SPSR,7)){		
		SPDR = a;
		SPSR = SPSR & ~(1<<7);
	}
		PORTF = 0XFF;
		_delay_ms(100);}

}//main

bool send(char totx){
		if (bit_is_set(SPSR,7)){		
			SPDR = totx;
		PORTF = ~PORTF;	
		//NSS HIgh	
		return true;
		}	
		else{
			//NSS Low
			return false;}
}
