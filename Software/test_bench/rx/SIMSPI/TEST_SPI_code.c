#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
char inbox();
int main(){
	SPCR 	= 0b01010111;
	SPSR 	= 0x00;
	DDRC 	= 0x00;
	DDRB 	= 7;
	DDRF 	= 0XFF;
	PORTC 	= (1<<7);
	PORTB 	= (1<<6);
	PORTF 	= 0b11111100;
	_delay_ms(100);
	uint8_t totx = 0;
	char    fromtx;
	uint8_t message = 0;
	bool progd = false;
	char for_opmode[2];
		for_opmode[0] = 0x01;		//address opmode
		for_opmode[1] = 0b00010000;	//rx mode , rx is 0b00010000, standby is 0b00000100;
	while(1){
		if(!progd){
			totx = for_opmode[message];
			if (bit_is_set(SPSR,7)){
				if(message > 1){progd = true; message = 0; PORTB |= (1<<0);}
				else{
					SPDR = totx;		
					message++;
				}
			}//if send
		}//if not programed
		else{
			fromtx = inbox();
			if(fromtx != 0){
				PORTF = 0xff;
			}
			else{PORTF = PORTF;}
		}
	_delay_us(200);
	}//while
	return 0;
}//main

char inbox(){
		PORTB = (PORTB&(~(1<<0)));// drive NSS low
		if (bit_is_set(SPSR,7)){		
			return SPDR;
		}	
		else{return 0;}
}
