/*
 * Flash.c
 *
 *  Created on: 15 Aug 2017
 *      Author: 11970744, 11986282
 */

#include "types.h"
#include "Flash.h"
#include "MK70F12.h"

//Block 2, sector 2 address is 0x0008_0000;



uint64union_t virtualRAM;
uint8_t ArrayIndex[7];

/*! @brief Enables the Flash module.
 *
 *  @return bool - TRUE if the Flash was setup successfully.
 */
bool Flash_Init(void){
  //Read FSTAT, ensure that the Flags are cleared (write 0xF0)
	//FTFE_FSTAT = 0xF0;
  return true;
}

/*! @brief Allocates space for a non-volatile variable in the Flash memory.
 *
 *  @param variable is the address of a pointer to a variable that is to be allocated space in Flash memory.
 *         The pointer will be allocated to a relevant address:
 *         If the variable is a byte, then any address.
 *         If the variable is a half-word, then an even address.
 *         If the variable is a word, then an address divisible by 4.
 *         This allows the resulting variable to be used with the relevant Flash_Write function which assumes a certain memory address.
 *         e.g. a 16-bit variable will be on an even address
 *  @param size The size, in bytes, of the variable that is to be allocated space in the Flash memory. Valid values are 1, 2 and 4.
 *  @return bool - TRUE if the variable was allocated space in the Flash memory.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_AllocateVar(volatile void** variable, const uint8_t size){
//to find if we are changing a packet, half word etc. do a case basis.
	uint8_t AvailableBytes = 0 ;
	uint8_t i;

  for (i=1;i<8;i++)
      {
     if (ArrayIndex[i] == 0) 		//if unallocated
       AvailableBytes++;
     else AvailableBytes = 0; 		//start from zero again

     if (AvailableBytes == size){
    	// long newsize = i - (size - 1); //this somehow always equals 0x0 so it's not writing anything. The math is probably an issue
    	*variable = (void*)FLASH_DATA_START + i;
      if ((int)*variable % (size) == 0x00) // if variable is either 1, 2 or 4
        return true;
    }
  }
  return false; // if the for loop fails.
}


bool ModifyPhrase(const uint32_t address, const uint64_t phrase)
{
	  //FCCOB from LSB: 32107654BA98
	uint64union_t phrasewrite;
	phrasewrite.l = phrase;

	while(!(FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK))
	{
		FTFE_FSTAT = 0x30;			//ensure old errors are clear
	  FTFE_FCCOB0 = 0x07;//PROGRAM_PHRASE; //NEEDS defining
	  FTFE_FCCOB1 = 0x08;//FlashStartByte.s.ByteofWord[3];//
	  FTFE_FCCOB2 = 0;//FlashStartByte.s.ByteofWord[2];
	  FTFE_FCCOB3 = 0;//FlashStartByte.s.ByteofWord[1];
	  FTFE_FCCOB4 = phrasewrite.BoP.ByteofPhrase[3];
	  FTFE_FCCOB5 = phrasewrite.BoP.ByteofPhrase[2];
	  FTFE_FCCOB6 = phrasewrite.BoP.ByteofPhrase[1];
	  FTFE_FCCOB7 = phrasewrite.BoP.ByteofPhrase[0];
	  FTFE_FCCOB8 = phrasewrite.BoP.ByteofPhrase[7];
	  FTFE_FCCOB9 = phrasewrite.BoP.ByteofPhrase[6];
	  FTFE_FCCOBA = phrasewrite.BoP.ByteofPhrase[5];
	  FTFE_FCCOBB = phrasewrite.BoP.ByteofPhrase[4];
	}
	FTFE_FSTAT = 0x80; 	//clear flag
	if (!(FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK))
		return true;
	return false;
}

uint8_t ReadByte(uint8_t offset)
{
  uint32_t read = FLASH_DATA_START + offset;
  return _FB(read);
}
uint64_t ReadPhrase()
{

// FTFE_FSTAT = FTFE_FSTAT_CCIF_MASK; 	//clear flag to start writing

//  FTFE_FCCOB0 = 0x03;//READ_PHRASE; //NEEDS defining
//  FTFE_FCCOB1 = 0x08;//FlashStartByte.s.ByteofWord[3];//
//  FTFE_FCCOB2 = 0;//FlashStartByte.s.ByteofWord[2];
//  FTFE_FCCOB3 = 0;//FlashStartByte.s.ByteofWord[1];
//
//  virtualRAM.l = FTFE_FCCOBB;
  return _FP(FLASH_DATA_START);
}

/*! @brief Writes a 32-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 32-bit data to write.
 *  @return bool - TRUE if Flash was written successfully, FALSE if address is not aligned to a 4-byte boundary or if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Write32(volatile uint32_t* const address, const uint32_t data)
{
  volatile uint32_t *newaddress;
  uint64union_t sixfourbit;

  if (*address == 0) //write into the low part of the union
    {
      sixfourbit.s.Lo = data;
      sixfourbit.s.Hi = virtualRAM.s.Hi;
      *newaddress = *address;
    }
  else
    {
      sixfourbit.s.Hi = data;
      *newaddress = *address - 4;	//change index as we need to realign the starting point
      sixfourbit.s.Lo = virtualRAM.s.Lo;
    }

  bool success;
  success = ModifyPhrase(FLASH_DATA_START, sixfourbit.l);
   if (success)
	   return true;
  return 0;
}

/*! @brief Writes a 16-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 16-bit data to write.
 *  @return bool - TRUE if Flash was written successfully, FALSE if address is not aligned to a 2-byte boundary or if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Write16(volatile uint16_t* const address, const uint16_t data)
{

  uint16_t *naddress;
  uint32union_t threetwobit;


  if (*address % 4 == 0) //write into the low part of the union
    {
      threetwobit.s.Lo = data;
      threetwobit.s.Hi = virtualRAM.HWoP.HWofPhrase [(int)*address + 2];
  //    *naddress = *address;
    }
  else
    {
      threetwobit.s.Hi = data;
      *naddress = *address - 2;	//change index as we need to realign the starting point
      threetwobit.s.Lo = virtualRAM.HWoP.HWofPhrase [(int)*address];
    }


  bool success;
  success = Flash_Write32((uint32_t*)address, threetwobit.l); //pass the half word to form a word.
   if (success){
     return true;}
    return 0;
}

/*! @brief Writes an 8-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 8-bit data to write.
 *  @return bool - TRUE if Flash was written successfully, FALSE if there is a programming error.
 *  @note Assumes Flash has been initialized.
 *
 *
 */
