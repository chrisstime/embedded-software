#include "SPI.h"


bool SPI_Init(const TSPIModule* const aSPIModule, const uint32_t moduleClock)
{
  //set up clock gates for SPI2
  SIM_SCGC3 |= SIM_SCGC3_DSPI2_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
  //Set the ports MUX PTD11-15

  //SPI2 MCR
  SPI2_MCR |= aSPIModule->isMaster << SPI_MCR_MSTR_SHIFT;
  SPI2_MCR |= aSPIModule->continuousClock << SPI_MCR_CONT_SCKE_SHIFT;
  SPI2_MCR |= SPI_MCR_FRZ_MASK;
  SPI2_MCR |= SPI_MCR_PCSIS(aSPIModule->inactiveHighClock);
  SPI2_MCR |= SPI_MCR_DIS_RXF_MASK;
  SPI2_MCR |= SPI_MCR_DIS_TXF_MASK;

  //SPI2 CTAR0
  SPI2_CTAR0 |= SPI_CTAR_FMSZ(15);
  SPI2_CTAR0 |= aSPIModule->LSBFirst << SPI_CTAR_LSBFE_SHIFT;
  SPI2_CTAR0 |= aSPIModule->changedOnLeadingClockEdge << SPI_CTAR_CPHA_SHIFT;
//  baud rate software calc. clock/SCK = 1/PBR * 1/BR
//  uint32_t divisor = moduleClock/aSPIModule->baudRate;
//  int i, j;
//  for(i = 1; i < 8; i++)
//  {
//    uint32_t br_mem[16], diff_mem[16], pbr_mem[16];
//    uint32_t *br_ptr[16] = br_mem[16], *diff_ptr[16] = diff_mem[16], pbr_ptr[8] = pbr_mem[8];
//    //if the number is odd
//    if ((i-1)%2 == 0)
//    {
//      pbr_ptr[i] = i;
//      for (j = 0; j < 16; j++)
//      {
//        //find the best value for PBR = i
//        if (j<8)
//        {
//          if(j == 0) br = 2;
//          if(j == 1) br = 4;
//          if(j == 2) br = 6;
//        }
//        else
//        {
//          br = 1 << j;
//        }
//        br_ptr[j] = j;
//        pbr_br = pbr * br;
//
//        //save to find the closest value and values
//        diff_mem[j] = divisor - pbr_br;
//
//
//      }
//    }
//  }
//
//  return true;

  //pbr=3
  //br=8

  //delays

  //PCS-SCK 5e-6
  SPI2_CTAR0 |= SPI_CTAR_CSSCK(6);
  SPI2_CTAR0 &= ~SPI_CTAR_PCSSCK_MASK;

  //BAUDRATE
  SPI2_CTAR0 |= SPI_CTAR_PBR(1);
  SPI2_CTAR0 |= SPI_CTAR_BR(3);

  return true;

}

void SPI_SelectSlaveDevice(const uint8_t slaveAddress)
{
  //use the GPIO to select the Slave device
  //GPIO 7 = 8 = 9 = high for ADC
  //GPIO 7 = 8 = high. GPIO 9 = low for DAC
  //gpio 7 = PTE27
  //gpio8 = pte5
  //gpio9 = pte18 (always high

  switch (slaveAddress)
  {
    case 0x07: //ADC
      GPIOE_PSOR = 0x3 << 8;

    case 0x04: //DAC
      GPIOE_PCOR = 0x03 << 8;
  }

  return;
}

void SPI_Exchange(const uint16_t dataTx, uint16_t* const dataRx)
{
  if (SPI2_SR & SPI_SR_TFFF_MASK)
  {
    //set PUSHR commands
    SPI2_PUSHR = 0x01 << 16;
    SPI2_PUSHR |= dataTx;
    SPI2_SR |= SPI_SR_TXRXS_MASK; //enable tx and rx

    //wait for transfers in the tx and rx have occured.
    while(!(SPI2_SR & SPI_SR_TCF_MASK));
      SPI2_SR != SPI_SR_TCF_MASK;
    //wait for the counter to be 1
    while (!(SPI2_SR & SPI_SR_RXCTR_MASK));
      &dataRx = SPI2_POPR;
      SPI2_SR != SPI_SR_RXCTR_MASK;
      SPI2_SR != SPI_SR_TFFF_MASK;
  }


  return true;
}
