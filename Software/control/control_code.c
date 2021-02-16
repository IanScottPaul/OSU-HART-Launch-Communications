//////////////////////////////////////////////////
// FILE:	control_code.c 			//
// DATE:	Winter 2021			//
// DEVELOPER:	Ian Scott Paul			//
// TEAM:	High Altitude Rocket Team	//
// PROGRAM:	Launch Communications Software	//
// VER.:	CONTROL-0.1			//
// PROJECT:	HART GROUND SYSTEMS		//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// DISCLAIMER: 	The Launch Communications Software is deployed on the 2021 Launch Communications PCBs. 	This software is written to specifically target these boards, but should function for other similar AVR systems with little configuration. See schematic in repository. 
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// USE THE INCLUDED MAKEFILE AND AVR 6 PIN ICSP TO PROGRAM LAUNCH COMMUNICATIONS PCBS //
////////////////////////////////////////////////////////////////////////////////////////
////INCLUDED LIBRARIES////
#include <avr/io.h>

////MACROS////
#define state_idle 	0
#define state_esablish	1
#define state_arm_ready 2
#define state_arming	4
#define state_armed!	8
#define state_launch	16
////ALT. DATA TYPES////


////FUCNTION DECLARATIONS////
void CONTROL_GPIO_INIT();

////INTERRUPT VECTORS////




uint8_t main(){	
	////Program Initialization////
	//START OF MAIN FUNCTION VARIABLE DECLARIATIONS //
		uint8_t state = state_idle;		// state determines the current state of the program, use state macros. 
	// END OF MAIN FUNCTION VARIABLE DECLARIATIONS  //
	//BEGIN SPI INIT
		////SPRC////[DS:17.2.1]
		//Interupts disabled, SPI enabled, MSB first, Master mode
		//Low clk idle, fosh/64
		SPRC = 0b01010111;	//every bit in this register can be written 	
		SPSR = 0b00000000;	//only bit zero can be written in this register, if set it doubles spi clock frequency. 
		//SPDR is data register, write data to it to transmit, read it twice to recieve data. 
	//END SPI INIT
	//BEGIN GIPO INIT
		//See system hardware documentation for GPIO connections	
		//DDRX is data direction. 1 is output, 0 is input [DS:10.2]
		//PORTX will set the value to output, or the pullmode of the input. 1 is up 0 is down. 
		//PINX can be used to read the value of the input pins of X
		DDRC |= 
		DDRB = 0b00000111;	//PB6 input for the arm switch, PB[2:0] output for SPI (This is forced by SPI but don't override)
	     	//PORT B is X, ARM, X, X, MISO, MOSI, SCK, NSS	
	//END GPIO INIT
	while(1){			//Endless while loop
		//STATE MACHINE
		//RF WATCHDOG
		//
		//
		switch(state){
		case(state_idle): 
			if(comm_en_debounced();){
				state = state_established;
			}
			else{
				state = state_idle;
			}
			break;
		case(state_establish):
			
			break;
		case(state_arm_ready):
			break;
		case(state_arming):
			break;	
		case(state_armed!):
			break;
		case(state_launch):
			break;
		default:
			break;
			}//switch
		
		
	}//while_1
}//main
