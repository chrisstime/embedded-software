/*
 * FIFO.c
 *
 *  Created on: 1 Aug 2017
 *  Last Modified 8 Aug 2017
 *      Author: 11970744, 11986282
 */
/*!
 **  @addtogroup FIFO_module FIFO module documentation
 **  @{
 */
/* MODULE FIFO */

#include "FIFO.h"

/*! @brief Initialize the FIFO before first use.
 *
 *  @param FIFO A pointer to the FIFO that needs initializing.
 *  @return void
 */
void FIFO_Init(TFIFO * const FIFO) {
	FIFO->Start = 0;
	FIFO->End = 0;
	FIFO->NbBytes = 0;
}

bool FIFO_Put(TFIFO * const FIFO, const uint8_t data) {
//  if (FIFO->NbBytes < FIFO_SIZE)
//    {
//      FIFO->Buffer[FIFO->End] = data;
//      FIFO->End = (FIFO->End + 1) % FIFO_SIZE;
//      FIFO->NbBytes++;
//      return true;
//    }
//  else {
//	  FIFO->End = 0;
//	  return false;
//  }
	if (FIFO->NbBytes >= FIFO_SIZE)
		return false;

	FIFO->Buffer[FIFO->End] = data;

	FIFO->End++;
	FIFO->NbBytes++;

	if (FIFO->End >= FIFO_SIZE)
		FIFO->End = 0;

	return true;
}

bool FIFO_Get(TFIFO * const FIFO, uint8_t volatile * const dataPtr)
{
	if (FIFO->NbBytes == 0)
		return false;

	*dataPtr = FIFO->Buffer[FIFO->Start];

	FIFO->Start++;
	FIFO->NbBytes--;

	if (FIFO->Start >= FIFO_SIZE)
		FIFO->Start = 0;

	return true;
}

/* END FIFO */
/*!
 ** @}
 */
