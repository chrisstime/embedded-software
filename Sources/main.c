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
#include "FTM.h"

// Define packet commands and other things that need to be defined
#define CMD_STARTUP 0x04
#define CMD_FLASH_PRG 0x07
#define CMD_FLASH_READ 0x08
#define CMD_TOWER_VER 0x09
#define CMD_TOWER_NB 0x0B
#define CMD_TOWER_MD 0x0D
#define CMD_TOWER_TIME 0x0C
#define TOWER_DEFAULT_VALUE 0x188A /* the student number here is 9862*/
#define TOWER_DEFAULT_MD 0x0001 /* default tower mode is 1*/
#define PACKET_ACK_MASK 0x80

const uint32_t BaudRate = 115200;

static volatile uint16union_t* NvTowerNb; // Tower Number variable
static volatile uint16union_t* NvTowerMd; // Tower Mode variable

static TFTMChannel aFTMChannel;

/*! @brief Pit call back function
 *
 */
static void PITCB()
{
  LEDs_Toggle(LED_YELLOW)
}

/*! @brief Initialises everything
 *  @return returns true if everything initialises successfully
 */
static void RTCCB()
{
  uint8_t hours, minutes, seconds;
  RTC_Get(&hours, &minutes, &seconds);
  LEDs_Toggle(LED_YELLOW);
  Packet_Put(CMD_TOWER_TIME ,hours,minutes,seconds);
}

/*! @brief Initialises everything
 *  @return returns true if everything initialises successfully
 */
static bool TowerStartup()
{
  bool success = false;

  /*Initialise Packet.c clockrate, flash, LEDs, PIT, RTC and FTM. Will pass 0x01 to success bool if all is gewd */
  success = Packet_Init(BaudRate, CPU_BUS_CLK_HZ) && Flash_Init() && LEDs_Init() && PIT_Init(PITCB(), Null) && RTC_Init(RTCCB(), Null) FTM_Init();

  if (success)
  {
    PIT_Set(1000000000, true);
    PIT_Enable(true);

    // Initialise FTM
    aFTMChannel.channelNb	= 0x00;
    aFTMChannel.delayCount = 24414;
    aFTMChannel.ioType.outputAction	= TIMER_OUTPUT_TOGGLE;
    //  aFTMChannel.ioType.inputDetection 	= 2;
    aFTMChannel.timerFunction = TIMER_FUNCTION_OUTPUT_COMPARE;
    aFTMChannel.userArguments = 0;
    aFTMChannel.userFunction 	= FTM_BLED_Off;
    FTM_Set(&aFTMChannel);

    /* Allocate memory in flash will return true if successful*/
	  success &= Flash_AllocateVar((volatile void**)&NvTowerNb, sizeof(*NvTowerNb));

    /* Check if there's no previously set value in NvTowerNb in flash memory*/
	  if (success && (*NvTowerNb).l == 0xFFFF)
	  {
      /* If it's empty, just allocate it the default value which is our student number*/
		  success &= Flash_Write16((uint16_t*)NvTowerNb, TOWER_DEFAULT_VALUE);

      /* Initialise flash just because */
		  uint64_t initialiseFlash = _FP(FLASH_DATA_START);
	  }

    /* Also allocate space in memory for the Tower mode */
	  success &= Flash_AllocateVar((volatile void**)&NvTowerMd, sizeof(*NvTowerMd));

    /* Check if there's no previously set value in NvTowerMd in flash memory*/
	  if (success && (*NvTowerMd).l == 0xFFFF)
	  {
      /* If it's empty, just allocate it the default value 1 for tower mode*/
		  success &= Flash_Write16((uint16_t*)NvTowerMd, TOWER_DEFAULT_MD);

      /* Initialise flash just because */
		  uint64_t initialiseFlash = _FP(FLASH_DATA_START);
	  }
  }
  /*If any one if these processes fails, return false and Packet Handle and UART polling will not run */
  return success;
}

static bool TowerVersion()
{
  // returns v 1.0
	return Packet_Put(Packet_Command, 0x76, 0x01, 0x00);
}

static bool StartUpPackets()
{
  // gonna use this to check is sending startup packets is successful
  bool success;

  // start up value of 0x04 and the rest of the packets are zero
  success = Packet_Put(CMD_STARTUP, 0x00, 0x00, 0x00);
  // signifies Tower V 1.0
  success &= Packet_Put(CMD_TOWER_VER, 0x76, 0x01, 0x00);
  // Whatever was saved/written in flash
  success &= Packet_Put(CMD_TOWER_NB, 0x01, (*NvTowerNb).s.Lo, (*NvTowerNb).s.Hi);
  // Whatever was saved/written  in flash
  success &= Packet_Put(CMD_TOWER_MD, 0x01, (*NvTowerMd).s.Lo, (*NvTowerMd).s.Hi);

  // Return success if all the packet put ran were successful
  return success;
}

static bool TowerNb()
{
  if (Packet_Parameter1 == 0x02) // if Parameter is set to 2
  {
    // Write to flash whatever packet's being passed through from PC
    return Flash_Write16((uint16_t*)NvTowerNb, Packet_Parameter23);
  }
  else if (Packet_Parameter1 == 0x01) // if Parameter is set to 1
  {
    if (Packet_Parameter2 == 0x00 && Packet_Parameter3 == 0x00)
    {
      // Return either default tower value if nothing's written in flash or saved value from flash mem
      return Packet_Put(CMD_TOWER_NB, 0x01, (*NvTowerNb).s.Lo, (*NvTowerNb).s.Hi);
    }
  }
  return false; // return false if unsuccessful :(
}

static bool FlashPrg()
{
	if (Packet_Parameter1 == CMD_FLASH_READ && Packet_Parameter2 == 0x00)
	{
		return Flash_Erase(); // Erases the stuff in flash
	}
	else if (Packet_Parameter1 < 0x08)
	{
    uint32_t *addressFlash = (uint32_t *)(FLASH_DATA_START + Packet_Parameter1);
    // writes to flash
    return Flash_Write8((uint8_t *) addressFlash, Packet_Parameter3);
	}
	return false; // return false if unsuccessful :(
}

static bool FlashRead()
{
  if (Packet_Parameter1 < 0x08 && Packet_Parameter2 == 0x00 && Packet_Parameter3 == 0x00)
  {
    uint32_t* data = (uint32_t*)(FLASH_DATA_START + Packet_Parameter1);
    // reads from flash
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

void Packet_Handle()
{
  EnterCritical();
  Packet_Command &= ~PACKET_ACK_MASK;

  bool ack = Packet_Command & PACKET_ACK_MASK;
  bool success = false;

	switch (Packet_Command)
  {
    case CMD_STARTUP:
      success = StartUpPackets();
      break;

    case CMD_TOWER_VER:
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
  ExitCritical();
}

void FTM_BLED_On(void (*fpointer))
{
  LEDs_On(LED_BLUE);
}

void FTM_BLED_Off(void (*fpointer))
{
  LEDs_Off(LED_BLUE);
}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
	/* Write your local variable definition here */

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
	/*** End of Processor Expert internal initialization.                    ***/

  __DI();

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
			//UART_Poll(); // UART Polling
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
