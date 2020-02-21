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
 * @defgroup spi_hal_api_gpio_abs GPIO hardware abstraction layer interface
 * @ingroup spi_driver
 *
 * @details This component provides a common API interface for GPIO control configuration, reagardless the platform used.
 *      For more information refer to platform-specific component
 */

#ifndef SPI_HAL_GPIO_H
#define SPI_HAL_GPIO_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "spi_drv_common_types.h"

/** Common GPIO description for initialization. For more information refer to
 * platform-specific description, and/or default settings for it.
 */
typedef struct {
    uint32_t port;          /**< Port ID */
    uint32_t mode;          /**< Pin mode */
    uint32_t pin;           /**< Pin ID */
    uint32_t miscFlags;     /**< Additional flags for initialization */
} GPIO_CommonConfig_t;

/** SPI driver's GPIO pins configuration */
typedef struct {
    uint16_t initDevId;
    GPIO_CommonConfig_t resetPin; /**< Reset pin configuration */
    GPIO_CommonConfig_t ready0Pin; /**< Ready pin configuration */
    GPIO_CommonConfig_t ready1Pin; /**< Ready pin configuration */
    GPIO_CommonConfig_t sel1Pin; /**< SEL1 pin configuration */
    GPIO_CommonConfig_t sel2Pin; /**< SEL2 pin configuration */
    GPIO_CommonConfig_t sel3Pin; /**< SEL3 pin configuration */
    GPIO_CommonConfig_t sel4Pin; /**< SEL4 pin configuration */
} GpioConfig_t;

/** Configures the host GPIO pins for the 75322 application. Example settings
 * might include, depending on platform specifics: pin assignment, I/O
 * direction, pullup/pulldown, initial value
 *
 * @param[in] pinCfg The platform-specific configuration structure. If NULL is
 * provided, default configuration will be used.
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 */
extern FuncResult_e spiDriver_PinInit(const GpioConfig_t* const pinCfg);

/** Asserts RST_B pin, waits RESET_ASSERTION_WIDTH_MS (milliseconds), Deasserts
 * RST_B pin, waits RESET_RECOVERY_TIME_MS (milliseconds).
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 */
extern FuncResult_e spiDriver_PinResetAsic(void);

/** For use in a multi-sensor application, this function stores a value to
 * global variable devIdPinGlobal, which will be used by the platform specific
 * spiDriver_PinWaitForReady() function to know which 75322 ASIC is the current
 * target for SPI transaction, and therefore which READY pin should be waited
 * upon.  This function should always be called in conjunction with
 * spiDriver_SpiSetDev() so as to coordinate device selection between the GPIO
 * and the SPI Comm modules. On startup, by default, devId 0 is selected by the
 * platform specific modules.
 *
 * @param[in] devId The Id of the device to be selected for subsequent SPI
 * transactions. Values must be integers from 0 to N-1, where N is the
 * number of devices in the application.
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 */
extern FuncResult_e spiDriver_PinSetDev(uint16_t devId);

/** Returns the current value of SEL lines (GPIO pins used to select a light
 * source for the current layer).
 *
 * @retval value of SEL lines
 */
extern uint16_t spiDriver_PinGetSel(void);

/** For use in a multi-layer multi-illumination-source application. This
 * function will set GPIO pins ("SEL" lines) according to the value passed
 * in. Used to enable (select, but not trigger) a light source.
 *
 * @param[in] fourBits A mask value to indicate which of the SEL lines to
 * assert.
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 */
extern FuncResult_e spiDriver_PinSetSel(uint16_t fourBits);

/** Blocks the code execution until the READY signal asserted.
 *
 * @retval CS_SUCCESS  READY was asserted within the timeout interval
 * @retval CS_TIMEOUT  READY was not asserted within the timeout interval
 */
extern ComStat_e spiDriver_PinWaitForReady(void);

#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* SPI_HAL_MISC_H */

