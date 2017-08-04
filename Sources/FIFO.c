/*
 * FIFO.c
 *
 *  Created on: 1 Aug 2017
 *      Author: 11970744
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
void FIFO_Init(TFIFO * const FIFO)
{
  FIFO->Start = 0;
  FIFO->End = 0;
  FIFO->NbBytes = 0;
}

bool FIFO_Put(TFIFO * const FIFO, const uint8_t data)
{
  if (FIFO->NbBytes < FIFO_SIZE)
    {
      FIFO->Buffer[FIFO->End] = data;
      FIFO->End = (FIFO->End + 1) % FIFO_SIZE;
      FIFO->NbBytes++;
      return true;
    }
  return false;
}


bool FIFO_Get(TFIFO * const FIFO, uint8_t * const dataPtr)
{
  if (FIFO->NbBytes > 0){
      *dataPtr = FIFO->Buffer[FIFO->Start];
      FIFO->Start++;
      FIFO->NbBytes--;
      return true;
  }
  return false;
}



/* END FIFO */
/*!
** @}
*/
