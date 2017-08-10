/* ###################################################################
 **     Filename    : main.c
 **     Project     : Lab1
 **     Processor   : MK70FN1M0VMJ12
 **     Version     : Driver 01.01
 **     Compiler    : GNU C Compiler
 **     Date/Time   : 2015-07-20, 13:27, # CodeGen: 0
 **     Abstract    :
 **         Main module.
 **         This module contains user's application code.
 **     Settings    :
 **     Contents    :
 **         No public methods
 **
 ** ###################################################################*/
/*!
 ** @file main.c
 ** @version 1.0
 ** @brief
 **         Main module.
 **         This module contains user's application code.
 */
/*!
 **  @addtogroup main_module main module documentation
 **  @{
 */
/* MODULE main */
/*
 * Main.c
 *
 *  Created on: 1 Aug 2017
 *  Last Modified 8 Aug 2017
 *      Author: 11970744, 11986282
 */

// CPU mpdule - contains low level hardware initialization routines
#include "Cpu.h"
#include "MK70F12.h"
#include "FIFO.h"
#include "packet.h"
#include "UART.h"

#define BAUD_RATE 38400
// Private Global Variable
TFIFO MyFIFO1;

static uint16union_t Tower_Value = { 0x188A };
uint8_t Packet_No_Ack;

void Tower_Version() {
	Packet_Put(Packet_No_Ack, 0x76, 0x01, 0x00);
}

void Tower_Startup() {
	Packet_Put(0x04, 0x00, 0x00, 0x00);
	Tower_Version();
	Packet_Put(0x08, 0x01, Tower_Value.s.Hi, Tower_Value.s.Lo);
}

void Check_Ack() {
	if ((Packet_Command & PACKET_ACK_MASK) == PACKET_ACK_MASK) {
		Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2,
				Packet_Parameter3);
	}
}

//check each bit with the packet_position to ensure that the packets are aligned.
void Packet_Handle() {
	uint8_t Packet_No_Ack = Packet_Command & ~PACKET_ACK_MASK;

	switch (Packet_Command) {
	case 0x04:	//Tower startup
		Tower_Startup();
		break;

	case 0x09:	//Special Tower version
		Tower_Version();
		break;

	case 0x0B:
		if (Packet_Parameter1 == 2) {
			Packet_Put(0x08, 0x01, Packet_Parameter1, Packet_Parameter2);
		} else
			Packet_Put(0x08, 0x01, Tower_Value.s.Hi, Tower_Value.s.Lo);
		break;

	}
	Check_Ack();

}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
	/* Write your local variable definition here */

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
	/*** End of Processor Expert internal initialization.                    ***/

	Packet_Init(BAUD_RATE, CPU_BUS_CLK_HZ);
	Tower_Startup();

	/* Write your code here */
	for (;;) {
		if (Packet_Get()) {
			Packet_Handle();
		}
		UART_Poll();
	}

	/*** Don't write any code pass this line, or it will be deleted during code generation. ***/
	/*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
#ifdef PEX_RTOS_START
	PEX_RTOS_START(); /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
#endif
	/*** End of RTOS startup code.  ***/
	/*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
	for (;;) {
	}
	/*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
 ** @}
 */
/*
 ** ###################################################################
 **
 **     This file was created by Processor Expert 10.5 [05.21]
 **     for the Freescale Kinetis series of microcontrollers.
 **
 ** ###################################################################
 */
