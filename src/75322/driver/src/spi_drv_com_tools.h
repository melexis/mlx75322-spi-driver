/**
 * @file
 * @brief Internal data defines, typedefs, and function declarations for low-level SPI Comm driver operations
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
 * @defgroup spi_com SPI protocol communication layer
 * @ingroup spi_api
 *
 * @details is the communication layer protocol. SPI communication provides the abstraction for IC(s)
 *     communication for selected platform. Thus, this part is platform-depended. These functions are allowed but
 *     preferred to be used from upper driver's layers and not from an application. Using them directly from an
 *     application should be used with an additional care and require good understanding of the data exchange flow.
 *
 */

#ifndef SPI_DRV_COM_TOOLS_H
#define SPI_DRV_COM_TOOLS_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "spi_drv_common_types.h"
#include "spi_drv_hal_spidev.h"
#include "spi_drv_hal_gpio.h"

#define DEV_STAT_IGNORE_MASK 0x80080001

#define BYTES_PER_WORD 2
#define MAX_PATCH_RAM_BYTES 0x3000
#define MAX_PATCH_RAM_WORDS (MAX_PATCH_RAM_BYTES / BYTES_PER_WORD)
#define MAX_PKT_PAYLOAD_WORDS 1208
#define PKT_HEADER_WORDS 1
#define PKT_CRC_WORDS 1
#define MAX_PKT_WORDS (MAX_PKT_PAYLOAD_WORDS + PKT_HEADER_WORDS + PKT_CRC_WORDS)
#define MAX_PKT_BYTES (MAX_PKT_WORDS * BYTES_PER_WORD)

#define MAX_RW_SIZE 256

#ifndef COM_DEBUG_DETAIL_0
#define COM_DEBUG_DETAIL_0 0
#endif

#ifndef COM_DEBUG_DETAIL_1
#define COM_DEBUG_DETAIL_1 0
#endif

#ifndef COM_DEBUG_DETAIL_2
#define COM_DEBUG_DETAIL_2 0
#endif

#if (COM_DEBUG_DETAIL_0 == 1) || \
    (COM_DEBUG_DETAIL_1 == 1) || \
    (COM_DEBUG_DETAIL_2 == 1)
#include <stdio.h>
FILE* comDebugFile;
#define COM_DEBUG_PRINT fprintf
#else
#define COM_DEBUG_PRINT(...)
#endif

/* ---------------- Types ---------------- */

/** HW-layer configuration set */
typedef struct {
    GpioConfig_t* pinCfg;       /**< Pointer to GPIO pins' set configuration, can be set NULL to use default values */
    SpiConfig_t* spiCfg;        /**< SPI configuration, can be set NULL to use default settings */
} SpiComConfig_t;


typedef enum PktType_e {
    READ = 0,
    WRITE = 1,
    FUNCTION = 2,
    /* Reserved = 3, */
    STATUS_SHORT = 4,
    STATUS_LONG = 5,
    READ_DATA_RESP_SHORT = 6,
    READ_DATA_RESP_LONG = 7,
    WRITE_DATA_LONG = 8,
    /* Reserved = 9,  */
    /* Reserved = 10, */
    /* Reserved = 11, */
    ECHO_DATA_RESP = 12,
    RAW_DATA_RESP = 13,
    SYNC = 14,
    WRITE_PATCH = 15
} PktType_t;

/** The FunctionId_e enumeration defines Function ID values for use in the FunctionID field of the FUNCTION SPI Packet Type.) */
typedef enum FunctionId_e {
    GET_ECHO = 1,           /**< Request for Echo data from ASIC.*/
    GET_RAW = 2,            /**< Request for Raw data from ASIC. */
    ACQU_SYNC = 3,          /**< Synchronization command to the ASIC. */
    SENSOR_START = 4,       /**< Starts data acquistion sequence in ASIC. */
    SENSOR_STOP = 5,        /**< Stops data acquistion sequence in ASIC. */
    SENSOR_STANDBY = 6,     /**< Request to put ASIC into Standby mode. */
    APPLY_PATCH = 7,        /**< Request to for ASIC to link in previously transmit firmware patch code. */
    /* Reserved = 8, */
    /* Reserved = 9 */
} FunctionId_t;

/* ---------------- Functions ---------------- */

void clearDiagDetails(void);

uint16_t calcCrc(uint16_t wordSizeOfCrcEnvelope, uint16_t* wordBuf);

#if (COM_DEBUG_DETAIL_1 == 1) || \
    (COM_DEBUG_DETAIL_2 == 1)
void printPtype(uint16_t ptype);
#endif

FuncResult_e makeSpiPacket(uint16_t ptype,  uint16_t sizeField,  uint16_t sizePayload, uint16_t* payload);

FuncResult_e validatePkt(uint8_t pktNum, uint16_t expectedPtype, uint16_t xactSize, uint16_t* validateBuf);

#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* SPI_DRV_COM_TOOLS_H */

