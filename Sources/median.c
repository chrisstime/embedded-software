/*
 * median.c
 *
 *    @date 11 Sep 2017
 *    @author 11986282
 *    @author 11970744
 */

// include header files with functions used for Lab4
 #include "types.h"
 #include "median.h"
 #include "Cpu.h"
 #include "MK70F12.h"

 /*! @brief isEven checks if the size of the array is even
  *
  *  @param sizeofArray
  *  @bool returns true if the size of the array is even
  */
 static bool isEven(uint32_t sizeOfArray)
 {
    if (sizeOfArray % 2 == 0)
      return true;
    return false;
 }

 /*! @brief quickSort is a function that sorts the array in ascending order
  *
  *  @param array[] is the array to be sorted
  *  @bool returns true if the after the array has been successfully sorted
  */
 static bool quickSort(const int16_t array[])
 {
   
 }

 int16_t Median_Filter(const int16_t array[], const uint32_t size)
 {
   // sort array of a length up to 1024
   // have to sort array into ascending order
   // median is the middle number if numbers are even
   // median is the average of the two middle sorted numbers

 }

/* END median */
/*!
 ** @}
 */
