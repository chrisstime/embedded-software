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

// Define packet commands
#define CMD_STARTUP 0x04
#define CMD_FLASH_PRG 0x07
#define CMD_FLASH_READ 0x08
#define CMD_TOWER_VER 0x09
#define CMD_TOWER_NB 0x0B
#define CMD_TOWER_MODE 0x0D
#define TOWER_DEFAULT_VALUE 0x188A
#define PACKET_ACK_MASK = 0x80

const uint8_t BaudRate = 115200;

static volatile uint16union_t* NvTowerNb;
static volatile uint16union_t* NvTowerMd;

void Tower_Version()
{
	Packet_Put(PacketNoAck, 0x76, 0x01, 0x00);
}

void Tower_Startup()
{
	Packet_Put(0x04, 0x00, 0x00, 0x00);
	Tower_Version();
	//Packet_Put(0x0B, 0x01, TOWER_DEFAULT_VALUE.s.Lo, TOWER_DEFAULT_VALUE.s.Hi);
    if ((*NvTowerNb).l == 0x0FFFF) {
        Flash_Write16((uint16_t*)NvTowerNb, TOWER_DEFAULT_VALUE);
    }
    Packet_Put(0x0B, 0x01, (*NvTowerNb).s.Lo, (*NvTowerNb).s.Hi);
	//check the tower number and mode bytes - program default.
}

void Check_Ack(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
	if ((Packet_Command & PACKET_ACK_MASK) == PACKET_ACK_MASK)
        Packet_Put(command, parameter1, parameter2, parameter3);
}

//check each bit with the packet_position to ensure that the packets are aligned.
void Packet_Handle()
{
    uint8_t packetNoAck = Packet_Command & ~PACKET_ACK_MASK;
	uint8_t data;

	switch (Packet_Command) {
	case CMD_STARTUP:	//Tower startup
		Tower_Startup();
		break;

	case CMD_TOWER_VER:	//Special Tower version
		Tower_Version();
		break;
//new lab 2 commands
	case CMD_TOWER_NB:
		if (Packet_Parameter1 == 0x02)
        {
            Flash_Write16((uint16_t*)NvTowerNb, Packet_Parameter23);
		}
        else if (Packet_Parameter1 == 0x01)
        {
            if (Packet_Parameter2 == 0x00 && Packet_Parameter3 == 0x00)
            {
                Packet_Put(CMD_TOWER_NB, 0x01, (*NvTowerNb).s.Lo, (*NvTowerNb).s.Hi);
            }
        }
		break;

	case CMD_FLASH_PRG:
		if (Packet_Parameter1 == CMD_FLASH_PRG && Packet_Parameter2 == 0x00 && Packet_Parameter3 == 0x00)
        {
            uint32_t* data = (uint32_t*)(FLASH_DATA_START + Packet_Parameter1);
            Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, *data);
        }
		break;

	case CMD_FLASH_READ:
        if (Packet_Parameter1 == CMD_FLASH_READ && Packet_Parameter2 == 0x00)
            Flash_Erase();
        else if (Packet_Parameter1 < 0x08)
        {
            uint32_t *addressFlash = (uint32_t *)(FLASH_DATA_START + Packet_Parameter1);
            Flash_Write8((uint8_t *) addressFlash, Packet_Parameter3);
        }


		break;

	case CMD_TOWER_MODE:
		if (Packet_Parameter1 == 0x01)
		{
			if (Packet_Parameter2 == 0x00 && Packet_Parameter3 == 0x00)
				Packet_Put(CMD_TOWER_MODE, 0x01, (*NvTowerMd).s.Lo, (*NvTowerMd).s.Hi);
		}
		if(Packet_Parameter1 == 0x02)
		{
            Flash_Write16((uint16_t*)&NvTowerMd, (const uint16_t)Packet_Parameter23);
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

	if (Packet_Init(BAUD_RATE, CPU_BUS_CLK_HZ) && Flash_Init() && LEDs_Init())
    {
        Tower_Startup();
		LEDs_On(LED_ORANGE);
		LEDs_On(LED_BLUE);
			/* Write your code here */
			for (;;)
            {
				if (Packet_Get())
                {
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
