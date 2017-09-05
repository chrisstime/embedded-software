/*
 * UART.c
 *
 *  Created on: 1 Aug 2017
 *  Last Modified 8 Aug 2017
 *      Author: 11970744, 11986282
 */
/*!
 **  @addtogroup UART_module packet module documentation
 **  @
 */
/* MODULE UART */

#include "UART.h"
#include "Cpu.h"
#include "MK70F12.h"
#include "FIFO.h"
#include "PE_Types.h"

static TFIFO TX_FIFO, RX_FIFO;

bool UART_Init(const uint32_t baudRate, const uint32_t moduleClk) // 38400, CPU_BUS_CLK_HZ
{
	//Initialise PORTE

	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;		//enable PORT_E
	SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;		//enable UART2 clock
	PORTE_PCR16 |= PORT_PCR_MUX(0x03);      //enable ALT3 for MUX to enable UART2_TX out of PORTE
	PORTE_PCR17 |= PORT_PCR_MUX(0x03);      //enable ALT3 for MUX to enable UART2_RX out of PORTE
	//Initialise control registers
	UART2_C1 = 0x00;
	UART2_C2 = 0x00;
	UART2_C2 |= UART_C2_RE_MASK;
	UART2_C2 |= UART_C2_TE_MASK;
	UART2_C2 |= UART_C2_RIE_MASK;			//enable to always receive packets
	UART2_C4 &= ~UART_C4_BRFA_MASK;
	UART2_C4 |= UART_C4_BRFA(((2 * moduleClk)/baudRate) % 32);
	//Initialise BaudRate
	uint16_t divisor = moduleClk / (16 * baudRate);
	UART2_BDH = (divisor & 0x1F00) >> 8;
	UART2_BDL = (divisor & 0x00FF);

	//Initialise NVIC

	NVICISER1 |= NVIC_ISER_SETENA(1 << 17);
	NVICICPR1 |= NVIC_ICPR_CLRPEND(1 << 17);

	FIFO_Init(&RX_FIFO);
	FIFO_Init(&TX_FIFO);

	return true;
}

bool UART_InChar(uint8_t * const dataPtr) {
	return FIFO_Get(&RX_FIFO, dataPtr);
}

bool UART_OutChar(const uint8_t data) {
	EnterCritical();

	bool success = false;
	if(FIFO_Put(&TX_FIFO, data)){
		UART2_C2 |= UART_C2_TIE_MASK;
		success = true;

	}
	ExitCritical();
	return success;
}

void UART_Poll(void)
{
	EnterCritical();

	if (UART2_C2 & UART_C2_TIE_MASK)
	{
		if (UART2_S1 & UART_S1_TDRE_MASK)//there's something to be received from the PC, put in the RX_FIFO
		{
		  if(!FIFO_Get(&TX_FIFO, &UART2_D))
		  {//disable TIE if fails
		    UART2_C2 &= ~UART_C2_TIE_MASK;

		  }
		}
}

	if (UART2_C2 & UART_C2_RIE_MASK)
	{
	  if (UART2_S1 & UART_S1_RDRF_MASK)// put something from PC to TX_FIFO to transmit to FIFO
	  {
	    FIFO_Put(&RX_FIFO, UART2_D);
	  }
	}
	ExitCritical();
}

void  __attribute__ ((interrupt)) UART_ISR(void)
{
	if (UART2_C2 & UART_C2_TIE_MASK)				//check
	{
		if (UART2_S1 & UART_S1_TDRE_MASK)			//there's something to be received from the PC, put in the RX_FIFO
		{
			if(!FIFO_Get(&TX_FIFO, &UART2_D))
			{
				//disable TIE if fails
				UART2_C2 &= ~UART_C2_TIE_MASK;
			}
		}
	}


	if (UART2_C2 & UART_C2_RIE_MASK)
	{
		if (UART2_S1 & UART_S1_RDRF_MASK)// put something from PC to TX_FIFO to transmit to FIFO
		{
			FIFO_Put(&RX_FIFO, UART2_D);
		}
	}
}



/* END UART */
/*!
 ** @}
 */
