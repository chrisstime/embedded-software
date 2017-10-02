/* ###################################################################
 **     Filename    : main.c
 **     Project     : Lab4
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
 *   @date 11 Sep 2017
 *   @author 11986282
 *   @author 11970744
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
#include "RTC.h"
#include "PIT.h"
#include "analog.h"
#include "median.h"
#include "SPI.h"

// Define packet commands
#define CMD_STARTUP 0x04
#define CMD_FLASH_PRG 0x07
#define CMD_FLASH_READ 0x08
#define CMD_TOWER_VER 0x09
#define CMD_TOWER_NB 0x0B
#define CMD_TOWER_MD 0x0D
#define CMD_TOWER_TIME 0x0C
#define CMD_ANALOG_INPUT 0x50
#define CMD_PROTOCOL_MODE 0x0A
// Define some default values like student number and tower number etc.
#define TOWER_DEFAULT_VALUE 0x188A /* the student number here is 9862*/
#define TOWER_DEFAULT_MD 0x0001 /* default tower mode is 1*/
#define PACKET_ACK_MASK 0x80

const uint32_t BaudRate = 115200;

static volatile uint16union_t* NvTowerNb; // Tower Number variable
static volatile uint16union_t* NvTowerMd; // Tower Mode variable

static TFTMChannel aFTMChannel;
uint8_t ProtocolMode;

/*! @brief Pit call back function
 *
 */
static void PITCallBack(void* arg)
{
  for (int i = 0 ; i < 2 ; i++)
  {
    Analog_Get(i);

    /* Wait */
    for (int count = 0; count < 100; count++);

    /* Then update analog value*/
    Analog_Input[channelNb].value.l = Median_Filter(Analog_Input[channelNb].values, ANALOG_WINDOW_SIZE);
  }

  /* if user sets to asynchronous */
  if(ProtocolMode == 0)
  {
    /* Compare if the analog value changes then PackPut the new value. So it will only transmits packets if there is a change */
    if (Analog_Input[0].oldValue.l != Analog_Input[0].value.l)
    {
      Packet_Put(CMD_ANALOG_INPUT, 0x00, Analog_Input[0].value.s.Lo, Analog_Input[0].value.s.Hi);
    }
    /* If the analog value changes then PackPut the new value. This occurs for falling edge and rising edge*/
    if (Analog_Input[1].oldValue.l != Analog_Input[1].value.l)
    {
      Packet_Put(CMD_ANALOG_INPUT, 0x01, Analog_Input[1].value.s.Lo, Analog_Input[1].value.s.Hi);
    }
  }
  else /* if synchronous */
  {
     Packet_Put(CMD_ANALOG_INPUT, 0x00, Analog_Input[0].value.s.Lo, Analog_Input[0].value.s.Hi);
     Packet_Put(CMD_ANALOG_INPUT, 0x01, Analog_Input[1].value.s.Lo, Analog_Input[1].value.s.Hi);
  }

  /* transfer value to oldValue  */
  Analog_Input[0].oldValue.l = Analog_Input[0].value.l;
  Analog_Input[1].oldValue.l = Analog_Input[1].value.l;
}

/*! @brief Initialises everything
 *  @return returns true if everything initialises successfully
 */
static void RTCCallBack(void* arg)
{
  uint8_t hours, minutes, seconds;
  RTC_Get(&hours, &minutes, &seconds);
  LEDs_Toggle(LED_YELLOW);
  (void)Packet_Put(CMD_TOWER_TIME, hours, minutes, seconds);
}

/*! @brief FTM call back function. Turns on blue LED
 *
 */
static void FTMCallBack(void* arg)
{
  LEDs_On(LED_BLUE);
}

/*! @brief Initialises everything
 *  @return returns true if everything initialises successfully
 */
