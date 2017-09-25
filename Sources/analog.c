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

  SPI_SelectSlaveDevice(7);

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
  uint16_t inputData;

  /* data to be transmitted to SPI to change depending on channel number chosen*/
  switch (channelNb)
  {
    case 0:
      inputData = 0x8400;
      break;
    case 1:
      inputData = 0xC400;
      break;
    default:
      return false;
  }

  /* Point to the location of the next sample to take */
  if (Analog_Input[channelNb].putPtr == &(Analog_Input[channelNb].values[ANALOG_WINDOW_SIZE - 1]))
  {
    Analog_Input[channelNb].putPtr = Analog_Input[channelNb].values;
  }
  else
  {
    (Analog_Input[channelNb].putPtr)++;
  }

  /* Read input from analog input channel into the sliding window */
  SPI_Exchange(inputData, Analog_Input[channelNb].putPtr);

  /* Wait for conversion */
  uint8_t i;
  for (i = 0; i < 100; i++)
  {
  }

  /* Update the current 'processed' analog value */
  Analog_Input[channelNb].value.l = Median_Filter(Analog_Input[channelNb].values, ANALOG_WINDOW_SIZE);

  return true;
}

/* END analog */
/*!
 ** @}
 */
