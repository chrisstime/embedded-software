/*
 * packet.c
 *
 *  Created on: 1 Aug 2017
 *  Last Modified 8 Aug 2017
 *      Author: 11970744, 11986282
 */
/*!
**  @addtogroup packet_module packet module documentation
**  @{
*/
/* MODULE packet */

#include "packet.h"
#include "UART.h"


//uint8_t	Packet_Command,	/*!< The packet's command */
//	Packet_Parameter1, 	/*!< The packet's 1st parameter */
//	Packet_Parameter2, 	/*!< The packet's 2nd parameter */
//	Packet_Parameter3,	/*!< The packet's 3rd parameter */
//	Packet_Checksum;	/*!< The packet's checksum */

TPacket Packet;

static uint8_t PacketPosition;
const uint8_t PACKET_ACK_MASK = 0x80;


/*! @brief Initializes the packets by calling the initialization routines of the supporting software modules.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz
 *  @return bool - TRUE if the packet module was successfully initialized.
 *  @uint8_t Calculated_Checksum - returns the Calculated checksum based on XORing all the packets together
 *  @bool Checksum_Check Compares the calculated checksum with the value of Checksum and returns true if the are the same
 */

uint8_t Calculated_Checksum(void)
{
	return Packet_Command ^ Packet_Parameter1 ^ Packet_Parameter2 ^ Packet_Parameter3;
}

static bool Checksum_Check(void)
{
  if (Calculated_Checksum() == Packet_Checksum)
    {
      return true;
    }
  return false;
}


bool Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  PacketPosition = 0;
  Packet_Parameter1 = 0;
  Packet_Parameter2 = 0;
  Packet_Parameter3 = 0;
  Packet_Checksum = 0;
  Packet_Command = 0;
  return UART_Init(baudRate, moduleClk);
}

void Packet_Shift(){
  Packet_Command = Packet_Parameter1;
  Packet_Parameter1 = Packet_Parameter2;
  Packet_Parameter2 = Packet_Parameter3;
  Packet_Parameter3 =  Packet_Checksum;
  Packet_Checksum = 0;
}

/*! @brief Attempts to get a packet from the received data.
 *
 *  @return bool - TRUE if a valid packet was received.
 */
bool Packet_Get(void){
	static int PacketPosition = 0;

	switch(PacketPosition){
	case 0:
		if (UART_InChar(&Packet_Command))
			PacketPosition++;
		break;

	case 1:
		if (UART_InChar(&Packet_Parameter1))
			PacketPosition++;
		break;

	case 2:
		if (UART_InChar(&Packet_Parameter2))
			PacketPosition++;
		break;

	case 3:
		if (UART_InChar(&Packet_Parameter3))
			PacketPosition++;
		break;

	case 4:
		if (UART_InChar(&Packet_Checksum))
			PacketPosition++;
			break;

	case 5:
		if (Checksum_Check())
		{
			PacketPosition = 0;
			return true;
		}
		else
		{
			PacketPosition = 4;
			Packet_Shift();
		}
		break;

	default:
		PacketPosition = 0;
		break;
	}

	return false;
}

/*! @brief Builds a packet and places it in the transmit FIFO buffer.
 *  call 5 times - shift if the check sum is not true
 *
 *  @return bool - TRUE if a valid packet was sent.
 */



bool Packet_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
	uint8_t checkSum = command ^ parameter1 ^ parameter2 ^ parameter3;
  //call UART_OutChar() 5 times
  // 5th byte is calculated -> Checksum
	return (UART_OutChar(command) &&
			UART_OutChar(parameter1) && UART_OutChar(parameter2) &&
			UART_OutChar(parameter3) && UART_OutChar(checkSum));
}

//call UART_outchar 5 times. 5th time is check sum

/* END packet */
/*!
** @}
*/
