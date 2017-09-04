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

// Define packet commands and other things that need to be defined
#define CMD_STARTUP 0x04
#define CMD_FLASH_PRG 0x07
#define CMD_FLASH_READ 0x08
#define CMD_TOWER_VER 0x09
#define CMD_TOWER_NB 0x0B
#define CMD_TOWER_MD 0x0D
#define TOWER_DEFAULT_VALUE 0x188A /* the student number here is 9862*/
#define TOWER_DEFAULT_MD 0x0001 /* default tower mode is 1*/
#define PACKET_ACK_MASK 0x80

const uint32_t BaudRate = 115200;

static volatile uint16union_t* NvTowerNb;
static volatile uint16union_t* NvTowerMd;

static bool TowerStartup()
{
  bool success = false;

  success = Packet_Init(BaudRate, CPU_BUS_CLK_HZ) && Flash_Init() && LEDs_Init(); /*Initialise Packet.c clockrate, then flash and then the LEDs */

  if (success)
  {
	  success &= Flash_AllocateVar((volatile void**)&NvTowerNb, sizeof(*NvTowerNb)); /* Allocate memory in flash will return true if successful*/

	  if (success && (*NvTowerNb).l == 0xFFFF) /* Check if there's no previously set value in NvTowerNb in flash memory*/
	  {
		  success &= Flash_Write16((uint16_t*)NvTowerNb, TOWER_DEFAULT_VALUE); /* If it's empty, just allocate it the default value which is our student number*/
		  uint64_t initialiseFlash = _FP(FLASH_DATA_START); /* Initialise flash */
	  }

	  success &= Flash_AllocateVar((volatile void**)&NvTowerMd, sizeof(*NvTowerMd)); /*Also allocate space in memory for the Tower mode*/

	  if (success && (*NvTowerMd).l == 0xFFFF) /* Check if there's no previously set value in NvTowerMd in flash memory*/
	  {
		  success &= Flash_Write16((uint16_t*)NvTowerMd, TOWER_DEFAULT_MD); /* If it's empty, just allocate it the default value 1 for tower mode*/
		  uint64_t initialiseFlash = _FP(FLASH_DATA_START); /* Initialise flash */
	  }
  }
  return success; /*If any one if these processes fails, return false and Packet Handle and UART polling will not run */
}

static bool TowerVersion()
{
	return Packet_Put(Packet_Command, 0x76, 0x01, 0x00);
}

static bool StartUpPackets()
{
  bool success;
  success = Packet_Put(CMD_STARTUP, 0x00, 0x00, 0x00); // start up value of 0x04 and the rest of the packets are zero
  success &= Packet_Put(CMD_TOWER_VER, 0x76, 0x01, 0x00); // signifies Tower V 1.0
  success &= Packet_Put(CMD_TOWER_NB, 0x01, (*NvTowerNb).s.Lo, (*NvTowerNb).s.Hi); // Whatever was saved in flash
  success &= Packet_Put(CMD_TOWER_MD, 0x01, (*NvTowerMd).s.Lo, (*NvTowerMd).s.Hi); // Whatever was saved in flash
  return success; // Return success if all the packet put ran were successful
}

static bool TowerNb()
{
  if (Packet_Parameter1 == 0x02) // if Parameter is set to 2
  {
    return Flash_Write16((uint16_t*)NvTowerNb, Packet_Parameter23); // Write to flash whatever packet's being passed through from PC
  }
  else if (Packet_Parameter1 == 0x01) // if Parameter is set to 1
  {
    if (Packet_Parameter2 == 0x00 && Packet_Parameter3 == 0x00)
    {
      return Packet_Put(CMD_TOWER_NB, 0x01, (*NvTowerNb).s.Lo, (*NvTowerNb).s.Hi); // Return either default tower value if nothing's written in flash or saved value from flash mem
    }
  }
  return false; // return false if unsuccessful :(
}

static bool FlashPrg()
{
	if (Packet_Parameter1 == CMD_FLASH_READ && Packet_Parameter2 == 0x00)
	{
		return Flash_Erase();
	}
	else if (Packet_Parameter1 < 0x08)
	{
	    uint32_t *addressFlash = (uint32_t *)(FLASH_DATA_START + Packet_Parameter1);
	    return Flash_Write8((uint8_t *) addressFlash, Packet_Parameter3);
	}
	return false; // return false if unsuccessful :(
}

static bool FlashRead()
{
  if (Packet_Parameter1 < 0x08 && Packet_Parameter2 == 0x00 && Packet_Parameter3 == 0x00)
  {
      uint32_t* data = (uint32_t*)(FLASH_DATA_START + Packet_Parameter1);
      return Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, *data);
  }
  return false; // return false if unsuccessful :(
}

static bool TowerMd()
{
  if (Packet_Parameter1 == 0x01)
  {
    if (Packet_Parameter2 == 0x00 && Packet_Parameter3 == 0x00)
      return Packet_Put(CMD_TOWER_MD, 0x01, (*NvTowerMd).s.Lo, (*NvTowerMd).s.Hi);
  }
  if(Packet_Parameter1 == 0x02)
  {
    return Flash_Write16((uint16_t*)&NvTowerMd, Packet_Parameter23);
  }
  return false; // return false if unsuccessful :(
}

/*check each bit with the packet_position to ensure that the packets are aligned.*/
void Packet_Handle()
{
  Packet_Command &= ~PACKET_ACK_MASK;

  bool ack = Packet_Command & PACKET_ACK_MASK;
  bool success = false;

	switch (Packet_Command)
  {
    case CMD_STARTUP:	/*Tower startup*/
      success = StartUpPackets();
      break;

    case CMD_TOWER_VER:	/*Special Tower version*/
      success = TowerVersion();
      break;

    case CMD_TOWER_NB:
      success = TowerNb();
      break;

    case CMD_FLASH_PRG:
      success = FlashPrg();
      break;

    case CMD_FLASH_READ:
      success = FlashRead();
      break;

    case CMD_TOWER_MD:
      success = TowerMd();
      break;

    default:
      break;
  }

  if (ack) // If acknowledgement bit is set
  {
    if (success) // if the switch case run was successful
    {
      Packet_Command |= PACKET_ACK_MASK; // return Packet_Command to have acknowledgement mask included
      Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3); // Put packets
    }
  }
}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
	/* Write your local variable definition here */

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
	/*** End of Processor Expert internal initialization.                    ***/

	if (TowerStartup() && StartUpPackets()) /* initialises everything, check if Flash, LED and tower was started up successfully */
	{
	    /*Turns on the LED lights to indicate that TowerStartup() was successful */
		LEDs_On(LED_ORANGE);
		LEDs_On(LED_BLUE);


		for (;;) /*to loop forever*/
		{
			if (Packet_Get())
			{
				Packet_Handle();
			}
			UART_Poll(); // UART Polling
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
