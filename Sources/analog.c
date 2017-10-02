/*! @file
 *
 *  @brief Routines for setting up and reading from the ADC.
 *
 *  This contains the functions needed for accessing the external TWR-ADCDAC-LTC board.
 *
 *  @author Declan Scott        11970744
 *          Christine Vinaviles 11986282
 *  @date 2017-09-26
 */

// include header files with functions used for Lab4
#include "types.h"
#include "analog.h"
#include "Cpu.h"
#include "MK70F12.h"
#include "SPI.h"
#include "median.h"

bool Analog_Init(const uint32_t moduleClock)
{
  // if module clock is empty for some reason immediately return false for Analog_Init
  if (moduleClock == 0)
  {
    return false;
  }

  // set SPI Module to whatever was stated in the lab.
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

  /* Loop througj to initialize Analog_Input */
  for (int i = 0; i < 2 ; i++)
  {
    Analog_Input[i].value.l = 0;
    Analog_Input[i].oldValue.l = 0;
    Analog_Input[i].putPtr = Analog_Input[i].values;
  }

  return true;
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

  /* Have SPI transmit the inputData */
  SPI_Exchange(inputData, Analog_Input[channelNb].putPtr);

  /* Point to the location of the next sample to take */
  if (Analog_Input[channelNb].putPtr == &(Analog_Input[channelNb].values[ANALOG_WINDOW_SIZE - 1]))
  {
    Analog_Input[channelNb].putPtr = Analog_Input[channelNb].values;
  }
  else
  {
    (Analog_Input[channelNb].putPtr)++;
  }

  return true;
}

/* END analog */
/*!
 ** @}
 */
