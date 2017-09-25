/*
 * analog.c
 *
 *      @date 11 Sep 2017
 *      @author 11986282
 *      @author 11970744
 */

// include header files with functions used for Lab4
#include "types.h"
#include "analog.h"
#include "Cpu.h"
#include "MK70F12.h"

bool Analog_Init(const uint32_t moduleClock)
{
  if (moduleClock == 0)
  {
    return false;
  }

  TSPIModule SPIModule;
  SPIModule.isMaster = true;
  SPIModule.continuousClock = false;
  SPIModule.inactiveHighClock = false;
  SPIModule.changedOnLeadingClockEdge = false;
  SPIModule.LSBFirst = false;
  SPIModule.baudRate = 1000000;

  if (!SPI_Init(&SPIModule, moduleClock))
    return false;

  SPI_SelectSlaveDevice(ANALOG_ADDRESS);

  /* Initialize Analog_Input */
  Analog_Input[0].value.l = 0;
  Analog_Input[1].value.l = 0;
  Analog_Input[0].oldValue.l = 0;
  Analog_Input[1].oldValue.l = 0;
  Analog_Input[0].putPtr = Analog_Input[0].values;
  Analog_Input[1].putPtr = Analog_Input[1].values;

  return success;
}

bool Analog_Get(const uint8_t channelNb)
{
  // something
}

/* END analog */
/*!
 ** @}
 */
