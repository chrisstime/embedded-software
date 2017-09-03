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

bool RTC_Init(void (*userFunction)(void*), void* userArguments)
{
}

void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
}

void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{
}

void __attribute__ ((interrupt)) RTC_ISR(void)
{
}


/* END RTC */
/*!
** @}
*/