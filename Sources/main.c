/* ###################################################################
Packet_Parameter3 **     Filename    : main.c
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
#include "Events.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "MK70F12.h"
#include "FIFO.h"
#include "packet.h"
#include "UART.h"
#include "Flash.h"
#include "types.h"
#include "LEDs.h"

#define BAUD_RATE 115200
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
	Packet_Put(0x0B, 0x01, Tower_Value.s.Lo, Tower_Value.s.Hi);
	//check the tower number and mode bytes - program default.
}

void Check_Ack(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3) {
	if ((Packet_Command & PACKET_ACK_MASK) == PACKET_ACK_MASK)
		Packet_Put(command, parameter1, parameter2, parameter3);

}

//check each bit with the packet_position to ensure that the packets are aligned.
void Packet_Handle() {
	uint8_t Packet_No_Ack = Packet_Command & ~PACKET_ACK_MASK;
	uint8_t data;
	volatile uint16union_t *NvTowerNb;
	volatile uint16union_t *NvTowerMd;

	switch (Packet_Command) {
	case 0x04:	//Tower startup
		Tower_Startup();
		break;

	case 0x09:	//Special Tower version
		Tower_Version();
		break;
//new lab 2 commands
	case 0x0B:
		if (Packet_Parameter1 == 2) {
			Packet_Put(0x0B, 0x01, Packet_Parameter1, Packet_Parameter2);
		} else
			Packet_Put(0x0B, 0x01, Tower_Value.s.Lo, Tower_Value.s.Hi);
		break;

	case 0x07: // from reading the manual why does it look like what we have for 0x0D is what we're meant to have for 0x07??
		if (Packet_Parameter1 == 8)
			Flash_Erase();
		//if ( Flash_AllocateVar((volatile void **)&NvTowerNb, sizeof(&NvTowerNb)))
		//{
		else{	Flash_Write8((volatile uint8_t*)&Packet_Parameter1, Packet_Parameter3);
			Packet_Put(0x0D, 0x00, NvTowerNb->s.Lo, NvTowerNb->s.Hi);}
		//}
		break;

	case 0x08:
		data = _FB(FLASH_DATA_START + Packet_Parameter1);
		Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, data);
		break;

	case 0x0D:
		if (Packet_Parameter1 == 1)
		Packet_Put(0x0D, 0x01, Tower_Value.s.Lo, Tower_Value.s.Hi);
		if(Packet_Parameter1 == 2)
		{
			if (Flash_AllocateVar((volatile void **)&NvTowerMd, sizeof(NvTowerMd)))
			{
			  Flash_Write16((volatile uint16_t*)&NvTowerMd, (const uint16_t)Packet_Parameter23);

			  Packet_Put(0x0D, 0x02, Packet_Parameter2, Packet_Parameter3);
			//Packet_Put(0x0D, 0x02, Packet_Parameter2, Packet_Parameter3);
			}
		}
		break;
	}
	Check_Ack(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);

}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
	/* Write your local variable definition here */

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
	/*** End of Processor Expert internal initialization.                    ***/

	if (Packet_Init(BAUD_RATE, CPU_BUS_CLK_HZ)){
		Tower_Startup();
		//Flash_Init();
		LEDs_Init();
		LEDs_On(LED_ORANGE);
			/* Write your code here */
			for (;;) {
				if (Packet_Get()) {
					Packet_Handle();
				}
				UART_Poll();
			}
		}


	/*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
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
