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
////SPI MACROS////
//ADDRESSES
#define READ(x)		(x)		//the addresses are in read mode by default
#define WRITE(x)	((x) | (1<<7))	//setting bit 7, write not read, sets the mode to write for that register. 
#define FIFO		0X00	//WNR 0, addr 0
#define OPMODE		0X01	//
#define	DATAMOD		0X02 
#define BITRATE_H	0X03
#define BITRATE_L	0X04
#define F_DEV_H		0X05
#define F_DEV_L		0X06
#define FRF_H		0X07
#define FRF_M		0X08
#define FRF_L		0X09
#define OSC1		0X0A
#define AFC_CTRL	0X0B
#define LISTEN1		0X0D
#define LISTEN2		0X0E
#define LISTEN3		0X0F
#define VERSION		0X10
#define PA_LEVEL	0X11
#define PA_RAMP		0X12
#define OCP		0X13
#define LNA		0X18
#define RX_BW		0X19
#define AFC_BW		0X1A
#define OOK_PEAK	0X1B
#define OOK_AVG		0X1C
#define OOK_FIX		0X1D
#define AFC_FEI		0X1E
#define AFC_H		0X1F
#define AFC_L		0X20
#define FEI_H		0X21
#define FEI_L		0X1A
#define FEI_L		0X22
#define RSSI_CONFIG	0X23
#define RSSI_VALUE	0X24
#define DIO_MAP_1	0X25
#define DIO_MAP_2	0X26
#define IRQ_FLAGS_1	0X27
#define IRQ_FLAGS_2	0X28
#define	RSSI_THRESHOLD	0X29
#define RX_TIMEOUT_1	0X2A
#define RX_TIMEOUT_2	0X2B
#define PREAMBLE_H	0X2C
#define PREAMBLE_L	0X2D
#define SYNC_CONFIG	0X2E
#define SYNC_VALUE_1	0X2F
#define SYNC_VALUE_8	0X36
#define PACKET_CONFIG_1	0X37
#define PAYLOAD_LENGTH	0X38
#define NODE_ADDRS	0X39
#define BRODCAST_ADDRS	0X3A
#define AUTO_MODES	0X3B
#define FIFO_THRESH	0X3C
#define PACKET_CONFIG_2	0X3D
#define AES_KEY_ADDR_H	0X3E
#define AES_KEY_ADDR_l	0X4D
#define TEMP_1		0X4E
#define TEMP_2		0X4F
#define TEST_LNA	0X58
#define TEST_PA_1	0x5A
#define TEST_PA_2	0x5C
#define TEST_DAGC	0x6F
#define TEST_AFC	0x71
//DATA
#define OPMODE_SLEEP	
#define OPMODE_STANDBY	
#define OPMODE_FS	
#define OPMODE_RX	
#define OPMODE_TX	
#define OPMODE_LISTEN
#define OPMODE_ABORT_LISTEN
#define DATA_MOD_FSK_PACKET
#define BITRATE_H_4800
#define BITRATE_L_4800
#define F_DEV_H_5K	0X00
#define F_DEV_L_5K	0X52
#define FRF_H_915M	0X
#define FRF_M_915M	0X
#define FRF_L_915M	0X
#define OSC1		0X
#define AFC_CTRL	0X
#define LISTEN1		0X
#define LISTEN2		0X
#define LISTEN3		0X
#define VERSION		0X
#define PA_LEVEL	0X
#define PA_RAMP		0X
#define OCP		0X
#define LNA		0X
#define RX_BW		0X
#define AFC_BW		0X
#define OOK_PEAK	0X
#define OOK_AVG		0X
#define OOK_FIX		0X
#define AFC_FEI		0X
#define AFC_H		0X
#define AFC_L		0X
#define FEI_H		0X
#define FEI_L		0X
#define FEI_L		0X
#define RSSI_CONFIG	0X
#define RSSI_VALUE	0X
#define DIO_MAP_1	0X
#define DIO_MAP_2	0X
#define IRQ_FLAGS_1	0X
#define IRQ_FLAGS_2	0X
#define	RSSI_THRESHOLD	0X
#define RX_TIMEOUT_1	0X
#define RX_TIMEOUT_2	0X
#define PREAMBLE_H	0X
#define PREAMBLE_L	0X
#define SYNC_CONFIG	0X
#define SYNC_VALUE_1	0X
#define SYNC_VALUE_8	0X
#define PACKET_CONFIG_1	0X
#define PAYLOAD_LENGTH	0X
#define NODE_ADDRS	0X
#define BRODCAST_ADDRS	0X
#define AUTO_MODES	0X
#define FIFO_THRESH	0X
#define PACKET_CONFIG_2	0X
#define AES_KEY_ADDR_H	0X
#define AES_KEY_ADDR_l	0X
#define TEMP_1		0X
#define TEMP_2		0X
#define TEST_LNA	0X
#define TEST_PA_1	0x
#define TEST_PA_2	0x
#define TEST_DAGC	0x
#define TEST_AFC	0x

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
	    	DDRE 	= 0x00;		//set PORT E input
		//IOPORT B is X, ARM, COMM_EN, X, MISO, MOSI, SCK, NSS
		PORTC	= (1<<LAUNCH_BUTTON);	// set launch button to be pullup, RESET SENSE to be z 
		PORTB 	= ((1<<ARM_SW)|(1<<NSS)|(1<<COMM_EN_SW));// set arm sw to be pullup, write pull not slave select high
	       	PORTE 	= 0;
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
		//BEGIN CASE STATEMENTS FOR STATE MACHINE CONTROL
		switch(state){	//switch case statements for state machine control
			case(STATE_IDLE): 		//in the idle state
				PORTF = SIG_IDLE;
				communications_enabled = poll_comm_sw();
				if(communications_enabled){	//if the communications enable switch is flipped
					state = STATE_ESTABLISH;	//change state to esablish communications
					set_mode_tx();
				}//if comm deb
				else{ state = STATE_IDLE;	//hold state as IDLE;
				}//else comm deb
			break;
		case(STATE_ESTABLISH):
				//establish functionality
				PORTF = SIG_ESTABLISH;
				tx_when_ready(0x80,'t','e','s','t');// 0b10000000 is write mode for FIFO, hex 0x80
				_delay_ms(500);
				if(bit_is_set(PINE, 6)){	
					set_mode_rx();
					read_fifo(message);
					if(strcmp(message, msg_tst_1)){
						communications_established = true;
					}
					else{
						communications_established = false;
					}
				}
				if(communications_established)		{state = STATE_READY;}//if comm est
				else					{state = STATE_ESTABLISH;}//else comm est
			break;
		case(STATE_READY):
				PORTF = SIG_READY;
				arm_sw_debounced = poll_arm_sw();
				if(!communications_established)	{state = STATE_ESTABLISH;}
				else if(arm_sw_debounced)	{state = STATE_ARMING;}//IF ARM SW DEB
				else				{state = STATE_READY;}//else arm sw deb
			break;
		case(STATE_ARMING):
				PORTF = SIG_ARMING;
				set_mode_tx();
				_delay_ms(TRANSITON_TIME);
				tx_when_ready(0x80,'a','r','m','1');
				_delay_ms(TX_TIME);// time to send 
				set_mode_rx();
				_delay_ms(TRANSITON_TIME);
				_delay_ms(RX_TIME);
				read_fifo(message);
				if(strcmp(message, msg_arm_1)){
					system_armed = true;
				}
				else{
					system_armed = false;
				}
				if(!arm_sw_debounced)		{state = STATE_READY;}
				else if(system_armed)		{state = STATE_ARMED;}//if armed
				else				{state = STATE_ARMING;}//else armed
			break;	
		case(STATE_ARMED):
				PORTF = SIG_ARMED;
				launch_button_debounced = poll_launch_button();
				if (!arm_sw_debounced)			{state = STATE_READY;}
				else if (launch_button_debounced)	{state = STATE_FIREING;}
				else 					{state = STATE_ARMED;}
			break;
		case(STATE_FIREING):
				PORTF = SIG_FIREING;
				set_mode_tx();
				tx_when_ready(0x80,'f','i','r','e');
				_delay_ms(TX_TIME);//allow time to tx
				set_mode_rx();
				_delay_ms(TRANSITON_TIME);
				_delay_ms(RX_TIME);
				read_fifo(message);
				if(strcmp(message, msg_fire1)){
					fire_ackd = true;
				}
				else{
					fire_ackd = false;
				}
				if(fire_ackd)		{ state = STATE_FIRED;  }//if fire ackd
				else			{ state = STATE_FIREING;}
			break;
		case(STATE_FIRED):
				PORTF = SIG_FIRED;
				if (!communications_enabled)		{state = STATE_IDLE;}
				if (!system_armed)			{state = STATE_ESTABLISH;}//IF SYS UNARMED
				else 					{state = STATE_FIRED;}
			break;
		default: 				state = STATE_IDLE;
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
	_delay_ms(1);			//time for nss pin to become low. #FIXME lower this value
	SPDR = address;
	while(!(SPSR & (1<<7))){;}	// this litteraly just does nothing until the spi isnt sending something. maybe make it flash arm2?
	for(i=0;i<4;i++){		 
		SPDR = word[i];
		while(!(SPSR & (1<<7))){;}// this litteraly just waits until the spi isnt sending something. 
	}
	PORTB |= (1<<0);//set nss high
	_delay_ms(1);			//time for nss pin to become low. #FIXME lower this value
}
//anticipated hold time: (FOSC / 64) SCK, MAX TIME is limit of six 1 byte transmissions, minimum of five 1 byte transmissions. 
// 64 * 8 * bytes = approx hold time for transmissions. add in time for other functions. 
//end tx when ready function description


