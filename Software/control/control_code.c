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
		//initialize GPIO
		uint8_t state = state_idle;
		//INIT SPI
		SPRC = 0b01010111;
		SPSR = 0b00000000;
		//SPDR is data register
		//
		//INIT TIMERS
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
