#include "analog.h"

TSPIModule aSPIModule;

bool Analog_Init(const uint32_t moduleClock)
{

  Analog_Input.putPtr = (Analog_Input.values[0]);

  //create ADC aSPIModule to send and initiate the SPI.
  //Other values will be adjusted as necessary when passed to Analog get.
  aSPIModule.isMaster = true;
  aSPIModule.continuousClock = false;
  aSPIModule.inactiveHighClock = false;
  aSPIModule.changedOnLeadingClockEdge = false;
  aSPIModule.LSBFirst = false;
  aSPIModule.baudRate = 1000000;

  //Initialise SPI with given ADC requests.
  SPI_Init(&aSPIModule, moduleClock);

  return true;
}

bool Analog_Get(const uint8_t channelNb)
{
  //form data
  ADCData data = 0x84;

  SPI2_PUSHR = data;

  //update the analog struct
  Analog_Input[channelNb].oldValue = Analog_Input[channelNb].value;
  //reset pointer value
  Analog_Input[channelNb].putPtr = (Analog_Input[channelNb].values[0]);
  for(Analog_Input[channelNb].putPtr == 0; Analog_Input[channelNb].putPtr < ANALOG_WINDOW_SIZE; Analog_Input[channelNb].putPtr ++)
  {
    SPI_Exchange(data, &(Analog_Input[channelNb].putPtr));
  }

  //update new value
  Median_Filter(Analog_Input[channelNb], ANALOG_WINDOW_SIZE);

  return true;
}