//void set_mode_tx();
void set_mode_tx(){
	PORTB = (PORTB & ~(1<<0));	//nss low for begining of packet transmission. 
	_delay_ms(1);			//time for nss pin to become low. #FIXME lower this value
	SPDR = 0b10000001;		//this says write mode address 0x01
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	SPDR = 0b10001100;		//mode register value for transmission mode. 
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	PORTB |= (1<<0);		//set nss high at end of transmission
	_delay_ms(1);
}	
//void set_mode_rx();
void set_mode_rx(){
	PORTB = (PORTB & ~(1<<0));	//nss low for begining of packet transmission. 
	_delay_ms(1);
	SPDR = 0b10000001;		//this says write mode address 0x01
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	SPDR = 0b10010000;		//mode register value for reciever mode. 
	while(!(SPSR & (1<<7))){;}	//this litteraly just waits until the spi isnt sending something.
	PORTB |= (1<<0);		//set nss high at end of transmission
	_delay_ms(1);			//time for nss pin to become low. #FIXME lower this value
}	
//void read_fifo(char*);
void read_fifo(char* current_char){	//no array for readablility of main
	uint8_t i;
	char word[4];
	PORTB = (PORTB & ~(1<<0));	//nss low for begining of packet transmission. 
	_delay_ms(1);			//time for nss pin to become low. #FIXME lower this value
	SPDR = 0b00000000;		//read FIFO address command
	while(!(SPSR & (1<<7))){;}	// this litteraly just does nothing until the spi isnt sending something. maybe make it flash arm2?
	for(i=0;i<4;i++){
		word[i] = SPDR;
		while(!(SPSR & (1<<7))){;}// this litteraly just waits until the spi isnt sending something. 
	}
	PORTB |= (1<<0);	//set nss high
	for(i=0;i<4;i++){
	*(current_char + i) = word[i];
	}
	_delay_ms(1);
}

