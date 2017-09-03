/*
 * PIT.c
 *
 *  Created on: 3 Sep 2017
 *  @date: 3 Sep 2017
 *  @author: 11970744, 11986282
 */
/*!
**  @addtogroup PIT_module PIT module documentation
**  @{
*/
/* MODULE PIT */

// peripheral memory map for tower
#include "MK70F12.h"
// include PIT header files
#include "PIT.h"

static uint32_t ModuleClockPeriod;

bool PIT_Init(const uint32_t moduleClk, void (*userFunction)(void*), void* userArguments)
{
}

void PIT_Set(const uint32_t period, const bool restart)
{
  // to create a timer with a period of 1000 “ticks” of the module clock, the LDVAL register needs to be loaded with 999.
  uint32_t cycles = (period/ModuleClockPeriod) - 1;
}

void PIT_Enable(const bool enable)
{
}

void __attribute__ ((interrupt)) PIT_ISR(void)
{
}

/* END PIT */
/*!
** @}
*/