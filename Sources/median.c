
#include "median.h"

#define SIZE_MAX 1024

int16_t Median_Filter(const int16_t array[], const uint32_t size)
{
  //make sure size is within constraint.
  if (size > SIZE_MAX)
  {
    return false;
  }

  //sort the array with insertion shift once method.
  int i, j, k;
   int16_t temp;
   //for the unsorted character
   for(i = 1; i < size; i++)
   {
     //if this is greater than the one below, copy out to sort.
     if(array[i] < array[i-1])
     {
       temp = array[1];
       //find the point in the array where it is greater than a sorted int.
       for(j = 2; j <= i; j++)
       {
         if(temp > array[i-j])
         {
           //shift all bits to the right 1 until array[i]
           for(k = i-j+1; k = i; k--)
           {
             array[k] = array[k-1];
           }
         }
         //find next half-word to sort
         break;
       }
     }
   }

  //find the median
  int16_t median1, median2, median;
  //find if array is even
  if (size % 2)
  {
    //find the median value
    median1=array[size/2];
    median2=array[(size-2)/2];
    median = (median1 + median2) / 2;
  }
  else
  {
    median = array[(size-1)/2];
  }
  Analog_Input.value = median;
  return Analog_Input.value;
}
