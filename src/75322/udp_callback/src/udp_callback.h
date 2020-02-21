/**
 * @file
 * @brief UDP custom callback implementation
 * @internal
 *
 * @copyright (C) 2020 Melexis N.V.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.  MELEXIS N.V. SHALL NOT IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * @endinternal
 *
 * @defgroup spi_udp SPI UDP callback implementation
 * @ingroup spi_driver
 *
 * @details This component provides a common API interface for SPI control configuration, reagardless the platform used.
 *      For more information refer to platform-specific component
 */

#include "cont_mode_lib.h"

/** Initializes the UDP socket and driver's callback function
 * @param[in]   dest_port   Destination port for the UDP-packets. If this parameter is 0, default value ::DEST_PORT is used.
 *
 * @note This function should be called before the first running of the continuous mode with the ::spiDriver_UdpCallback.
 */
void spiDriver_InitUdpCallback(uint16_t dest_port);

/** Implements the UDP socket callback function
 * This function should be used with the continuous mode as an explicit callback
 *
 * @param[in] chipData    the pointer used to transfer the data from the driver into the callback function
 * @return                callback's result of an operation, whether the flow should be continued.
 */
ContModeCbRet_t spiDriver_UdpCallback(spiDriver_ChipData_t* chipData);

