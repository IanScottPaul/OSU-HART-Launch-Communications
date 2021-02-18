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
#include <stdbool.h>
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
void tx_when_ready(uint8_t, char, char, char, char);
void set_mode_tx();
void set_mode_rx();
void read_fifo(char*);

bool poll_comm_en();
bool poll_arm_sw();
bool poll_launch_button();

uint8_t main(){	
	////Program Initialization////
	//START OF MAIN FUNCTION VARIABLE DECLARIATIONS //
		uint8_t	state = STATE_IDLE;		// state determines the current state of the program, use state macros. 
		char[4]	message;			// the four character code transmitted or recieved.  
			//address[7] is WNR, adsress[6:0] are the actual address bits. I have chosen to hardcode addresses in funtion calls.  
			//The two critical addresses are 0x00 for FIFO, and 0x01 for the Operation mode register REGOPMODE	
		//boolean variables for state machine. 
		bool communications_enabled 	= false;
		bool communications_established = false;
		bool arm_sw_debounced		= false;
		bool system_armed		= false;
		bool launch_button_debounced 	= false;
	// END OF MAIN FUNCTION VARIABLE DECLARIATIONS  //
/////////BEGIN GIPO INIT/////////////////////////
						//See system hardware documentation for GPIO connection
						//DDRX is data direction. 1 is output, 0 is input [DS:10.2]
						//PORTX will set the value to output, or the pullmode of the input. 1 is up 0 is down. 
						//PINX can be used to read the value of the input pins of X
		DDRC 	= 0b00000000;	//The only two pins on DDRC are inputs
		DDRF	= 0xFF;		// all PORTF pins are outputs. 	
		DDRB 	= 0b00000111;	//PB6 input for the arm switch, PB[2:0] output for SPI 
	    	//IOPORT B is X, ARM, X, X, MISO, MOSI, SCK, NSS
		PORTC	= (1<<LAUNCH_BUTTON);	// set launch button to be pullup, RESET SENSE to be z 
		PORTB 	= ((1<<ARM_SW)|(1<<NSS));// set arm sw to be pullup, write pull not slave select high
	       	PORTF	= 0b11111100;		//just in case that statements ends up in the ignition, set fire to high.
//////////END GPIO INIT///////////////////////////
///////////BEGIN SPI INIT////////////////////////////////////////////////////////////////
		////SPRC////[DS:17.2.1]
		//Interupts disabled, SPI enabled, MSB first, Master mode
		//Low clk idle, fosh/64
		SPRC = 0b01010010;	//every bit in this register can be written. 
		SPSR = 0b00000000;	//only bit zero can be written in this reg so im not too worried about clobbering it here. 
		//SPDR is data register, write data to it to transmit, read it twice to recieve data. 
//////////END SPI INIT
	////END OF PROGRAM INITIALIZATION
	//BEGIN ENDLESS WHILE LOOP
	while(1){						
		//BEGIN CASE STATEMENTS FOR STATE MACHINE CONTROL
		switch(state){	//switch case statements for state machine control
			case(STATE_IDLE): 		//in the idle state
				if(communications_enabled()){	//if the communications enable switch is flipped
					state = STATE_ESABLISH;	//change state to esablish communications
				}//if comm deb
				else{ state = STATE_IDLE;	//hold state as IDLE;
				}//else comm deb
			break;
		case(STATE_ESABLISH):
				if(communications_established)	{state = STATE_READY;}//if comm est
				else					{state = STATE_IDLE;}//else comm est
			break;
		case(STATE_READY):
				if(arm_sw_debounced)		{state = STATE_ARMING;}//IF ARM SW DEB
				else				{STATE = STATE_READY;}//else arm sw deb
			break;
		case(STATE_ARMING):
				if(system_armed)		{STATE = STATE_ARMED!;}//if armed
				else				{state = STATE_ARMING;}//else armed
			break;	
		case(STATE_ARMED!):
				if(system_unarmed)			{state = STATE_ESABLISH;}//IF SYS UNARMED
				else if (launch_button_debounced)	{state = STATE_FIREING;}
				else 					{state = STATE_ARMED!;}
			break;
		case(STATE_FIREING):
				if(fire_ackd)	{state = STATE_FIRED;}//if fire ackd
				else			{ STATE = STATE_FIREING;}
			break;
		case(STATE_FIRED!):
				if(system_unarmed())		{state = STATE_ESABLISH;}//IF SYS UNARMED
				else if (comms_disabled())	{state = STATE_IDLE;}
				else 				{state = STATE_FIRED!;}
			break;
		default: state = STATE_IDLE;
			break;
		}//switch control
////////////////////END OF STATE MACHINE CONTROL CASES////////////////////////////////////////////////////////////////


////////////////////BEGIN CASE STATEMENTS FOR STATE MACHINE FUNCTIONALITY/////////////////////////////////////////////
		switch(state){	//switch case statements for state machine functionality
		//CODE HERE IS STATE DEPENDENT
			case(STATE_IDLE): 
			//idle functionality
			communications_enabled = poll_comm_sw();
			break;
		case(STATE_ESABLISH):
			//establish functionality
			set_mode_tx();
			tx_when_ready(0x80,'t','e','s','t');// 0b10000000 is write mode for FIFO, hex 0x80
			_delay_ms(50);
			set_mode_rx();
			_delay_ms(100);//time for message to tx, be decoded, and sent back 
			read_fifo(&message);
			if(strcmp(&message, &msg_tst_1)){
				comms_established = true;
			}
			else{
				comms_established = false;
			}
			break;
		case(STATE_READY):
			arm_sw_debounced = poll_arm_sw();
			//ready functionality
			break;
		case(STATE_ARMING):
			//arming functionality
			set_mode_tx();
			tx_when_ready(0x80,'A','R','M''1');
			_delay_ms(50);// time to send 
			set_mode_rx();
			read_fifo(&message);
			if(strcmp(&message, &msg_arm1)){
				system_armed = true;
			}
			else{
				system_armed = false;
			}
break;	
		case(STATE_ARMED!):
			launch_button_debounced = poll_launch_button();
			break;
		case(STATE_FIREING):
			set_mode_tx();
			tx_when_ready(0x80,'F','I','R','E');
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
//FUNCTIONS//
//
//
//void tx_when_ready(uint8_t, char, char, char, char);
//this function takes full control of the uC during the SPI tranmission. 
void tx_when_ready(uint8_t address, char c0, char c1, char c2, char c3){//no array for readablility of main
	while(!(SPSR & (1<<7))){;}// this litteraly just waits until the spi isnt sending something. 
	uint8_t i;
	char word[4];
	word[0] = c0;
	word[1] = c1;
	word[2] = c2;
	word[3] = c3;
	PORTB = (PORTB & ~(1<<0));//nss low for begining of packet transmission. 
	SPDR = address;
	while(!(SPSR & (1<<7))){;}// this litteraly just does nothing until the spi isnt sending something. maybe make it flash arm2?
	for(i=0;i<4;i++){
		SPDR = word[i];
		while(!(SPSR & (1<<7))){;}// this litteraly just waits until the spi isnt sending something. 
	}
	PORTB |= (1<<0);//set nss high
}
//anticipated hold time: (FOSC / 64) SCK, MAX TIME is limit of six 1 byte transmissions, minimum of five 1 byte transmissions. 
// 64 * 8 * bytes = approx hold time for transmissions. add in time for other functions. 
//end tx when ready function description


//void set_mode_tx();
void set_mode_tx(){
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	PORTB = (PORTB & ~(1<<0));	//nss low for begining of packet transmission. 
	SPDR = 0b10000001;		//this says write mode address 0x01
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	SPDR = 0b00001100;		//mode register value for transmission mode. 
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	PORTB |= (1<<0);		//set nss high at end of transmission
}	
//void set_mode_rx();
void set_mode_rx(){
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	PORTB = (PORTB & ~(1<<0));	//nss low for begining of packet transmission. 
	SPDR = 0b10000001;		//this says write mode address 0x01
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	SPDR = 0b00010000;		//mode register value for reciever mode. 
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	PORTB |= (1<<0);		//set nss high at end of transmission
}	
//void read_fifo(char*);
void read_fifo(char current_char*){	//no array for readablility of main
	while(!(SPSR & (1<<7))){;}	// this litteraly just waits until the spi isnt sending something. 
	uint8_t i;
	char word[4];
	PORTB = (PORTB & ~(1<<0));	//nss low for begining of packet transmission. 
	SPDR = 0b00000000;		//read FIFO address command
	while(!(SPSR & (1<<7))){;}	// this litteraly just does nothing until the spi isnt sending something. maybe make it flash arm2?
	for(i=0;i<4;i++){
		word[i]; = SPDR;
		while(!(SPSR & (1<<7))){;}// this litteraly just waits until the spi isnt sending something. 
	}
	PORTB |= (1<<0);		//set nss high
	
	for(i=0;i<4;i++){
	current_char[i] = word[i];// FIXME THIS MIGHT NOT WORK. IF THERE IS NO MEMORY PROTECTION IT SHOULD BE FINE
	}
}

//bool poll_comm_en();
bool poll_comm_en(){
//TODO IMPLEMENT SHIFTING DEBOUNCER, TOGGLED OUPUT POST DEBOUNCED. 
}
//bool poll_arm_sw();
bool poll_arm_sw(){
//TODO IMPLEMENT SHIFTING DEBOUNCER, TOGGLED OUPUT POST DEBOUNCED. 

}
//bool poll_launch_button();
bool poll_launch_buttion(){
//TODO IMPLEMENT SHIFTING DEBOUNCER, TOGGLED OUPUT POST DEBOUNCED. 

}


