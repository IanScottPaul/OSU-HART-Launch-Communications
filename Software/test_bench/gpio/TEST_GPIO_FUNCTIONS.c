//////////////////////////////////////////////////
// FILE:	TEST_GPIO_FUNCTIONS.c 		//
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
#define STATE_IDLE 	0
#define STATE_ESTABLISH	1
#define STATE_READY 	2
#define STATE_ARMING	4
#define STATE_ARMED	8
#define STATE_FIREING	16
#define STATE_FIRED	255
////PORTC MACROS////
#define LAUNCH_BUTTON 	7
#define RESET_SENSOR	6
////PORTB MACROS////
#define NSS 		0
#define ARM_SW		6
#define COMM_EN_SW	5
////PORTF MACROS////
#define ARM_LED_1	0
#define ARM_LED_2	1
#define ARM_IG_1	4
#define FIRE_IG_1	5
#define ARM_IG_2	6
#define FIRE_IG_2	7
#define SIG_ERROR	0b11111111
#define SIG_IDLE	0b00000011
#define SIG_ESTABLISH	0b00000001
#define SIG_READY	0b00000010
#define SIG_ARMING	0b10000000
#define SIG_ARMED	0b10000001
#define SIG_FIREING	0b10000010
#define SIG_FIRED	0b10000011
////TIMING MACROS////
#define TRANSITON_TIME 	10
#define RX_TIME		100
#define TX_TIME		100

////////FUCNTION DECLARATIONS////////
void tx_when_ready(uint8_t, char, char, char, char);
void set_mode_tx();
void set_mode_rx();
void read_fifo(char*);

bool poll_comm_sw();
bool poll_arm_sw();
bool poll_launch_button();
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
		uint8_t	state = STATE_IDLE;		// state determines the current state of the program, use state macros. 
		char message[4];			// the four character code transmitted or recieved.  
		char msg_tst_1[4] = {'T','A','C','K'};
		char msg_arm_1[4] = {'A','R','M','A'};
		char msg_fire1[4] = {'F','I','R','A'};
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
		DDRC 	= 0b00000000;	//The only two pins on DDRC are inputs
		DDRF	= 0xFF;		// all PORTF pins are outputs. 	
		DDRB 	= 0b00000111;	//PB6 input for the arm switch, PB[2:0] output for SPI 
	    	//IOPORT B is X, ARM, COMM_EN, X, MISO, MOSI, SCK, NSS
		PORTC	= (1<<LAUNCH_BUTTON);	// set launch button to be pullup, RESET SENSE to be z 
		PORTB 	= ((1<<ARM_SW)|(1<<NSS)|(1<<COMM_EN_SW));// set arm sw to be pullup, write pull not slave select high
	       	PORTF	= SIG_ERROR;		//just in case that statements ends up in the ignition, set fire to high.
//////////END GPIO INIT///////////////////////////
///////////BEGIN SPI INIT////////////////////////////////////////////////////////////////
		////SPRC////[DS:17.2.1]
		//Interupts disabled, SPI enabled, MSB first, Master mode
		//Low clk idle, fosh/64
		SPCR = 0b01010010;	//every bit in this register can be written. 
		SPSR = 0b00000000;	//only bit zero can be written in this reg so im not too worried about clobbering it here. 
		//SPDR is data register, write data to it to transmit, read it twice to recieve data. 
//////////END SPI INIT
	////END OF PROGRAM INITIALIZATION
	//BEGIN ENDLESS WHILE LOOP
	while(1){						
	
		communications_enabled = poll_comm_sw();
		if(communications_enabled){PORTF = SIG_ESTABLISH;}
		else{PORTF = SIG_IDLE;}

		//BEGIN CASE STATEMENTS FOR STATE MACHINE CONTROL
}
}//main
//bool poll_arm_sw();
bool poll_comm_sw(){
	static uint8_t arm_sw_deb = 0;
	_delay_ms(1);
	if(bit_is_clear(PINB, 5)){arm_sw_deb++;}
	else{ if(arm_sw_deb == 0){;} else {arm_sw_deb = arm_sw_deb - 1;}}
	if (arm_sw_deb >= 50){arm_sw_deb = 50; return true;}
	else {return false;}
}
//bool poll_launch_button();
bool poll_launch_button(){
	static uint16_t lb_deb = 0;
	lb_deb = ((lb_deb << 1) | (!bit_is_clear(PINC,LAUNCH_BUTTON)) | 0xe000);
	if (lb_deb <= 0xF000){return true;}
	else return false;

}
