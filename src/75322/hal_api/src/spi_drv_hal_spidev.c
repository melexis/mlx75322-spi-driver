/**
 * @file
 * @brief Miscellanious tools and helpers for common purpose
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
 * @defgroup spi_hal_api_spi Empty (weak) SPI HAL library
 * @ingroup spi_hal_api_spi_abs
 *
 * @details HAL SPI library provides the common API interface for the SPI driver SPI interface configuration functions
 */

#include "stdio.h"
#include "spi_drv_hal_spidev.h"

SpiConfig_t spiCfg;

__attribute__((weak)) FuncResult_e spiDriver_SpiOpenPort(const SpiConfig_t* const spiCfgInput)
{
    if (spiCfgInput != NULL) {
        spiCfg = *spiCfgInput;
        printf("Configured init SPI devId = %d\n", spiCfgInput->initDevId);
    } else {
        printf("Using default SPI devId\n");
    }
    return SPI_DRV_FUNC_RES_OK;
}

__attribute__((weak)) FuncResult_e spiDriver_SpiSetDev(uint16_t devId)
{
    printf("SPI Set device: %d\n", devId);
    return CS_SUCCESS;
}

__attribute__((weak)) uint16_t spiDriver_SpiGetDev(void)
{
    return 0;
}

__attribute__((weak)) FuncResult_e spiDriver_SpiWriteAndRead(unsigned char* data, int length)
{
    unsigned char* ptr_data = data;

    printf("\nSPI Write and Read\n");

#ifdef DEBUG_SPI_COM
    for (int ii = 0; ii < length; ii++ ) {
        printf("spiDemo.c::SpiWriteAndRead(): ii = %d, *ptr_data = 0x%02X \n", ii, *ptr_data);
        *ptr_data++ = ii;
    }
#else
    printf("spiDemo.c::SpiWriteAndRead(): packet length: %d, *ptr_data = 0x%02X \n", length, *ptr_data);
#endif

    printf("spiDemo.c::SpiWriteAndRead(): return\n\n");

    return SPI_DRV_FUNC_RES_OK;
}

__attribute__((weak)) FuncResult_e spiDriver_SpiClosePort(void)
{
    printf("Closing all SPI devices\n");
    return SPI_DRV_FUNC_RES_OK;
}