static bool TowerStartup()
{
  bool success = false;

  /*Initialise Packet.c clockrate, flash, LEDs, PIT, RTC and FTM. Will pass 0x01 to success bool if all is gewd */
  success = Packet_Init(BaudRate, CPU_BUS_CLK_HZ) && Flash_Init() && LEDs_Init() && PIT_Init(CPU_BUS_CLK_HZ, PITCallBack, NULL) 
            && RTC_Init(RTCCallBack, NULL) && FTM_Init() && Analog_Init(CPU_BUS_CLK_HZ);

  if (success)
  {
      /* Start up the PIT timer using a period of 500 ms */
          PIT_Set(10000000, true);
          PIT_Enable(true);

          /* Initialize FTM Channel 0 */
          aFTMChannel.channelNb = 0x00;
          aFTMChannel.delayCount = 24414;
          aFTMChannel.timerFunction = TIMER_FUNCTION_OUTPUT_COMPARE;
          aFTMChannel.ioType.outputAction = TIMER_OUTPUT_HIGH;
          aFTMChannel.userFunction = FTMCallBack;
          aFTMChannel.userArguments = NULL;
          success &= FTM_Set(&aFTMChannel);

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

  /*This is set to be asynchronous as the default*/
  ProtocolMode = 0;

  /*If any one if these processes fails, return false and Packet Handle and UART polling will not run */
  return success;
}

/*! @brief Sets time everything
 *  @return returns true if everything time is set successfully in RTC
 */
static bool SetTime()
{
  if (Packet_Parameter1 <= 23 && Packet_Parameter2 <= 59 && Packet_Parameter3 <= 59)
  {
     RTC_Set(Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
     return true;
  }
  return false;
}

/*! @brief Sends tower version packets
 *
 *  @bool returns true if packet send is successful
 */
static bool TowerVersion()
{
  // returns v 1.0
  return Packet_Put(Packet_Command, 0x76, 0x01, 0x00);
}

/*! @brief Sends startup packets. Should be v 1.0
 *
 * @bool returns true if packet send is successful
 */
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
  /*Protocol command, by default protocol mode is 0*/
  success &= Packet_Put(CMD_PROTOCOL_MODE, 0x01, ProtocolMode, 0x00);
  // Return success if all the packet put ran were successful
  return success;
}

/*! @brief Sends Tower number packets. Should be 6282.
 *
 * @bool returns true if packet send is successful
 */
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

/*! @brief Programs writes or erases to flash
 *
 * @bool returns true if Flash erase or flash write is successful
 */
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

/*! @brief Programs writes or erases to flash
 *
 * @bool returns true if Flash erase or flash write is successful
 */
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

/*! @brief Sends Tower Mode or writes to flash the packets you set
 *
 * @bool returns true if flash tower mode was successfully written
 */
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

static bool Protocol_Mode()
{
  if (Packet_Parameter1 == 0x01)
    Packet_Put(Packet_Command, 0x01, ProtocolMode, 0x00);
  else if (Packet_Parameter1 == 0x02)
  {
    ProtocolMode = Packet_Parameter2;
    return true;
  }
  return false;
}

void Packet_Handle()
{
  EnterCritical();
  Packet_Command &= ~PACKET_ACK_MASK; // removes the acknowledgement mask from the Packet_Command

  bool ack = Packet_Command & PACKET_ACK_MASK;
  bool success = false; // command success checker

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

    case CMD_TOWER_TIME:
      success = SetTime();
    break;

    case CMD_PROTOCOL_MODE:
      success = Protocol_Mode(); //create this command
    break;

    default:
      break;
  }

  if (ack) // If acknowledgment bit is set
  {
    if (success) // if the switch case run was successful
    {
      Packet_Command |= PACKET_ACK_MASK; // return Packet_Command to have acknowledgement mask included
      Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3); // Put packets
    }
  }
  ExitCritical();
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
    LEDs_On(LED_ORANGE);
    __EI();
    for (;;) /*to loop forever*/
    {
       if (Packet_Get())
       {
         FTM_StartTimer(&aFTMChannel);
         LEDs_On(LED_BLUE);
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
