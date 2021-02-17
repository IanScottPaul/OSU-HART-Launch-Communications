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
////INCLUDED LIBRARIES////
#include <avr/io.h>

////STATE MACROS////
#define STATE_IDLE 	0
#define STATE_ESABLISH	1
#define STATE_READY 	2
#define STATE_ARMING	4
#define STATE_ARMED!	8
#define STATE_FIREING	16
#define STATE_FIRED!	255
////PORTC MACROS////
#define LAUNCH_BUTTON 	7
#define RESET_SENSOR	6
////PORTB MACROS////
#define NSS 		0
#define ARM_SW		6
////PORTF MACROS////
#define ARM_LED_1	0
#define ARM_LED_2	1
#define ARM_IG_1	4
#define FIRE_IG_1	5
#define ARM_IG_2	6
#define FIRE_IG_2	7
////petty macros	
#define STATE		state
#define TRUE		(1==1)
#define FALSE		(1==0)
////ALT. DATA TYPES////


////FUCNTION DECLARATIONS////
void CONTROL_GPIO_INIT();

////INTERRUPT VECTORS////




uint8_t main(){	
	////Program Initialization////
	//START OF MAIN FUNCTION VARIABLE DECLARIATIONS //
		uint8_t	state = STATE_IDLE;		// state determines the current state of the program, use state macros. 
		char[4]	message;			// the four character code transmitted or recieved.  
		uint8_t address;			// the adress in transceiver memory that the spi data will be stored.
			//address[7] is WNR, adsress[6:0] are the actual address bits. 
			//The two critical addresses are 0x00 for FIFO, and 0x01 for the Operation mode register REGOPMODE	
		uint8_t mode;
	// END OF MAIN FUNCTION VARIABLE DECLARIATIONS  //
///////////BEGIN SPI INIT////////////////////////////////////////////////////////////////
		////SPRC////[DS:17.2.1]
		//Interupts disabled, SPI enabled, MSB first, Master mode
		//Low clk idle, fosh/64
		SPRC = 0b01010111;	//every bit in this register can be written 	
		SPSR = 0b00000000;	//only bit zero  
		//SPDR is data register, write data to it to transmit, read it twice to recieve data. 
//////////END SPI INIT
//////////BEGIN GIPO INIT/////////////////////////
						//See system hardware documentation for GPIO connection
						//DDRX is data direction. 1 is output, 0 is input [DS:10.2]
						//PORTX will set the value to output, or the pullmode of the input. 1 is up 0 is down. 
						//PINX can be used to read the value of the input pins of X
		DDRC 	= 0b00000000;	//The only two pins on DDRC are inputs
		DDRF	= 0xFF;		// all PORTF pins are outputs. 	
		DDRB 	= 0b00000111;	//PB6 input for the arm switch, PB[2:0] output for SPI 
	    	//IOPORT B is X, ARM, X, X, MISO, MOSI, SCK, NSS
		PORTC	= (1<<LAUNCH_BUTTON);	// set launch button to be pullup, RESET SENSE to be z 
		PORTB 	= (1<<ARM_SW);		// set arm sw to be pullup, write 0 to all spi pins
	       	PORTF	= 0b11111100;		//just in case that statements ends up in the ignition, set fire to high.
//////////END GPIO INIT///////////////////////////
		//init timer?
		//
	//begin endless loop
	while(1){						//Endless while loop
		switch(state){	//switch case statements for state machine control
			case(STATE_IDLE): 		//in the idle state
				if(communications_enabled();){	//if the communications enable switch is flipped
					state = STATE_ESABLISH;	//change state to esablish communications
				}//if comm deb
				else{ state = STATE_IDLE;	//hold state as IDLE;
				}//else comm deb
			break;
		case(STATE_ESABLISH):
				if(communications_established();)	{state = STATE_READY;}//if comm est
				else					{state = STATE_IDLE;}//else comm est
			break;
		case(STATE_READY):
				if(arm_sw_debounced();)		{state = STATE_ARMING;}//IF ARM SW DEB
				else				{STATE = STATE_READY;}//else arm sw deb
			break;
		case(STATE_ARMING):
				if(system_armed();)		{STATE = STATE_ARMED!;}//if armed
				else				{state = STATE_ARMING;}//else armed
			break;	
		case(STATE_ARMED!):
				if(system_unarmed();)			{state = STATE_ESABLISH;}//IF SYS UNARMED
				else if (launch_button_debounced();)	{state = STATE_FIREING;}
				else 					{state = STATE_ARMED!;}
			break;
		case(STATE_FIREING):
				if(fire_ackd();)	{state = STATE_FIRED;}//if fire ackd
				else			{ STATE = STATE_FIREING;}
			break;
		case(STATE_FIRED!):
				if(system_unarmed();)		{state = STATE_ESABLISH;}//IF SYS UNARMED
				else if (comms_disabled();)	{state = STATE_IDLE;}
				else 				{state = STATE_FIRED!;}
			break;
		default: state = STATE_IDLE;
			break;
		}//switch control
		switch(state){	//switch case statements for state machine functionality
		//CODE HERE IS STATE DEPENDENT
			case(STATE_IDLE): 
			//idle functionality
			break;
		case(STATE_ESABLISH):
			//establish functionality
			break;
		case(STATE_READY):
			//ready functionality
			break;
		case(STATE_ARMING):
			//arming functionality
			break;	
		case(STATE_ARMED!):
			//af
			break;
		case(STATE_FIREING):
			//fireing functionality
			break;
		case(STATE_FIRED!):
			//post fire functionality
			break;
		default:
			//defualty functionality
			break;
			}//switch control
		//CODE HERE IS STATE INDEPENDENT
		
	}//while_1
}//main
