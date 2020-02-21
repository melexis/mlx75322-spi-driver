/**
 * @file
 * @brief HAL UDP API
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
 * @defgroup spi_hal_api_udp_abs SPI UDP abstraction layer interface
 * @ingroup spi_driver
 *
 * @details This component provides a common API interface for UDP control configuration, reagardless the platform used.
 *      For more information refer to platform-specific component. This API assumes working with the continuous mode component.
 */

#ifndef SPI_DRV_HAL_UDP_H
#define SPI_DRV_HAL_UDP_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "cont_mode_lib.h"

/** UDP destination port for socket's packets */
#define DEST_PORT 8541

/** Provides the API for UDP's callback function used to transfer the data via UDP
 * @param[in] chipData    the pointer used to transfer the data from the driver into the callback function
 *
 * @note This function is overloaded by the UDP-component's implementation
 */
ContModeCbRet_t spiDriver_UdpCallback(spiDriver_ChipData_t* chipData);

/** Provides initialization functions called from continuous mode initialization
 *
 * @param[in]   dest_port   Destination port for the UDP-packets. If this parameter is 0, default value ::DEST_PORT is used.
 * @return                callback's result of an operation, whether the flow should be continued.
 *
 * @note This function is overloaded by the UDP-component's implementation
 */
void spiDriver_InitUdpCallback(uint16_t dest_port);

#ifdef __cplusplus
}
#endif

/** @}*/

#endif  /* SPI_DRV_HAL_UDP_H */

