//*****************************************************************************
//
// blinky.c - Simple example to blink the on-board LED.
//
// Copyright (c) 2007-2011 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 8049 of the EK-LM3S8962 Firmware Package.
//
//*****************************************************************************

#include "inc/lm3s8962.h"
#include <stdint.h>
#include "APICommand.h"
#include "channel.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Blinky (blinky)</h1>
//!
//! A very simple example that blinks the on-board LED.
//
//*****************************************************************************

//*****************************************************************************
//
// Blink the on-board LED.
//
//*****************************************************************************
int
main(void)
{
    volatile unsigned long ulLoop;

    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;

    //
    // Do a dummy read to insert a few cycles after enabling the peripheral.
    //
    ulLoop = SYSCTL_RCGC2_R;

    //
    // Enable the GPIO pin for the LED (PF0).  Set the direction as output, and
    // enable the GPIO pin for digital function.
    //
    GPIO_PORTF_DIR_R = 0x01;
    GPIO_PORTF_DEN_R = 0x01;

    ApiRead r1, r2;
    ApiWrite w1, w2;
    //ApiAck a1, a2;
    //ApiNot n1, n2;

    ApiWrite_ctor(&w1, 1, INPUT, EQB1, BW, 2.0);
    ApiWrite_ctor(&w2, 2, OUTPUT, EQB2, GAIN, 4.0);

    ApiRead_ctor(&r1, 1, INPUT, EQB1, BW);
    ApiRead_ctor(&r2, 2, INPUT, EQB1, BW);

    //ApiAck(&a1, )


    //
    // Loop forever.
    //
//    while(1) {

	//
	// Turn on the LED.
	//
	GPIO_PORTF_DATA_R |= 0x01;

	//
	// Delay for a bit.
	//
	for(ulLoop = 0; ulLoop < 500000; ulLoop++)
	{
	}

	//
	// Turn off the LED.
	//
	//GPIO_PORTF_DATA_R &= ~(0x01);

	//
	// Delay for a bit.
	//
	for(ulLoop = 0; ulLoop < 5000000; ulLoop++)
	{
	}
}
//}
