/**
 * @file
 * @brief HAL GPIO API
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
 * @defgroup spi_hal_api_gpio Empty (weak) GPIO HAL library
 * @ingroup spi_hal_api_gpio_abs
 *
 * @details HAL GPIO library provides the common API interface for the SPI driver GPIO configuration functions
 */

#include <stdio.h>
#include "spi_drv_common_types.h"
#include "spi_drv_hal_gpio.h"

__attribute__((weak)) FuncResult_e spiDriver_PinInit(const GpioConfig_t* const pinCfg)
{
    (void)pinCfg;
    printf("GPIO PinInit\n");
    return SPI_DRV_FUNC_RES_OK;
}

__attribute__((weak)) FuncResult_e spiDriver_PinResetAsic(void)
{
    printf("GPIO PinResetASIC\n");
    return SPI_DRV_FUNC_RES_OK;
}

__attribute__((weak)) FuncResult_e spiDriver_PinSetDev(uint16_t devId)
{
    printf("GPIO PinSetDev = %d\n", devId);
    return SPI_DRV_FUNC_RES_OK;
}

__attribute__((weak)) uint16_t spiDriver_PinGetSel(void)
{
    printf("GPIO PinGetSel\n");
    return 0;
}

__attribute__((weak)) FuncResult_e spiDriver_PinSetSel(uint16_t fourBits)
{
    (void)fourBits;
    printf("GPIO PinSetSel\n");
    return SPI_DRV_FUNC_RES_OK;
}

__attribute__((weak)) ComStat_e spiDriver_PinWaitForReady(void)
{
    printf("GPIO PinWaitForReady\n");
    return CS_SUCCESS;
}

