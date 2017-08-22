/*
 * LEDs.c
 *
 *  Created on: 20 Aug 2017
 *      Author: 11986282
 */
/*
/*!
 **  @addtogroup LEDs_module LEDs module documentation
 **  @{
 */
/* MODULE LEDs */

#include "LEDs.h"
#include "MK70F12.h"


bool LEDs_Init(void){
  // Enable PORT A
  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

  // MUX
  PORTA_PCR11 |= PORT_PCR_MUX(0x01);
  PORTA_PCR28 |= PORT_PCR_MUX(0x01);
  PORTA_PCR29 |= PORT_PCR_MUX(0x01);
  PORTA_PCR10 |= PORT_PCR_MUX(0x01);

  PORTA_PCR11 |= PORT_PCR_DSE_MASK;
  PORTA_PCR28 |= PORT_PCR_DSE_MASK;
  PORTA_PCR29 |= PORT_PCR_DSE_MASK;
  PORTA_PCR10 |= PORT_PCR_DSE_MASK;

  GPIOA_PDOR |= GPIO_PDOR_PDO_MASK;

  GPIOA_PDDR |= (LED_ORANGE | LED_YELLOW | LED_GREEN | LED_BLUE);

  return true;
}


void LEDs_On(const TLED color){
  GPIOA_PCOR |= color;
}


void LEDs_Off(const TLED color){
  GPIOA_PSOR = color;
}


void LEDs_Toggle(const TLED color){
  GPIOA_PTOR |= color;
}

/* END LEDs */
/*!
 ** @}
 */




