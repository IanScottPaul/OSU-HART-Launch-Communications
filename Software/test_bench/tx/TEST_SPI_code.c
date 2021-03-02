#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
bool send(char);
int main(){
	SPCR 	= 0b01010111;
	SPSR 	= (1<<7);
	DDRC 	= 0x00;
	DDRB 	= 7;
	DDRF 	= 0XFF;
	PORTC 	= (1<<7);
	PORTB 	= (1<<6);
	PORTF 	= 0b11111100;
	_delay_ms(100);
	PORTF 	= 0b11111111;
	uint8_t totx = 0;
	uint8_t message = 0;
	bool progd = false;
	char for_fifo[5];
	char for_opmode[2];
		for_fifo[0] = 0x00;		//fifo address
		for_fifo[1] = 'F';
		for_fifo[2] = 'I';
		for_fifo[3] = 'R';
		for_fifo[4] = 'E';	
		for_opmode[0] = 0x01;		//address opmode
		for_opmode[1] = 0b00001100;	//tx mode , rx is 0b00010000, standby is 0b00000100;
	bool looping = true;
	while(looping){
		if(!progd){
			totx = for_opmode[message];
			if(send(totx)){
				message++;
				if(message < 1){progd = true; message = 0; PORTB |= (1<<0);}
			}//if send
		}//if not programed
		else{
			totx = for_fifo[message];
			if(send(totx)){message ++;}
			if(message >= 5){PORTB |= (1<<0); message = 0; looping = false; PORTF = 0b11111100;}
			else{
			PORTB = (PORTB&(~(1<<0)));// drive NSS low
			}
		}

	_delay_us(200);
	}//while
	_delay_ms(200);
	return 0;
}//main

bool send(char totx){
		
		if (bit_is_set(SPSR,7)){		
			SPDR = totx;
		PORTF 	= 0b11111100;	
			return true;

		}	
		else{return false;}
}
