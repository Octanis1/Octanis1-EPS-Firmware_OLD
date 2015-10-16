/*
 * main.cpp
 *
 *  Created on: Aug, 2015
 *      Author: Quentin Cabrol
 */

#include <msp430.h>
#include <msp430g2744.h>
#include "TI_USCI_I2C_slave.h"

/*********************************************************************/
//I2C callback functions annd address definitions
//http://www.ti.com/general/docs/litabsmultiplefilelist.tsp?literatureNumber=slaa383

#define I2C_EPS_SLAVE_ADDR 0X90

static unsigned char RXData;
static unsigned char TXData;

/* callback for start condition */
void start_cb();

/* callback for bytes received */
void receive_cb(unsigned char receive);

/* callback to transmit bytes */
void transmit_cb(unsigned char volatile *send_next);



/******************************/
//alias definition for the pins
//analogic onboard current and voltage sensors
#define CH_SOL_CURRENT		INCH_2
#define CH_BAT_CURRENT		INCH_1
#define CH_BUS_VOLT			INCH_6
#define CH_BAT_VOLT			INCH_7
//4 external analog inputs
#define ANALOG1		INCH_12
#define ANALOG2 	INCH_13
#define ANALOG3 	INCH_14
#define ANALOG4 	INCH_15
//protected external power switches  --> to be set
#define EXT_PW1		BIT5
#define EXT_PW2		BIT6
#define EXT_PW3		BIT7
//heater on Hexfet transistors   --> to be set
#define HEAT1		BIT1
#define HEAT2		BIT2
#define HEAT3		BIT3
//enable pin for battery charging  --> to be set
#define EN_CHG		BIT4
//output port
#define CTRL_PORT	P1OUT
//I2C address for mainboarf  --> to be set
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


void EPS_init(void){

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	//initializing pins I/O direction 0 = IN 1=OUT
	P1DIR=0b11111110; //all but P1.0 are OUTPUTS
	P2DIR=0b10000000; //all but XOUT are INPUTS
	P3DIR=0b00000000; //all INPUTs
	P4DIR=0b00000000; //all INPUTs
	//initializing pins mode 0 = I/O 1=AUX
	P1SEL=0x00;
	P2SEL=0xC6; //init (A01) (A02) and (Xin) (Xout) functions TO BE CHECKED for XIN !!
	P3SEL=0xC6; //init (A06) (A07) and SDA SCL functions
	P4SEL=0x78; //init (A12) to (A15)
	//initializing analog pins
	ADC10AE0 = 0b11000110; //(A01)(A02)(A06)(A07)
	ADC10AE1 = 0b11110000; //(A12)(A13)(A14)(A15)

	//initializing OUTPUT values
	CTRL_PORT = P1DIR | 0x00; //init all outputs to GND

	//init I2C with EPS as slave
	TI_USCI_I2C_slaveinit(start_cb, transmit_cb, receive_cb, I2C_EPS_SLAVE_ADDR); // init the slave
	_EINT();

	//initilize clock
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;
	//LPM0;

	//initializing ADC
	ADC10CTL1 = CH_BAT_VOLT + CONSEQ_1 + ADC10SSEL_2 + ADC10DIV_7;            // A7 to A0, single sequence
	ADC10CTL0 = ADC10SHT_1 + SREF_1 + MSC + ADC10ON
			  + ADC10IE + REFON + REF2_5V + REFBURST; //16 clock cycle SH, 2.5V/0V reference, Burst reference, interrupt enabled
}

void updateMeasurements(int blocking, int *buffer) {
	//ADC measurements (automatic update in BG?)
	ADC10CTL0 &= ~ENC;
	while (ADC10CTL1 & BUSY);               // Wait if ADC10 core is active

	ADC10SA = (unsigned int) buffer;                   // Data buffer start
	ADC10CTL1 = CH_BAT_VOLT + CONSEQ_1;     // A7 to A0, single sequence
	ADC10DTC1 = 0x08;                       // 8 conversions

	ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start

	if(blocking)
	{
		__bis_SR_register(CPUOFF + GIE);        // LPM0, ADC10_ISR will force exit

		//wait for ADC to finish (just in case i2C interrupt awakened processor)
		ADC10CTL0 &= ~ENC;
		while (ADC10CTL1 & BUSY);               // Wait if ADC10 core is active
	}
}


/*
 * main.c
 */
void main(void) {

	char flags = 0x01; // start in emergency state with main off
	float Vcc = 0;
	float Vbat = 0;
	int AdcResults[8];

	EPS_init();

//	while(1){

//	}

	for(;;)
	{
		// update voltage and batery values, blocking call to ADC, putting CPU to sleep
		updateMeasurements(1, AdcResults);

		//TODO conversion raw values to correct values & ref. values

		//Enable Mainboard
		if(flags & (1<<FLAG_EMERGENCY))
		{
			if(Vbat > 3.2 || Vcc > 3.30) {
				CTRL_PORT |= EXT_PW1; //Enable mainboard
				flags &= ~(1<<FLAG_EMERGENCY);
			}
		}
		else {
			if(Vbat < 2.8 || Vcc < 3.28) {
				CTRL_PORT &= ~EXT_PW1; //Disable mainboard
				flags |= (1<<FLAG_EMERGENCY);
			}
		}

		//Charging
		if(flags & (1<<FLAG_CHARGING)) {
			if(Vbat > 4.20 || Vcc < 3.32) {
				CTRL_PORT &= ~EN_CHG; //Disable charging
				flags &= ~(1<<FLAG_CHARGING);
			}
		}
		else {
			if(Vbat < 4.18 && Vcc > 3.34) {
				CTRL_PORT |= EN_CHG; //Enable charging
				flags |= (1<<FLAG_CHARGING);
			}
		}
	}
}



void start_cb(){
RXData = 0;
}
void receive_cb(unsigned char receive){
RXData = receive;
}
void transmit_cb(unsigned char volatile *send_next){
*send_next = TXData++;
}
