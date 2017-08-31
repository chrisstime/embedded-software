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

// Define packet commands and other things that need to be defined
#define CMD_STARTUP 0x04
#define CMD_FLASH_PRG 0x07
#define CMD_FLASH_READ 0x08
#define CMD_TOWER_VER 0x09
#define CMD_TOWER_NB 0x0B
#define CMD_TOWER_MODE 0x0D
#define TOWER_DEFAULT_VALUE 0x188A
#define PACKET_ACK_MASK 0x80

const uint32_t BaudRate = 115200;

static volatile uint16union_t* NvTowerNb;
static volatile uint16union_t* NvTowerMd;

bool TowerVersion(void)
{
	if (Packet_Put((Packet_Command & ~PACKET_ACK_MASK), 0x76, 0x01, 0x00))
    return true;
  return false;
}

bool TowerStartup(void)
{
	Packet_Put(0x04, 0x00, 0x00, 0x00);
	Tower_Version();
	Flash_AllocateVar((volatile void**)&NvTowerNb, sizeof(*NvTowerNb)); //returns bool
    if ((*NvTowerNb).l == 0xFFFF)
    {
      if (Flash_Write16((uint16_t*)NvTowerNb, TOWER_DEFAULT_VALUE))
      {
        uint64_t initialiseFlash = _FP(FLASH_DATA_START);
      }
    }

//	Flash_AllocateVar((volatile void**)&NvTowerMd, sizeof(*NvTowerMd)); //returns bool
//    if ((*NvTowerMd).l == 0xFFFF)
//    {
//      Flash_Write16((uint16_t*)NvTowerMd, TOWER_DEFAULT_VALUE);
//    }

  Packet_Put(0x0B, 0x01, (*NvTowerNb).s.Lo, (*NvTowerNb).s.Hi);
	//check the tower number and mode bytes - program default.
}

bool TowerNb(void)
{
  if (Packet_Parameter1 == 0x02)
  {
    if (Flash_Write16((uint16_t*)NvTowerNb, Packet_Parameter23))
      return true;
  }
  else if (Packet_Parameter1 == 0x01)
  {
    if (Packet_Parameter2 == 0x00 && Packet_Parameter3 == 0x00)
    {
      if ((Packet_Put(CMD_TOWER_NB, 0x01, (*NvTowerNb).s.Lo, (*NvTowerNb).s.Hi)))
        return true;
    }
  }
  return false;
}

bool FlashPrg(void)
{
  if (Packet_Parameter1 == CMD_FLASH_PRG && Packet_Parameter2 == 0x00 && Packet_Parameter3 == 0x00)
  {
    uint32_t* data = (uint32_t*)(FLASH_DATA_START + Packet_Parameter1);
    if (Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, *data))
      return true;
  }
  return false;
}

bool FlashRead(void)
{
  if (Packet_Parameter1 == CMD_FLASH_READ && Packet_Parameter2 == 0x00)
  {
    Flash_Erase();
    return true;
  }
  else if (Packet_Parameter1 < 0x08)
  {
    uint32_t *addressFlash = (uint32_t *)(FLASH_DATA_START + Packet_Parameter1);
    if (Flash_Write8((uint8_t *) addressFlash, Packet_Parameter3))
      return true;
  }
  return false;
}

bool CheckAck(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
	if ((command & PACKET_ACK_MASK) == PACKET_ACK_MASK)
    {
      if (Packet_Put(command, parameter1, parameter2, parameter3))
        return true;
    }
    return false;
}

bool TowerMd(void)
{
  if (Packet_Parameter1 == 0x01)
  {
    if (Packet_Parameter2 == 0x00 && Packet_Parameter3 == 0x00)
      if (Packet_Put(CMD_TOWER_MODE, 0x01, (*NvTowerMd).s.Lo, (*NvTowerMd).s.Hi))
        return true;
  }
  if(Packet_Parameter1 == 0x02)
  {
    if (Flash_Write16((uint16_t*)&NvTowerMd, (const uint16_t)Packet_Parameter23))
      return true;
  }
  return false;
}

//check each bit with the packet_position to ensure that the packets are aligned.
void Packet_Handle()
{
  uint8_t PacketNoAck = Packet_Command & ~PACKET_ACK_MASK;
  bool success;

	switch (PacketNoAck)
    {
      case CMD_STARTUP:	//Tower startup
        success = TowerStartup();
        break;

      case CMD_TOWER_VER:	//Special Tower version
		    success = TowerVersion();
		    break;

            //new lab 2 commands
      case CMD_TOWER_NB:
		    success = TowerNb();
		    break;

      case CMD_FLASH_PRG:
        success = FlashPrg();
        break;

      case CMD_FLASH_READ:
        success = FlashRead();
		    break;

      case CMD_TOWER_MODE:
		    success = TowerMd();
		  break;
    }

    if (success)
    {
      if (CheckAck(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3))
        return true;
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

	if (Packet_Init(BaudRate, CPU_BUS_CLK_HZ) && Flash_Init() && LEDs_Init())
  {
    TowerStartup();
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
