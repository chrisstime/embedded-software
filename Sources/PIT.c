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
// include CPU function header files
#include "Cpu.h"
#include "PE_Types.h"

static uint32_t ModuleClockPeriod;

// initiate instances of UserFunction and UserArguments
static void (*UserFunction)(void*);
static void* UserArguments;

bool PIT_Init(const uint32_t moduleClk, void (*userFunction)(void*), void* userArguments)
{
  // if userFunction being passed through doesn't exist stop function and return false
  if (!userFunction)
  {
    return false;
  }

  // enable System Clock Gating Control Register bit for PIT
  SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;

  // Disabled PIT clock
  PIT_MCR |= PIT_MCR_MDIS_MASK;
  // and enable freezing timers during debug
  PIT_MCR |= PIT_MCR_FRZ_MASK;

  // clear the interrupt flag for PIT 0
  PIT_TFLG0 &= ~PIT_TFLG_TIF_MASK;

  // enable the Timer interrupt for PIT 0
  PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;

  // after clearing the flag, we want re-enable the PIT Clock
  PIT_MCR &= ~PIT_MCR_MDIS_MASK;

  // calculate the module's clock period
  ModuleClockPeriod = (1000000000/moduleClk);
  // pass User function to use for call back
  UserFunction = userFunction;
  UserArguments = userArguments;

  return true;
}

void PIT_Set(const uint32_t period, const bool restart)
{
  // to create a timer with a period of 1000 ticks of the module clock, the LDVAL register needs to be loaded with 999.
  uint32_t cycles = (period/ModuleClockPeriod) - 1;

  PIT_LDVAL0 = PIT_LDVAL_TSV(cycles);

  if (restart)
  {
     // disable then renable the clock ... because that's what a restart generally means kek
      PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK;
      PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;
  }
}

void PIT_Enable(const bool enable)
{
  if (enable)
  { // enable the clock... because that's what enables generally do
    PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;
  }
  else // if they don se so den don do et
    PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK;
}

void __attribute__ ((interrupt)) PIT_ISR(void)
{
  // clear the interrupt flag
  PIT_TFLG0 = PIT_TFLG_TIF_MASK;

  // call back function
  if (UserFunction)
    (*UserFunction)(UserArguments);
}

/* END PIT */
/*!
** @}
*/
