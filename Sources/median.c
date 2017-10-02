/*! @file
 *
 *  @brief sorts array and finds the median.
 *
 *  @author Declan Scott         11970744
 *  @author Christine Vinaviles  11986282
 *  @date 2017-09-26
 */

// include header files with functions used for Lab4
#include <stdlib.h> /*contains quick sort library*/
 #include "types.h"
 #include "median.h"
 #include "Cpu.h"
 #include "MK70F12.h"

/*! @brief Compares Values.
 *
 *  @param const void * a pointer to where int a is stored used. First element for comparing in the array
 *  @param const void * a pointer to where int b is stored. Second element for comparing values in the array
 *  @return int the result of the equation int a - int b
 */
int CompareValues(const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}

/*! @brief Uses Qsort function available in c library.
 *
 *  @param const array that will be sorted
 *  @param const size of the array
 *  @return int16_t the resulting median found from the qsort function
 */
int16_t SortAndFindMedian(const int16_t array[], const uint32_t size)
{
  // temporary array to pass the values of constant array to. Max size of 1024
  int16_t tempArray[1024];
  // variable to contain median
  int median;

  // loop through the constant array and pass the values of that array into a non-constant one
  for (int i = 0; i < size ; i ++)
  {
    tempArray[i] = array[i];
  }

  // use qsort function to sort the array in ascending order
  qsort(tempArray, size, sizeof(tempArray[0]), CompareValues);

  // if the array size is even 
  if (size % 2 == 0 )
  {
    // say that the median is the avarage of the two middle values
    median = (tempArray[size/2] + tempArray[(size/2)+1]) /2;
  }
  else
  {
    // else the median is the variable in the very middle of the array
    median = tempArray[(size+1)/2];
  }
  return median;
}

 int16_t Median_Filter(const int16_t array[], const uint32_t size)
 {
   // sort array of a length up to 1024
   // have to sort array into ascending order
   // median is the middle number if numbers are even
   // median is the average of the two middle sorted numbers

   // if the size of the passed array is 0, return false. 
   if (size == 0)
     return false;

   return SortAndFindMedian(array, size); // resulting median is returned
 }

/* END median */
/*!
 ** @}
 */
