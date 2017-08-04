/*
 * UART.c
 *
 *  Created on: 1 Aug 2017
 *      Author: 11970744
 */
/*!
**  @addtogroup UART_module packet module documentation
**  @{
*/
/* MODULE UART */

#include "UART.h"
#include "Cpu.h"
#include "MK70F12.h"



static FIFO TX_FIFO, RX_FIFO

bool UART_Init(const uint32_t baudRate, const uint32_t moduleClk) // 38400, CPU_BUS_CLK_HZ
{
  //Initialise PORTE
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;		//enable PORT_E
  SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;		//enable UART2 clock
  PORTE_PCR16 |= PORT_PCR_MUX(3);		//enable ALT3 for MUX to enable UART2_TX out of PORTE
  PORTE_PCR17 |= PORT_PCR_MUX(3);		//enable ALT3 for MUX to enable UART2_RX out of PORTE
  UART_C2 |= UART_C2_RE_MASK;
  UART_C2 |= UART_C2_TE_MASK;

  //UART2_C1
  return true
}

bool UART_InChar(uint8_t * const dataPtr)
{
  return FIFO_Get(RX_FIFO, *dataPtr);
}

bool UART_OutChar(const uint8_t data)
{
  return FIFO_Put(RX_FIFO, *dataPtr);	//shouldnt this be TX?
}

void UART_Poll(void){
  if (UART_S1 & UART_S1_TDRE_MASK)	//there's something to be received from the PC, put in the RX_FIFO
    {
      //run FIFO_get or UART_InChar
    }
  if (UART_S1 & UART_S1_RDRF_MASK)	// put something from PC to TX_FIFO to transmitto FIFO
    {
      //run FIFO_put or UART_OutChar
    }
					   //UART_S1 UART_S1_REG(UART2);?? I dont know what this is

}

/* END UART */
/*!
** @}
*/
