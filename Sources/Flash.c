/*
 * Flash.c
 *
 *      @date: 15 Aug 2017
 *      @author: 11970744, 11986282
 */

#include "types.h"
// add in header file for Flash.c
#include "Flash.h"
// peripheral memory map for tower
#include "MK70F12.h"

// Size of the memory available
#define MEMORY_SIZE ((FLASH_DATA_END - FLASH_DATA_START) + 1)
#define FLASH_CMD_ERASE_SECTOR FTFE_FCCOB0_CCOBn(0x09)
#define FLASH_CMD_PROGRAM_PHRASE FTFE_FCCOB0_CCOBn(0x07)

static bool addressAvailable[MEMORY_SIZE];

/*!
 * @brief This launches the Command
 *
 * @return bool - TRUE if the command launches successfully
 */
static bool LaunchCommand(TFCCOB* commonCommandObject)
{
  /* returns false if FCCOB is not available, LaunchCommand fails */
  if(!(FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK))
  {
    return false;
  }

  if((FTFE_FSTAT & FTFE_FSTAT_ACCERR_MASK) || (FTFE_FSTAT & FTFE_FSTAT_FPVIOL_MASK))
  {
    /* Clear the error flags if they happen to have been set previously */
    FTFE_FSTAT |= (FTFE_FSTAT_ACCERR_MASK | FTFE_FSTAT_FPVIOL_MASK);
  }

    FTFE_FCCOB0 = commonCommandObject->FCCOB0;
    FTFE_FCCOB1 = commonCommandObject->FCCOB1;
    FTFE_FCCOB2 = commonCommandObject->FCCOB2;
    FTFE_FCCOB3 = commonCommandObject->FCCOB3;
    FTFE_FCCOB7 = commonCommandObject->FCCOB7;
    FTFE_FCCOB6 = commonCommandObject->FCCOB6;
    FTFE_FCCOB5 = commonCommandObject->FCCOB5;
    FTFE_FCCOB4 = commonCommandObject->FCCOB4;
    FTFE_FCCOBB = commonCommandObject->FCCOBB;
    FTFE_FCCOBA = commonCommandObject->FCCOBA;
    FTFE_FCCOB9 = commonCommandObject->FCCOB9;
    FTFE_FCCOB8 = commonCommandObject->FCCOB8;

    FTFE_FSTAT |= FTFE_FSTAT_CCIF_MASK;  // this be the register for the flash status?? allegedly...
    // le make sure we shoot it into the right spots
  while(!(FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK))
  {
  }

  return true;
}

/*! @brief Enables the Flash module.
 *
 *  @return bool - TRUE if the Flash was setup successfully.
 */
