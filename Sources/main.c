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
#include "MyUART.h"
#include "Flash.h"
#include "types.h"
#include "LEDs.h"
#include "FTM.h"
#include "RTC.h"
#include "PIT.h"
#include "Analog.h"
#include "median.h"
#include "SPI.h"
#include "OS.h"

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

#define THREAD_STACK_SIZE 100
#define NB_LEDS 4

const uint32_t BaudRate = 38400;

static volatile uint16union_t* NvTowerNb; // Tower Number variable
static volatile uint16union_t* NvTowerMd; // Tower Mode variable

static 	TFTMChannel aFTMChannel;
static 	TSPIModule 	aSPIModule;
		TFIFO 		TX_FIFO, RX_FIFO;

TAnalogInput Analog_Input[ANALOG_NB_INPUTS];

OS_ECB* StartupSemaphore;

uint8_t ProtocolMode;

// Thread stacks
OS_THREAD_STACK(InitModuleThreadStack, THREAD_STACK_SIZE);

static uint32_t RTCThreadStacks[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
static uint32_t PITThreadStacks[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
static uint32_t FTMThreadStacks[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
static uint32_t PacketThreadStacks[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
//static uint32_t AnalogThreadStacks[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
static uint32_t UARTTxThreadStacks[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
static uint32_t UARTRxThreadStacks[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));




 static bool setSPIModule()
{
	aSPIModule.isMaster = true;
	aSPIModule.continuousClock = false;
	aSPIModule.inactiveHighClock = false;
	aSPIModule.changedOnLeadingClockEdge = false;
	aSPIModule.LSBFirst = false;
	aSPIModule.baudRate = 1000000;

	return true;
}


//static void AnalogThread(void* arg)
//{
//  for(;;)
//  {
//	  OS_SemaphoreWait(AnalogNb0Semaphore,0);
//
//		 //Analog_Input[channelNb].value.l = Analog_Input[channelNb].values[0];
//
//
//		if(ProtocolMode == 0)
//		{
//		  if (Analog_Input[0].oldValue.l != Analog_Input[0].value.l)
//		  {
//			MyPacket_Put(CMD_ANALOG_INPUT, 0x00, Analog_Input[0].value.s.Lo, Analog_Input[0].value.s.Hi);
//		  }
//		  if (Analog_Input[1].oldValue.l != Analog_Input[1].value.l)
//		  {
//			MyPacket_Put(CMD_ANALOG_INPUT, 0x01, Analog_Input[1].value.s.Lo, Analog_Input[1].value.s.Hi);
//		  }
//		}
//		else
//		{
//			MyPacket_Put(CMD_ANALOG_INPUT, 0x00, Analog_Input[0].value.s.Lo, Analog_Input[0].value.s.Hi);
//			MyPacket_Put(CMD_ANALOG_INPUT, 0x01, Analog_Input[1].value.s.Lo, Analog_Input[1].value.s.Hi);
//		}
//		Analog_Input[0].oldValue.l = Analog_Input[0].value.l;
//		Analog_Input[1].oldValue.l = Analog_Input[1].value.l;
//  }
//}
static void AnalogGetInput()
{
  for (int i = 0 ; i <= 1 ; i++)
    {
	  int16_t* dataPtr;
      Analog_Get(i, &Analog_Input[i].value);
      /* Wait */
      uint16_t count;
      for (count = 0; count < 100; count++);

      /* Then update analog value*/
      Analog_Input[i].value.l = Median_Filter(Analog_Input[i].values, 5);
      //Analog_Input[channelNb].value.l = Analog_Input[channelNb].values[0];
    }

    if(ProtocolMode == 0)
    {
      if (Analog_Input[0].oldValue.l != Analog_Input[0].value.l)
      {
        MyPacket_Put(CMD_ANALOG_INPUT, 0x00, Analog_Input[0].value.s.Lo, Analog_Input[0].value.s.Hi);
      }
      if (Analog_Input[1].oldValue.l != Analog_Input[1].value.l)
      {
    	  MyPacket_Put(CMD_ANALOG_INPUT, 0x01, Analog_Input[1].value.s.Lo, Analog_Input[1].value.s.Hi);
      }
    }
    else
    {
    	MyPacket_Put(CMD_ANALOG_INPUT, 0x00, Analog_Input[0].value.s.Lo, Analog_Input[0].value.s.Hi);
    	MyPacket_Put(CMD_ANALOG_INPUT, 0x01, Analog_Input[1].value.s.Lo, Analog_Input[1].value.s.Hi);
    }
    Analog_Input[0].oldValue.l = Analog_Input[0].value.l;
    Analog_Input[1].oldValue.l = Analog_Input[1].value.l;
}


/*! @brief Pit call back function
 *
 */
static void PITCallBack(void* arg)
{
  OS_ISREnter();

  //AnalogGetInput();
  OS_SemaphoreSignal(PITSemaphore);

  OS_ISRExit();
}

void PITThread(void* pData)//(void *arguments)
{
  for(;;)
  {
    OS_SemaphoreWait(PITSemaphore, 0);
    LEDs_Toggle(LED_GREEN);
    /*
    for (int i = 0 ; i < 2 ; i++)
    {
    	MyAnalog_Get(i);
    	Analog_Input[i].value.l = Median_Filter(Analog_Input[i].values, ANALOG_WINDOW_SIZE);
    }

    OS_SemaphoreSignal(AnalogNb0Semaphore);
    OS_SemaphoreSignal(AnalogNb1Semaphore);
    */
  }
}
//
//void AnalogThread0(void* arg)
//{
//	for(;;)
//	{
//		OS_SemaphoreWait(AnalogNb0Semaphore);
//		Median_Filter(Analog_Input[0],ANALOG_WINDOW_SIZE);
//		MyPacket_Put()
//	}
//}

//
//void processanalogthread(){
//	for(;;){
//		//OS_SemaphoreWait(AnalogProcess,0);
//		//median filtering
//		//send analog packet
//	}
//}



/*! @brief Initialises everything
 *  @param no argument required/ void
 */
static void RTCCallBack(void* arg)
{
  OS_ISREnter();

  OS_SemaphoreSignal(RTCSemaphore);

  OS_ISRExit();

}

static void RTCThread(void* arg)
{
  for(;;)
  {
	  OS_SemaphoreWait(RTCSemaphore, 0);
	  uint8_t hours, minutes, seconds;
	  RTC_Get(&hours, &minutes, &seconds);
	  LEDs_Toggle(LED_YELLOW);
	 // (void)MyPacket_Put(CMD_TOWER_TIME, hours, minutes, seconds);
  }
}

/*! @brief FTM call back function. Turns on blue LED
 *
 */
static void FTMCallBack(void* arg)
{
  OS_ISREnter();
  OS_SemaphoreSignal(FTMSemaphore);
  OS_ISRExit();
}

static void FTMThread(void* arg)
{
	for(;;)
	{
	  OS_SemaphoreWait(FTMSemaphore,0);
	  LEDs_Off(LED_BLUE);
	}
}

static void TxThread(void* arg)
{

	for(;;)
	{
		OS_SemaphoreWait(UARTTxSemaphore,0);
		MyFIFO_Get(&TX_FIFO, &UART2_D);
		UART2_C2 |= UART_C2_TIE_MASK;

	}
}

static void RxThread(void* arg)
{
	for(;;)
	{
		OS_SemaphoreWait(UARTRxSemaphore,0);
		MyFIFO_Put(&RX_FIFO, MyUART_TempRxData);
	}
}

/*! @brief Sends startup packets. Should be v 1.0
 *
 * @bool returns true if packet send is successful
 */
static bool StartUpPackets()
{
  // gonna use this to check is sending startup packets is successful
  bool success;
  OS_SemaphoreWait(StartupSemaphore,0);

  // Send void packet
  MyPacket_Put(0x76, 0x6F, 0x69, 0x64);
  // start up value of 0x04 and the rest of the packets are zero
  success = MyPacket_Put(CMD_STARTUP, 0x00, 0x00, 0x00);
  // signifies Tower V 1.0
  success &= MyPacket_Put(CMD_TOWER_VER, 0x76, 0x01, 0x00);
  // Whatever was saved/written in flash
  success &= MyPacket_Put(CMD_TOWER_NB, 0x01, (*NvTowerNb).s.Lo, (*NvTowerNb).s.Hi);
  // Whatever was saved/written  in flash
  success &= MyPacket_Put(CMD_TOWER_MD, 0x01, (*NvTowerMd).s.Lo, (*NvTowerMd).s.Hi);
  /*Protocol command, by default protocol mode is 0*/
  success &= MyPacket_Put(CMD_PROTOCOL_MODE, 0x01, ProtocolMode, 0x00);
  // Return success if all the packet put ran were successful
  return success;
}

void TowerStart()
{
  bool success = false;

  /*Initialise Packet.c clockrate, flash, LEDs, PIT, RTC and FTM. Will pass 0x01 to success bool if all is gewd */
  success = MyPacket_Init(BaudRate, CPU_BUS_CLK_HZ) && Flash_Init() && LEDs_Init() && PIT_Init(CPU_BUS_CLK_HZ, PITCallBack, NULL)
            && RTC_Init(RTCCallBack, NULL) && FTM_Init() && Analog_Init(CPU_BUS_CLK_HZ) /*&& setSPIModule()*/;

  if (success)
  {
	LEDs_On(LED_ORANGE);
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
  }

static void InitModulesThread(void* pData)
{
  OS_DisableInterrupts();

  // Initialise PIT
  // PIT_Init(CPU_BUS_CLK_HZ,NULL,NULL);
  TowerStart();

  // Start modules
	// PIT
  PIT_Set(500000000, true);
  PIT_Enable(true);

    // FTM
  FTM_StartTimer(&aFTMChannel);

  PITSemaphore    = OS_SemaphoreCreate(0);
  RTCSemaphore 	  = OS_SemaphoreCreate(0);
  FTMSemaphore    = OS_SemaphoreCreate(0);
  PacketSemaphore = OS_SemaphoreCreate(1);
  StartupSemaphore= OS_SemaphoreCreate(0);

  OS_EnableInterrupts();

  OS_SemaphoreSignal(StartupSemaphore);
  StartUpPackets();

  // We only do this once - so delete this thread
  OS_ThreadDelete(OS_PRIORITY_SELF);
}

/*! @brief Initialises everything
 *  @return returns true if everything initialises successfully
 */


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
  return MyPacket_Put(Packet_Command, 0x76, 0x01, 0x00);
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
      return MyPacket_Put(CMD_TOWER_NB, 0x01, (*NvTowerNb).s.Lo, (*NvTowerNb).s.Hi);
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
    return MyPacket_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, *data);
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
      return MyPacket_Put(CMD_TOWER_MD, 0x01, (*NvTowerMd).s.Lo, (*NvTowerMd).s.Hi);
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
	  MyPacket_Put(Packet_Command, 0x01, ProtocolMode, 0x00);
  else if (Packet_Parameter1 == 0x02)
  {
    //range checking on param2
    ProtocolMode = Packet_Parameter2;
    return true;
  }
  return false;
}

void Packet_Handle()
{
//  EnterCritical();
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
      MyPacket_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3); // Put packets
    }
  }
 // ExitCritical();
}

