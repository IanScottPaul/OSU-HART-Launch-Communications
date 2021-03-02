//////////////////////////////////////////////////
// FILE:	control_code.c 			//
// DATE:	Winter 2021			//
// DEVELOPER:	Ian Scott Paul			//
// TEAM:	High Altitude Rocket Team	//
// PROGRAM:	Launch Communications Software	//
// VER.:	CONTROL-0.2			//
// PROJECT:	HART GROUND SYSTEMS		//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// DISCLAIMER: 	The Launch Communications Software is deployed on the 2021 Launch Communications PCBs. 	This software is written to specifically target these boards, but should function for other similar AVR systems with little configuration. See schematic in repository. 
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// USE THE INCLUDED MAKEFILE AND AVR 6 PIN ICSP TO PROGRAM LAUNCH COMMUNICATIONS PCBS //
////////////////////////////////////////////////////////////////////////////////////////
// THE AVR MICROCONTROLLER USED IS THE ATMEGA16U4. 
// CRITICAL NAMED REGISTERES USED IN THIS DESIGN THAT ARE RECOGNISED BY AVR-GCC
// ANY REFERENCE TO DS ARE TO THE DATASHEET FOR THE ATMEGA16U4
//////////////////////////////////////////////////	
//	////DDRB////DDRC////DDRF////		//DATA DIRECTION REGISTERS. BIT SET FOR OUTPUT, CLEAR FOR INPUT
// 	////PORTB///PORTC///PORTF///		//DATA FOR OUTPUT, IF INPUT CONTROLLS PULLMODE, SET PULLUP
//	////PINB////PINC////PINF////		//REGISTER TO READ INPUTS. 
//	////SPDR////SPCR////SPRS////		//SPI DATA REGISTER, SPI CONTROL REGISTER, SPI STATUS REGISTER. 
//	///TCCR1A//TCCR1B//TCCR1C///		//TIMER1 CONTROL REGISTERS [DS:14.10]
//	///TCNT1H//TCNT1L//			//THE COUNT VALUE ON TIMER1
//	///TIMSK1///TIFR1///			//TIMER 1 INTERRUPT MASK AND TIMER 1 INTERUPT FLAG  REGISTER
//////////////////////////////////////////////////
////////INCLUDED LIBRARIES////////
#include <avr/io.h>		//needed to map GPIO
#include <stdbool.h>		//needed for bools
#include <util/delay.h>		//needed for delays, in future versions use Timers
#include <string.h>
////////STATE MACROS////////
#define STATE_LISTENING 0
#define STATE_READY 	2
#define STATE_ARM	8
#define STATE_FIRE 	255
////PORTB MACROS////
#define NSS 		0
////PORTF MACROS////
#define ARM_LED_1	0
#define ARM_LED_2	1
#define ARM_IG_1	4
#define FIRE_IG_1	5
#define ARM_IG_2	6
#define FIRE_IG_2	7
#define LISTEN_REG	((1<<ARM_IG_1)|(1<<FIRE_IG_1))
#define READY_REG	((1<<ARM_IG_1)|(1<<FIRE_IG_1)|(1<<ARM_LED_2))
#define ARM_REG		((1<<FIRE_IG_1)|(1<<ARM_LED_1))
#define FIRE_REG	((1<<ARM_LED_1)|(1<<ARM_LED_2))
////TIMING MACROS////
#define TRANSITON_TIME 	10
#define RX_TIME		100
#define TX_TIME		100

////////FUCNTION DECLARATIONS////////
void tx_when_ready(uint8_t, char, char, char, char);
void set_mode_tx();
void set_mode_rx();
void read_fifo(char*);

