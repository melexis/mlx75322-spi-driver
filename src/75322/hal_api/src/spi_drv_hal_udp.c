/**
 * @file
 * @brief API for UDP functions used optionally as data transfer interface
 * @internal
 *
 * @copyright (C) 2019 Melexis N.V.
 *
 * Melexis N.V. is supplying this code for use with Melexis N.V. processor based microcontrollers only.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.  MELEXIS N.V. SHALL NOT IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * @endinternal
 *
 * @defgroup spi_hal_api_udp Empty (weak) UDP library
 * @ingroup spi_hal_api_udp_abs
 *
 * @details UDP SPI library provides the common API interface for the SPI driver of UDP interface
 *     configuration and data tranfer functions
 */

#include "stdio.h"
#include "spi_drv_hal_udp.h"

__attribute__((weak)) void spiDriver_InitUdpCallback(uint16_t dest_port)
{
    printf("Init the UDP interface, port:%u\n", dest_port);
}

__attribute__((weak)) ContModeCbRet_t spiDriver_UdpCallback(spiDriver_ChipData_t* chipData)
{
    printf("\nUDP callback processing: Data offset is %08lx\n", (unsigned long)chipData);
    return CB_RET_OK;
}