static void PacketThread(void* arg)
{
  for(;;)
  {
	if (MyPacket_Get())
	{
	  FTM_StartTimer(&aFTMChannel);
	  LEDs_On(LED_BLUE);
	  Packet_Handle();
    }
  }
}


/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */
	OS_ERROR error;
  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  OS_Init(CPU_BUS_CLK_HZ, false);

    // Create module initialisation thread
  error = OS_ThreadCreate(InitModulesThread,NULL, &InitModuleThreadStack[THREAD_STACK_SIZE - 1], 0); // Highest priority
  	// Create other threads
  error = OS_ThreadCreate(PITThread, &PITSemaphore, &PITThreadStacks[THREAD_STACK_SIZE - 1],4);
  error = OS_ThreadCreate(RTCThread, &RTCSemaphore, &RTCThreadStacks[THREAD_STACK_SIZE - 1],5);
  error = OS_ThreadCreate(FTMThread, &FTMSemaphore, &FTMThreadStacks[THREAD_STACK_SIZE - 1],6);
  error = OS_ThreadCreate(PacketThread, &PacketSemaphore, &PacketThreadStacks[THREAD_STACK_SIZE - 1],7);
  //error = OS_ThreadCreate(AnalogThread, &AnalogNb0Semaphore, &AnalogThreadStacks[THREAD_STACK_SIZE - 1],3);
  error = OS_ThreadCreate(RxThread, &UARTRxSemaphore, &UARTRxThreadStacks[THREAD_STACK_SIZE - 1],2);
  error = OS_ThreadCreate(TxThread, &UARTTxSemaphore, &UARTTxThreadStacks[THREAD_STACK_SIZE - 1],1);

  OS_Start();


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