////////MAIN FUNCTION DESCRIPTION////////
//as of version 0.2, main holds control uninterrupted.
//main is broken into primary two sections: Initialization and Loop
//in initialization:
//	 the state transition variable is set to IDLE
//	 the character array for recieved messages is initalized. 
//	 the boolean variables for the state machine are created and set false
uint8_t main(){	
	////Program Initialization////
	//START OF MAIN FUNCTION VARIABLE DECLARIATIONS //
		uint8_t i;
		uint8_t	state = STATE_LISTENING;		// state determines the current state of the program, use state macros. 
		char message[4]   = {NULL, NULL, NULL, NULL};			// the four character code transmitted or recieved.  
		char msg_tst_1[4] = {'t','e','s','t'};
		char msg_arm_1[4] = {'a','r','m','1'};
		char msg_fire1[4] = {'f','i','r','e'};
			//address[7] is WNR, adsress[6:0] are the actual address bits. I have chosen to hardcode addresses in funtion calls.  
			//The two critical addresses are 0x00 for FIFO, and 0x01 for the Operation mode register REGOPMODE	
		//boolean variables for state machine. 
		bool communications_enabled 	= false;	
		bool communications_established = false;
		bool arm_sw_debounced		= false;
		bool system_armed		= false;
		bool launch_button_debounced 	= false;
		bool fire_ackd			= false;
	// END OF MAIN FUNCTION VARIABLE DECLARIATIONS  //
/////////BEGIN GIPO INIT//////////
						//See system hardware documentation for GPIO connection
						//DDRX is data direction. 1 is output, 0 is input [DS:10.2]
						//PORTX will set the value to output, or the pullmode of the input. 1 is up 0 is down. 
						//PINX can be used to read the value of the input pins of X
		DDRF	= 0xFF;		// all PORTF pins are outputs. 	
		DDRB 	= 0b00000111;	// PB[2:0] output for SPI pb[4] for fifo level detector
	    	//IOPORT B is X, X, X, X, MISO, MOSI, SCK, NSS
		PORTB 	= (1<<NSS);// set arm sw to be pullup, write pull not slave select high
	       	PORTF	= 0xFF;		//.
//////////END GPIO INIT///////////////////////////
///////////BEGIN SPI INIT////////////////////////////////////////////////////////////////
		////SPRC////[DS:17.2.1]
		//Interupts disabled, SPI enabled, MSB first, Master mode
		//Low clk idle, fosh/64
		SPCR = 0b01010010;	//every bit in this register can be written. 
		SPSR = 0b00000000;	//only bit zero can be written in this reg so im not too worried about clobbering it here. 
		//SPDR is data register, write data to it to transmit, read it twice to recieve data. 
//////////END SPI INIT
		_delay_ms(TRANSITON_TIME);
		set_mode_rx();
		_delay_ms(TRANSITON_TIME);
		_delay_ms(RX_TIME);//time for message to tx, be decoded, and sent back 
	////END OF PROGRAM INITIALIZATION
	//BEGIN ENDLESS WHILE LOOP
	while(1){						
		//BEGIN CASE STATEMENTS FOR STATE MACHINE CONTROL
		switch(state){	//switch case statements for state machine control
			case(STATE_LISTENING): 		//in the idle state
				//PORTF = LISTEN_REG;
				if (bit_is_set(PINB,4)){
					_delay_ms(200);
					PORTB = (PORTB & ~(1<<0));	//nss low for begining of packet transmission. 
					_delay_ms(1);			//time for nss to change #FIXME lower this value
					SPDR = 0b00000000;		//read FIFO address command
					while(!(SPSR & (1<<7))){;}	// this litteraly just does nothing until the spi isnt sending something. maybe make it flash arm2?
					for(i=0;i<4;i++){
						SPDR = 0b00000000;		//read FIFO address command
						while(!(SPSR & (1<<7))){;}	// this
						message[i] = SPDR;
					}
					PORTB |= (1<<0);		//set nss high
					_delay_ms(1);			//time for nss to change #FIXME lower this value
				}
				//if (strcmp(message, msg_tst_1)){	//
				//	state = STATE_ARM;		//
				//}
				//else
				uint8_t i;
				for(i=0; i<4; i++){
				if (message[i] == 't'){		//#FIXME CHANGE THIS IF IT WORKS!
					state = STATE_READY;}	
				else{ ;//state = STATE_LISTENING;	//hold state as IDLE;
				}//else comm deb
				}
			break;
		case(STATE_READY):
				PORTF = READY_REG;	
			break;
		case(STATE_ARM):
				PORTF = ARM_REG;
			break;
		case(STATE_FIRE):

			break;
		default: 
			state = STATE_LISTENING;
			break;
		}//switch control
	}//while_1
}//main
//FUNCTIONS//
//
//
//void tx_when_ready(uint8_t, char, char, char, char);
//this function takes full control of the uC during the SPI tranmission. 
void tx_when_ready(uint8_t address, char c0, char c1, char c2, char c3){//no array for readablility of main
	uint8_t i;
	char word[4];
	word[0] = c0;
	word[1] = c1;
	word[2] = c2;
	word[3] = c3;
	PORTB = (PORTB & ~(1<<0));//nss low for begining of packet transmission. 
	_delay_ms(1);			//time for nss to change #FIXME lower this value
	SPDR = address;
	while(!(SPSR & (1<<7))){;}// this litteraly just does nothing until the spi isnt sending something. maybe make it flash arm2?
	for(i=0;i<4;i++){
		SPDR = word[i];
		while(!(SPSR & (1<<7))){;}// this litteraly just waits until the spi isnt sending something. 
	}
	PORTB |= (1<<0);//set nss high
	_delay_ms(1);			//time for nss to change #FIXME lower this value
}
//anticipated hold time: (FOSC / 64) SCK, MAX TIME is limit of six 1 byte transmissions, minimum of five 1 byte transmissions. 
// 64 * 8 * bytes = approx hold time for transmissions. add in time for other functions. 
//end tx when ready function description


