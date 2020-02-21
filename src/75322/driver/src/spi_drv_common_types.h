/**
 * @file
 * @brief REGMAP tools data types and API prototypes
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
 * @ingroup spi_tools
 *
 * @details
 *
 */

#ifndef SPI_DRV_COMMON_TYPES_H
#define SPI_DRV_COMMON_TYPES_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

/** Common-purpose API functions result type
 * Intended to handle all possible cases to deliver the driver's functions result */
typedef enum {
    SPI_DRV_FUNC_RES_OK = 0u,           /**< all's ok. Positive func's result */
    SPI_DRV_FUNC_RES_FAIL,              /**< Negative func's result in common sense */
    SPI_DRV_FUNC_RES_FAIL_MEMORY,       /**< Negative result, some wrong memory operation */
    SPI_DRV_FUNC_RES_FAIL_INPUT_CFG,    /**< Negative result, Something is wrong with an input configuration (in input parameters) */
    SPI_DRV_FUNC_RES_FAIL_INPUT_DATA,   /**< Negative result, Something is wrong with an input data (while parsing/reading some extern/global variables */
    SPI_DRV_FUNC_RES_FAIL_COMM,         /**< Negative result, Something is wrong with low_level communication */
    SPI_DRV_FUNC_RES_UNKNOWN = 127u,    /**< Something really unexpected */
} FuncResult_e;

typedef enum {
    CS_SUCCESS = 0x0000,                /**< Communication status's success */
    CS_CRC = 0x0001,
    CS_TYPE = 0x0002,
    CS_SIZE = 0x0004,
    CS_LEN = 0x0008,
    CS_API_BAD_SIZE = 0x0010,
    CS_API_BAD_ARGS = 0x0020,
    CS_TIMEOUT = 0x0040
} ComStat_e;


typedef struct {
    int halStat;
    uint16_t comStat;
    uint32_t devStat;
} DiagDetailsPkt_t;


/** Defines the function's return value type */
typedef bool spiDriver_Status_t;
#define SPI_DRV_TRUE true
#define SPI_DRV_FALSE false

/** Defines the fields names type */
typedef char SpiDriver_FldName_t;

/** Specifies the erroneous value whether it's applicable */
#define SPI_DRV_ERR_VALUE (0xFFFF)

#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* SPI_DRV_COMMON_TYPES_H */

