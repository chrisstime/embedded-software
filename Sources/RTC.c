/*
 * RTC.c
 *
 *  Created on: 3 Sep 2017
 *  @date: 3 Sep 2017
 *  @author: 11970744, 11986282
 */
/*!
**  @addtogroup RTC_module RTC module documentation
**  @{
*/
/* MODULE RTC */

// peripheral memory map for tower
#include "MK70F12.h"
// includes RTC.h header files
#include "RTC.h"
// include CPU function header files
#include "Cpu.h"
#include "PE_Types.h"

// initiate instances of UserFunction and UserArguments
static void (*UserFunction)(void*);
static void* UserArguments;

bool RTC_Init(void (*userFunction)(void*), void* userArguments)
{
  // if userFunction being passed through doesn't exist stop function and return false
  if (!userFunction)
  {
    return false;
  }

  // enable System Clock Gating Control Register bit for RTC
  SIM_SCGC6 |= SIM_SCGC6_RTC_MASK;

  // if the Real Time Clock Register Oscillator is not enabled, enable it
  if(!(RTC_CR & RTC_CR_OSCE_MASK))
  {
    /* Enable 32.768 kHz oscillator */
    RTC_CR |= RTC_CR_SC2P_MASK;
    RTC_CR &= ~RTC_CR_SC4P_MASK;
    RTC_CR &= ~RTC_CR_SC8P_MASK;
    RTC_CR |= RTC_CR_SC16P_MASK;
    RTC_CR |= RTC_CR_OSCE_MASK;

    /* Count to 100ms to Wait for the oscillator to startup */
    uint8_t count;
    for (count = 0; count < 100; count++)
    {
    }

    // Enable the RTC Time Seconds Register
    RTC_TSR |= RTC_TSR_TSR_MASK;

    // Clear le bits in RTC Control Register
    RTC_CR &= ~RTC_CR_SWR_MASK;
    RTC_CR &= ~RTC_CR_WPE_MASK;
    RTC_CR &= ~RTC_CR_SUP_MASK;
    RTC_CR &= ~RTC_CR_UM_MASK;
    RTC_CR &= ~RTC_CR_CLKO_MASK;

    // Enable the RTC Lock Register
    RTC_LR &= ~RTC_LR_CRL_MASK;

    //Clear Interrupt Register bits. Just chucked in all the masks for the IER bit fields m8
    RTC_IER &= ~RTC_IER_TIIE_MASK;
    RTC_IER &= ~RTC_IER_TOIE_MASK;
    RTC_IER &= ~RTC_IER_TAIE_MASK;
    RTC_IER &= ~RTC_IER_MOIE_MASK;

    UserFunction = userFunction;
    UserArguments = userArguments;
  }

}

void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
  // apparently we have to do this so all Exception handler changes doesn't screw us over
  EnterCritical();

  // sets the time
  uint32_t time = (hours*3600) + (minutes*60) + seconds;

  // Disable the status register first before writing to the RTC Time Seconds Register. Disables but does not increment
  RTC_SR &= ~RTC_SR_TCE_MASK;

  // Enable the RTC Time Seconds Register
  RTC_TSR = time;

  // Enable the status register so that it can increment
  RTC_SR |= RTC_SR_TCE_MASK;

  ExitCritical();
}

void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{
  // apparently we have to do this so all Exception handler changes doesn't screw us over
  EnterCritical();
  // variable to hold RTC_TSR
  uint32_t time = RTC_TSR;

  // convert the passed RTC_TSR time (because it's all in seconds)
  uint8_t passHours = time/3600;
  uint8_t passMinutes = (time/60) % 60;
  uint8_t passSeconds = time % 60;

  // pop in the values to the address
  *hours = passHours;
  *minutes = passMinutes;
  *seconds = passSeconds;

  ExitCritical();
}

void __attribute__ ((interrupt)) RTC_ISR(void)
{
  // call back function
  if (UserFunction)
    (*UserFunction)(UserArguments);
}


/* END RTC */
/*!
** @}
*/