//void set_mode_tx();
void set_mode_tx(){
	PORTB = (PORTB & ~(1<<0));	//nss low for begining of packet transmission. 
	_delay_ms(1);			//time for nss to change #FIXME lower this value
	SPDR = 0b10000001;		//this says write mode address 0x01
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	SPDR = 0b10001100;		//mode register value for transmission mode. 
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	PORTB |= (1<<0);		//set nss high at end of transmission
	_delay_ms(1);			//time for nss to change #FIXME lower this value
}	
//void set_mode_rx();
void set_mode_rx(){
	PORTB = (PORTB & ~(1<<0));	//nss low for begining of packet transmission. 
	_delay_ms(1);			//time for nss to change #FIXME lower this value
	SPDR = 0b10000001;		//this says write mode address 0x01
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	SPDR = 0b10010000;		//mode register value for reciever mode. 
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	PORTB |= (1<<0);		//set nss high at end of transmission
	_delay_ms(1);			//time for nss to change #FIXME lower this value
}	
//void read_fifo(char*);
void read_fifo(char* current_char){	//no array for readablility of main
	uint8_t i;
	char word[4];
	PORTB = (PORTB & ~(1<<0));	//nss low for begining of packet transmission. 
	_delay_ms(1);			//time for nss to change #FIXME lower this value
	SPDR = 0b00000000;		//read FIFO address command
	while(!(SPSR & (1<<7))){;}	// this litteraly just does nothing until the spi isnt sending something. maybe make it flash arm2?
	word[0] = SPDR; 
	while(!(SPSR & (1<<7))){;}// this litteraly just waits until the spi isnt sending something. 
	for(i=0;i<4;i++){
		word[i] = SPDR;
		while(!(SPSR & (1<<7))){;}// this litteraly just waits until the spi isnt sending something. 
	}
	PORTB |= (1<<0);		//set nss high
	_delay_ms(1);			//time for nss to change #FIXME lower this value
	for(i=0;i<1;i++){
	*(current_char + i) = word[i];
	}	
}

