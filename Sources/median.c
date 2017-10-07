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
 *  @param const void * a pointer to where int a is stored used. First element for comparing in the array
 *  @param const void * a pointer to where int b is stored. Second element for comparing values in the array
 *  @return int the result of the equation int a - int b
 */
int CompareValues(const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}

 int16_t Median_Filter(const int16_t array[], const uint32_t size)
 {
   int16_t tempArray[size];
   for (int i = 0; i < size ; i++)
   {
     tempArray[i] = array[i];
   }
   // sort array of a length up to 1024
   // have to sort array into ascending order
   // median is the middle number if numbers are even
   // median is the average of the two middle sorted numbers
   qsort(tempArray, size, sizeof(array[0]), CompareValues);

   int16_t median = 0;

   if (size % 2 == 0 )
   {
     median = (tempArray[size/2] + tempArray[(size/2)-1]) /2;
   }
   else
   {
     median = tempArray[(size+1)/2];
   }

   return median;
 }

/* END median */
/*!
 ** @}
 */