//bool poll_comm_en();
bool poll_comm_sw(){
	static uint8_t comm_en_deb = 0;		//static variable to count low signal time
	_delay_ms(10);							//delay so debounce is in human times
	if(bit_is_clear(PINB, COMM_EN_SW))	{comm_en_deb++;}	//if the bit is cleared, increment the counter
	else{								//else bit set
		if(comm_en_deb == 0)	{;}					//if the counter is 0 do nothing
		else			{comm_en_deb = comm_en_deb - 1;}	//else the counter is non0, decrement it	
	}
	if (comm_en_deb >= 25){comm_en_deb = 25; return true;}		//if the count is greater than 50, set back to 50 and return true
	else return false;						//else return false
}
//bool poll_arm_sw();
bool poll_arm_sw(){
	static uint16_t arm_sw_deb = 0;
	if(bit_is_clear(PINB,ARM_SW)){arm_sw_deb++;}		//if the bit is cleared, increment the counter
	else{								//else bit set
		if(arm_sw_deb == 0)	{;}					//if the counter is 0 do nothing
		else			{arm_sw_deb = arm_sw_deb - 1;}	//else the counter is non0, decrement it	
	}
	_delay_ms(10);							//delay so debounce is in human times
	if (arm_sw_deb >= 25){arm_sw_deb = 25; return true;}		//if the count is greater than 50, set back to 50 and return true
	else return false;						//else return false
}
//bool poll_launch_button();
bool poll_launch_button(){
	static uint16_t lb_deb = 0;
	if(bit_is_clear(PINB,LAUNCH_BUTTON)){lb_deb++;}			//if the bit is cleared, increment the counter
	else{								//else bit set
		if(lb_deb == 0)	{;}					//if the counter is 0 do nothing
		else			{lb_deb = lb_deb - 1;}		//else the counter is non0, decrement it	
	}
	_delay_ms(10);							//delay so debounce is in human times
	if (lb_deb >= 25){lb_deb = 25; return true;}			//if the count is greater than 50, set back to 50 and return true
	else return false;						//else return false
}