bool Flash_Init(void)
{
  uint8_t checkAvailability;
  for (checkAvailability = 0; checkAvailability < MEMORY_SIZE ; checkAvailability++) // set availability of each address
  {
    addressAvailable[checkAvailability] = true; // set to true - for use when Flash_AllocateVar()
  }
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
bool Flash_AllocateVar(volatile void** variable, const uint8_t size)
{
  uint32_t position; // 32 bit position

  if (size == 1) // if size is 1 byte, we can allocate it to any address
  {
    for (position = 0 ; position < MEMORY_SIZE ; position++)
    {
      if (addressAvailable[position])
      {
        *variable = (void *)(FLASH_DATA_START + position);
        addressAvailable[position] = false;
        return true;
      }
    }
  }
  else if (size == 2)
  {
    for (position = 0 ; position < MEMORY_SIZE ; position += 2 )
    {
      if (addressAvailable[position] && addressAvailable[position + 1])
      {
        *variable = (void*)FLASH_DATA_START + position;
        addressAvailable[position] = false;
        addressAvailable[position + 1] = false;
        return true;
      }
    }
  }
  else if (size == 4)
  {
    for(position = 0; position < MEMORY_SIZE ; position += 4)
    {
      if (addressAvailable[position] && addressAvailable[position + 1] && addressAvailable[position + 2] && addressAvailable[position + 3])
      {
        *variable = (void*)FLASH_DATA_START + position;
        addressAvailable[position] = false;
        addressAvailable[position + 1] = false;
        addressAvailable[position + 2] = false;
        addressAvailable[position + 3] = false;
        return true;
      }
    }
  }
  return false; // size doesn't match 1, 2 or 4 OR no memory is left to allocate OR allocation failed for god knows what reason
}

/*!
 * @brief Writes phrase to the flash sector
 *
 * @param address phrase address
 * @param phrase what is going to be written in that address
 *
 * @return boool - TRUE if phrase was written successfully
 */
static bool WritePhrase(const uint32_t address, const uint64union_t phrase)
{
  TFCCOB FlashFCCOB;

  FlashFCCOB.FCCOB0 = FLASH_CMD_PROGRAM_PHRASE;
  FlashFCCOB.FCCOB1 = (address >> 16);
  FlashFCCOB.FCCOB2 = (address >> 8);
  FlashFCCOB.FCCOB3 = address;
  FlashFCCOB.FCCOB7 = (phrase.s.Lo);
  FlashFCCOB.FCCOB6 = (address >> 8);
  FlashFCCOB.FCCOB5 = (address >> 16);
  FlashFCCOB.FCCOB4 = (address >> 24);
  FlashFCCOB.FCCOBB = (phrase.s.Hi);
  FlashFCCOB.FCCOBA = (address >> 8);
  FlashFCCOB.FCCOB9 = (address >> 16);
  FlashFCCOB.FCCOB8 = (address >> 24);

  if (!LaunchCommand(&FlashFCCOB))
  {
    return false;
  }
  return !((FTFE_FSTAT & FTFE_FSTAT_MGSTAT0_MASK) || (FTFE_FSTAT & FTFE_FSTAT_ACCERR_MASK) || (FTFE_FSTAT & FTFE_FSTAT_FPVIOL_MASK));
}

/*!
 * @brief Erases the flash sector
 * @param address the address of the sector
 *
 * @return bool - true if the phrase was erased successfully
 */
static bool EraseSector(const uint32_t address)
{
  bool successfulErase = false;

  TFCCOB FlashFCCOB;

  FlashFCCOB.FCCOB0 = FLASH_CMD_ERASE_SECTOR;
  FlashFCCOB.FCCOB1 = (uint8_t)(address >> 16);
  FlashFCCOB.FCCOB2 = (uint8_t)(address >> 8);
  FlashFCCOB.FCCOB3 = (uint8_t)(address);

  if (!LaunchCommand(&FlashFCCOB))
  {
    return false;
  }
  return !((FTFE_FSTAT & FTFE_FSTAT_MGSTAT0_MASK) || (FTFE_FSTAT & FTFE_FSTAT_ACCERR_MASK) || (FTFE_FSTAT & FTFE_FSTAT_FPVIOL_MASK));
}

/*!
 * @brief This erases then writes to the address
 * @param address
 * @param phrase
 * @return bool - if the phase is successfully modified
 */
bool ModifyPhrase(const uint32_t address, const uint64union_t phrase)
{
  return EraseSector(address) && WritePhrase(address, phrase);
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
  volatile uint32_t* phraseAdd = address;
  uint64union_t phrase;

  if(!((uint32_t)address >= FLASH_DATA_START && (uint32_t)address <= FLASH_DATA_END && (uint32_t)address % 4 ==0))
  {
    return false;
  }

  if((uint32_t)address % 8 == 0 )
  {
    phrase.s.Lo = data;
    phrase.s.Hi = *(address + 1);
  }
  else
  {
    phrase.s.Hi = data;
    phrase.s.Lo = *(address -1 );

    phraseAdd--;
  }

  return ModifyPhrase((uint32_t)phraseAdd, phrase);
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
  volatile uint16_t* wordAdd = address;
  uint32union_t word;

  if(!((uint32_t)address >= FLASH_DATA_START && (uint32_t)address <= FLASH_DATA_END && (uint32_t)address %2 == 0))
  {
    return false;
  }

  if((uint32_t)address % 4 == 0)
  {
    word.s.Lo = data;
    word.s.Hi = *(address + 1);
  }
  else
  {
    word.s.Hi = data;
    word.s.Lo = *(address - 1);

    wordAdd--;
  }

  return Flash_Write32((uint32_t*)wordAdd, word.l);
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
  volatile uint8_t* halfWordAdd = address;
  uint16union_t halfWord;

  if(!((uint32_t)address >= FLASH_DATA_START && (uint32_t)address <= FLASH_DATA_END))
  {
    return false;
  }

  if ((uint32_t)address % 2 == 0)
  {
    halfWord.s.Lo = data;
    halfWord.s.Hi = *(address + 1);
  }
  else
  {
    halfWord.s.Hi = data;
    halfWord.s.Lo = *(address - 1);

    halfWordAdd--;
  }

  return Flash_Write16((uint16_t*)halfWordAdd, halfWord.l);
}

/*! @brief Erases the entire Flash sector.
 *
 *  @return bool - TRUE if the Flash "data" sector was erased successfully.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Erase(void)
{
  return EraseSector(FLASH_DATA_START);
}

/* END Flash */
/*!
** @}
*/
