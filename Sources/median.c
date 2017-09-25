/*
 * median.c
 *
 *    @date 11 Sep 2017
 *    @author 11986282
 *    @author 11970744
 */

// include header files with functions used for Lab4
#include <stdlib.h> /*contains quick sort library*/
 #include "types.h"
 #include "median.h"
 #include "Cpu.h"
 #include "MK70F12.h"

/*! @brief Sets up the SPI before first use.
 *
 *  @param aSPIModule is a structure containing the operating conditions for the module.
 *  @param moduleClk The module clock in Hz.
 *  @return BOOL - true if the SPI module was successfully initialized.
 */
int CompareValues(const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}

 int16_t Median_Filter(const int16_t array[], const uint32_t size)
 {
   // sort array of a length up to 1024
   // have to sort array into ascending order
   // median is the middle number if numbers are even
   // median is the average of the two middle sorted numbers
   // this function is recursive.
   if (size == 0)
     return false;

   qsort(array, size, sizeOf(int) CompareValues);

   return false;
 }

/* END median */
/*!
 ** @}
 */
