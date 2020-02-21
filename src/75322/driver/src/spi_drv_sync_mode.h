/**
 * @file
 * @brief SPI driver syncronous multi-IC configuration support
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
 * @ingroup spi_com_sync
 *
 * @details
 *
 * This API provides the multi-IC syncronous mode data acquisition support. To run the sychronous mode,
 * there are several points that should be assured in HW:
 *
 * - The ICs should be connected with cross-IC wire, organising their chains. There should be one IC selected,
 *     havining the master's role.
 * - All ICs should be configured with similar characteristics, assuring there equivalent time schedule. This
 *   means all layer's configurations should be the same in terms of:
 *     - "raw-data" or "trace" selection
 *     - samples number choice;
 *     - layers number;
 * - All the ICs are working for one scene, regardless of their HW bus connections. Thus, it does not matter which
 *  SPI bus(es) or which chip-selects are used.
 *
 * @{
 */
#ifndef SPI_DRV_SYNC_MODE_H
#define SPI_DRV_SYNC_MODE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "spi_drv_common_types.h"
#include "cont_mode_lib.h"
#include "spi_drv_trace.h"
#include "spi_drv_sync_com.h"

/* Use SYNC_MODE_DEBUG=1 option to add syncronous mode debug output */
#ifndef SYNC_MODE_DEBUG
#define SYNC_MODE_DEBUG 0
#endif /* SYNC_MODE_DEBUG */

#if (SYNC_MODE_DEBUG == 1)
#define SYNC_PRINT printf
#else
#define SYNC_PRINT(...)
#endif

/** Modes of synchronization with the host */
typedef enum {
    ACQU_SYNC_LAYER = 0, /**< AcquStart command from host will be required before every layer start (to timely switch light sources) */
    ACQU_SYNC_FRAME,     /**< AcquStart command from host will be required before every frame start (required option to work with `hw_sync`) */
    ACQU_SYNC_NONE,      /**< AcquStart command from host will not be required and triggered automatically (i.e. former 'continuous' mode) */
} AcquSyncMode;

/** Synchronous mode configuration */
typedef struct {
    uint16_t icCount;       /**< IC's count in a sequence the very first one is master */
} SyncModeCfg_t;

extern SyncModeCfg_t syncModeCfg;


/** Inititates the syncronous mode
 * The function copies the configuration to the local variables
 * @param[in]   cfg     Syncronous mode initialization
 * @return      result of an operation
 */
FuncResult_e spiDriver_SyncModeInit(const SyncModeCfg_t* cfg);


/** Configures the sequence of IC IDs in the sequence
 * This function configures the set of IC IDs to be used for syncronous mode. This data is copied into the internal
 * structure and used as a reference for data aqcusition. The very first IC id is considered as a MASTER in sync mode sequence
 * @note this function should be called before any continuous or single modes of data aqcuisition
 *
 * @param[in]   icIds           IC IDs sequence
 * @param[in]   icCount         The number of ICs in a set

 * @retval      SPI_DRV_FUNC_RES_OK     positive result of an operation
 * @retval      SPI_DRV_FUNC_RES_FAIL_INPUT_CFG input configuration has some errors
 */
FuncResult_e spiDriver_SetSyncIcOrder(const uint16_t* const icIds, const uint16_t icCount);


/** Organizes the Wait_For_Ready loop considering the multi-IC configuration
 * Waits for all ICs involved in the process. This function supports the "single-IC" config when ::SyncModeCfg_t.icCount < 2.
 * @retval  SPI_DRV_FUNC_RES_OK             when ::spiDriver_PinWaitForReady returned ::CS_SUCCESS for all ICs
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG when the multi-IC configuration is not set and/or should be reconfigured
 * @retval  SPI_DRV_FUNC_RES_FAIL_COMM      when ::spiDriver_PinWaitForReady returned not ::CS_SUCCESS for any of IC in a list
 */
FuncResult_e spiDriver_SyncPinWaitForReady(void);


/** Checks the current IC configurations to ensure the ICs can be executed within the synchronous mode with their settings
 * This function compares the current configuration of ICs of:
 *   - `layer->averaging` should match;
 *   - `layer->trigger_period` should match;
 *   - `layer->sampling.mode` should match;
 *   - `layer->dark_frame_en` should match;
 *   - `scene->lsm_configs[layer_index].lsm_enable` should match
 *   - `scene->sync_mode` should match;
 *   - `scene->recharge_led_en` should match;
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     the ICs configuration is incorrect
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_DATA    the driver's configuration of layers is incorrect
 * @retval  SPI_DRV_FUNC_RES_FAIL_COMM          Low-level communication operation had failed
 */
FuncResult_e spiDriver_CheckSyncConfig(void);


#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* SPI_DRV_SYNC_MODE_H */

