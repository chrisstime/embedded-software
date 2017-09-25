/*
 * SPI.c
 *
 *      @date 11 Sep 2017
 *      @author  11986282
 *      @author 11970744
 */

// include header files with functions used for Lab4
 #include "types.h"
 #include "SPI.h"
 #include "Cpu.h"
 #include "MK70F12.h"

bool SPI_Init(const TSPIModule* const aSPIModule, const uint32_t moduleClock)
{
  /* Enable SPI2, PORTD and PORTE clock gate control */
  SIM_SCGC3 |= SIM_SCGC3_DSPI2_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

  /* Enable SPI2 signals */
  PORTD_PCR11 |= PORT_PCR_MUX(0x02);
  PORTD_PCR12 |= PORT_PCR_MUX(0x02);
  PORTD_PCR13 |= PORT_PCR_MUX(0x02);
  PORTD_PCR14 |= PORT_PCR_MUX(0x02);
  PORTD_PCR15 |= PORT_PCR_MUX(0x02);

  /* Set up PTE5 & PTE27 as outputs */
  PORTE_PCR5 |= PORT_PCR_MUX(0x01);
  PORTE_PCR27 |= PORT_PCR_MUX(0x01);

  /* Sets pins as outputs */
  GPIOE_PDDR |= (1<<5);
  GPIOE_PDDR |= (1<<27);    
}

void SPI_SelectSlaveDevice(const uint8_t slaveAddress)
{
  // something
}

void SPI_Exchange(const uint16_t dataTx, uint16_t* const dataRx)
{
  // something
}

/* END SPI */
/*!
 ** @}
 */
