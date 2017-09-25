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

 /*! @brief merge checks if the size of the array is even
  *
  *  @param leftIndex is used for indexing purposes, starting from 0
  *  @param rightIndex is used for indexing, starting from the end of the array
  *  @bool returns true if the merge was successful
  */
 static bool Merge(const uint16_t array[], int leftIndex, int rightIndex, int middleIndex)
 {
   int counter1, counter2, counter3;
    int arrayIndexLeft = middleIndex - leftIndex + 1;
    int arrayIndexRight =  rightIndex - middleIndex;

    /* create temp arrays */
    uint16_t leftArray[arrayIndexLeft], rightArray[arrayIndexRight];

    /* Copy data to temp arrays leftArray[] and rightArray[] */
    for (counter1 = 0; counter1 < arrayIndexLeft; counter1++)
        leftArray[counter1] = array[leftIndex + counter1];
    for (counter2 = 0; counter2 < arrayIndexRight; counter2++)
        rightArray[counter2] = array[middleIndex + 1+ counter2];

    /* Merge the temp arrays back into array[leftIndex..rightIndex]*/
    counter1 = 0; // Initial index of first subarray
    counter2 = 0; // Initial index of second subarray
    counter3 = leftIndex; // Initial index of merged subarray
    while (counter1 < arrayIndexLeft && counter2 < arrayIndexRight)
    {
        if (leftArray[counter1] <= rightArray[counter2])
        {
            array[counter3] = leftArray[counter1];
            counter1++;
        }
        else
        {
            array[counter3] = rightArray[counter2];
            counter2++;
        }
        counter3++;
    }

    /* Copy the remaining elements of leftArray[], if there
       are any */
    while (counter1 < arrayIndexLeft)
    {
        array[counter3] = leftArray[counter1];
        counter1++;
        counter3++;
    }

    /* Copy the remaining elements of rightArray[], if there
       are any */
    while (counter2 < arrayIndexRight)
    {
        array[counter3] = rightArray[counter2];
        counter2++;
        counter3++;
    }
 }

 /*! @brief quickSort is a function that sorts the array in ascending order
  *  This is a recursive function
  *
  *  @param array[] is the array to be sorted
  *  @param int rightIndex starts from the end of the array
  *  @param int leftIndex starts from 0 and increments as we go through the array
  *  @bool returns true if the after the array has been successfully sorted
  */
 static bool MergeSort(const int16_t array[], int leftIndex, int rightIndex)
 {
    // we will be using the mergeSort method https://brilliant.org/wiki/merge/
    if (leftIndex < rightIndex)
    {
        // Same as (leftIndex+rightIndex)/2, but avoids overflow for
        // large leftIndex and h
        int middleIndex = leftIndex+(rightIndex-leftIndex)/2;

        // Sort first and second halves
        mergeSort(array, leftIndex, middleIndex);
        mergeSort(array, rightIndex, middleIndex+1);

        merge(array, leftIndex, rightIndex, middleIndex);
    }
   return true;
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

   if (MergeSort(array[], 0, size))
     return true; // return true if successful

   return false;
 }

/* END median */
/*!
 ** @}
 */
