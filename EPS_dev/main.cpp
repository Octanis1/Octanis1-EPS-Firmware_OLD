/*
 * main.cpp
 *
 *  Created on: May 9, 2015
 *      Author: quentin
 */

#include <msp430.h>

/*
 * main.c
 */
void main(void) {

	while(1){
		WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	}

}

