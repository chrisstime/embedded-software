/*! @file
 *
 *  @brief Routines to implement packet encoding and decoding for the serial port.
 *
 *  This contains the functions for implementing the "Tower to PC Protocol" 5-byte packets.
 *
 *  @author PMcL
 *  @date 2015-07-23
 */

#ifndef PACKET_H
#define PACKET_H

// new types
#include "types.h"

// Packet structure
extern uint8_t 	Packet_Command,		/*!< The packet's command */
		Packet_Parameter1, 	/*!< The packet's 1st parameter */
		Packet_Parameter2, 	/*!< The packet's 2nd parameter */
		Packet_Parameter3,	/*!< The packet's 3rd parameter */
		Packet_Checksum;	/*!< The packet's checksum */

// Acknowledgement bit mask
extern const uint8_t PACKET_ACK_MASK;

/*! @brief Initializes the packets by calling the initialization routines of the supporting software modules.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz
 *  @return bool - TRUE if the packet module was successfully initialized.
 */
bool Packet_Init(const uint32_t baudRate, const uint32_t moduleClk);

/*! @brief Attempts to get a packet from the received data.
 *
 *  @return bool - TRUE if a valid packet was received.
 */
bool Packet_Get(void);

/*! @brief Builds a packet and places it in the transmit FIFO buffer.
 *
 *  @return bool - TRUE if a valid packet was sent.
 */
bool Packet_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3);

/*! @brief Shift all the packets by one by passing their value to the previous packet.
 *
 *  @void  - this is purely a function.
 */
void Packet_Shift();

/*! @brief Calculates the checksum by XORing the values of Packet Command to Packet 3.
 *
 *  @return uint8_t - returns the calculated checksum value
 */
uint8_t Calculated_Checksum(void);

/*! @brief Compares the actual value of checksum with the calculated checksum based on the Checksum_Check() function.
 *  static because we will only ever use this within this .c file
 *
 *  @return bool - TRUE if calculated checksum matches the value checksum packet.
 */
static bool Checksum_Check(void);

#endif
