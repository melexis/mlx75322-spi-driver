/**
 * @file
 * @brief Application-level SPI driver (top-level) interface file, for MLX75322
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
 * @defgroup spi_driver Melexis Application-level SPI driver API for MLX75322
 *
 * @details SPI driver provides an API to configure and get a data-access to the data of MLX75322, using the selected
 *      communication layer interface. The driver's API is split into components according to their purpose
 */

#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include "regmap_tools.h"
#include "spi_drv_common_types.h"
#include "spi_drv_data.h"
#include "spi_drv_api.h"
#include "spi_drv_trace.h"
#include "spi_drv_hal_gpio.h"
#include "spi_drv_hal_spidev.h"
#include "spi_drv_tools.h"
#include "cont_mode_lib.h"


#endif /* SPI_DRIVER_H */