bool Flash_Write8(volatile uint8_t* const address, const uint8_t data)
{
  uint8_t *newaddress;
  uint16union_t onesixbit;

	ReadPhrase(); // copy flash phrase into RAM
	if (Flash_Erase()){

	  if ((int)address > 7)//FlashSize needs defining //return false if the offset is too large
		return false;
	  if (*address % 2 == 0)		//if index is even put flash in low part of union
		{
		  onesixbit.s.Lo = data;
		  onesixbit.s.Hi = virtualRAM.BoP.ByteofPhrase [(int)*address+1]; //get RAM unchanged part to add to union
		  *newaddress = *address;
		}
	  else
		{				 //reverse of true to align the bytes
		  onesixbit.s.Hi = data;
		  *newaddress = *address - 1;	//change index as we need to realign the starting point
		  onesixbit.s.Lo = virtualRAM.BoP.ByteofPhrase [(int)*newaddress];
		}

	   bool success;
	   success = Flash_Write16((uint16_t*)newaddress, onesixbit.l); //pass the half word to form a word.
		if (success){
		  return true;
		}
	}
	return 0;
}

/*! @brief Erases the entire Flash sector.
 *
 *  @return bool - TRUE if the Flash "data" sector was erased successfully.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Erase(void)
{

    FTFE_FCCOB0 = 0x08; //PROGRAM_PHRASE; //NEEDS defining
    FTFE_FCCOB1 = 0x08; //FlashStartByte.s.ByteofWord[3];//
    FTFE_FCCOB2 = 0; //FlashStartByte.s.ByteofWord[2];
    FTFE_FCCOB3 = 0; //FlashStartByte.s.ByteofWord[1];
    FTFE_FSTAT = FTFE_FSTAT_CCIF_MASK; 	//clear flag to start writing

  return true;
}
