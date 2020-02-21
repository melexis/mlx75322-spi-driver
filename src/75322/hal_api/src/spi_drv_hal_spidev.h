/**
 * @file
 * @brief HAL SPI API
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
 * @defgroup spi_hal_api_spi_abs SPI hardware abstraction layer interface
 * @ingroup spi_driver
 *
 * @details This component provides a common API interface for SPI control configuration, reagardless the platform used.
 *      For more information refer to platform-specific component
 */

#ifndef SPI_DRV_HAL_SPIDEV_H
#define SPI_DRV_HAL_SPIDEV_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "spi_drv_common_types.h"

/** SPI abstract layer configuration. For more information there should be platform-specific description */
typedef struct {
    uint32_t initDevId;     /**< ID of device to be targeted for all subsequent SPI packet trasnfers */
    uint32_t mode;          /**< The SPI mode configuration (polarity, phase, LSB/MSB order) */
    uint32_t speed;         /**< The SPI baudrate speed of communication */
    uint32_t bitsPerWord;   /**< The number of bits in a 'word' */
} SpiConfig_t;

/** This function configures the SPI connection based on provided platform-specific settings.
 * Refer to platform-specific default for more information
 * @param[in]   spiCfgInput     SPI configuration
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_COMM          Low-level communication operation had failed
 */
FuncResult_e spiDriver_SpiOpenPort(const SpiConfig_t* const spiCfgInput);

/** In a multi-sensor application, this function gets the ID of the current target for
 * subsequent SPI transactions.  Refer to platform-specific default for more
 * information
 *
 * @retval  uint16_t    ID of the currently targeted SPI device
 */
uint16_t spiDriver_SpiGetDev(void);

/** In a multi-sensor application, this function gets the ID of the current target for
 * subsequent SPI transactions.  Refer to platform-specific default for more
 * information
 *
 * @retval  uint16_t    ID of the currently targeted SPI device
 */
uint16_t spiDriver_SpiGetDev(void);

/** In a multi-sensor application, this function sets the current target for
 * subsequent SPI transactions.  Refer to platform-specific default for more
 * information
 *
 * @param[in] devId ID of device to be targeted on subsequent SPI transaction
 * requests.
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 */
FuncResult_e spiDriver_SpiSetDev(uint16_t devId);

/** This function initiates a SPI packet transaction.
 *
 * @param[in] data Entire contents of the SPI MOSI packet to be sent, as an arry of bytes.
 *
 * @param[in] length The 16-bit word count of the SPI MOSI packet to be sent,
 * including header, payload, and CRC fields.
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_COMM          Low-level communication operation had failed
 */
FuncResult_e spiDriver_SpiWriteAndRead(unsigned char* data, int length);

/** Shutdown procedure for the platform-specific SPI peripheral interface
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_COMM          Low-level communication operation had failed
 */
FuncResult_e spiDriver_SpiClosePort(void);

#ifdef __cplusplus
}
#endif

/** @}*/

#endif  /* SPI_DRV_HAL_SPIDEV_H */

