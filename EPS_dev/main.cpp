/*
 * main.cpp
 *
 *  Created on: May 9, 2015
 *      Author: quentin
 */

#include <msp430.h>
#include <msp430g2744.h>


/******************************/
//alias definition for the pins
//analogic onboard current and voltage sensors
#define CH_SOL_CURRENT		INCH02
#define CH_BAT_CURRENT		INCH01
#define CH_BUS_VOLT			INCH06
#define CH_BAT_VOLT			INCH07
//4 external analog inputs
#define ANALOG1		INCH12
#define ANALOG2 	INCH13
#define ANALOG3 	INCH14
#define ANALOG4 	INCH15
//protected external power switches
#define EXT_PW1
#define EXT_PW2
#define EXT_PW3
//heater on Hexfet transistors
#define HEAT1
#define HEAT2
#define HEAT3
//enable pin for battery charging
#define EN_CHG
//I2C address for mainboarf
#define I2C_MAIN_ADDR

/********************************/
// state machine
#define PARAM_STATUS  0  //store status on one byte

	#define FLAG_EMERGENCY  0     //main is OFF
	#define FLAG_COM        1     //COM power status
	#define FLAG_MOTORS     2     //MOTORS power status
	#define FLAG_HEAT1      3
	#define FLAG_HEAT2      4
	#define FLAG_HEAT3      5
	#define FLAG_CHARGING   6
	#define FLAG_TEMP_BT_OK 7

/*******************************/
//errors to be solved by Main
#define PARAM_ERROR_BYTE
	#define ERROR_MOTOR_STALL

/******************************/
//Transmitted parameters
#define PARAM_POWER_CONSO
#define PARAM_POWER_PROD
#define PARAM_ESTIMATED_AUTONOMY
#define PARAM_BAT_VOLT
#define PARAM_BUS_VOLT
#define PARAM_BAT_TEMP

/*
 * main.c
 */
void main(void) {

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	//initializing pins I/O direction 0 = IN 1=OUT
	P1DIR=0b11111110; //all but P1.0 are OUTPUTS
	P2DIR=0b10000000; //all but XOUT are INPUTS
	P3DIR=0b00000000; //all INPUTs
	P4DIR=0b00000000; //all INPUTs

	//initializing pins mode 0 = I/O 1=AUX
	P1SEL=0x00;
	P2SEL=0xC6; //init (A01) (A02) and (Xin) (Xout) functions TO BE CHECKED for XIN !!
	P3SEL=0xC6; //init (A06) (A07) and SDA SCL functions      TO Be CHECKED for I2C !!
	P4SEL=0x78; //init (A12) to (A15)

	//initializing OUTPUT values
	P1OUT= P1DIR | 0x00; //init all outputs to GND

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

//	while(1){

//	}

}


