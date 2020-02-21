/**
 * @file
 * @brief API for different SPI driver layers, which provides the functions for the synchronouse mode of Multi-IC configuration
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
 * @defgroup spi_com_sync Synchronous multi-IC mode functions
 * @ingroup spi_driver
 *
 * @details In the synchronous mode the ICs should be accessed in special manner, providing the synchronisation phases and data aqcuisition in the sequential manner.
 */

#ifndef SPI_DRV_SYNC_COM_H
#define SPI_DRV_SYNC_COM_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "spi_drv_com.h"

/** The maximum number of ICs handled by the driver */
#define MAX_IC_ID_NUMBER 16
/** The IC's id used to handle the command as a broadcast message */
#define IC_ID_BROADCAST MAX_IC_ID_NUMBER


/* ---------------- External Functions for the multiple function calls, based on IC's IDs (taken from the script/configuration file ---------------- */

/** Sends the multiple commands to multiple ICs if requested
 * Checks whether the input parameter (id) is a BROADCAST, and can iterate the sequence.
 * @param[in]   id          IC's identifier, index configured by ::spiDriver_SetupMultiICs
 * @param[in]   varName     variable's name
 * @param[in]   value       value to set
 * @param[in]   bitFieldName specifies the bit-fieldname, if such option is avaialable for the
 *      variable with varName. Can be omitted by setting to an empty string or NULL
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input variable name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL               Low-level communication operation had failed
 */
FuncResult_e spiDriver_SetMultiByName(const uint16_t id,
                                      const SpiDriver_FldName_t* const varName,
                                      uint32_t value,
                                      const SpiDriver_FldName_t* const bitFieldName);


/** Gets the multiple variables by variable name via its offset
 * @param[in]   id          IC's identifier, index configured by ::spiDriver_SetupMultiICs
 * @param[in]   varName     variable's name
 * @param[in]   values       values array buffer to read and store the data from ICs (in case of broadcast. For the single mode only the one value pointer is needed)
 * @param[in]   bitFieldName specifies the bit-fieldname, if such option is avaialable for the
 *      variable with varName. Can be omitted by setting to an empty string or NULL
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input variable name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL               Low-level communication operation had failed
 */
FuncResult_e spiDriver_GetMultiByName(const uint16_t id,
                                      const SpiDriver_FldName_t* const varName,
                                      uint32_t* values,
                                      const SpiDriver_FldName_t* const bitFieldName);



/* ---------------- External Functions for the multiple function calls, based on IC's order---------------- */

/** Sets the variable by variable name via its offset
 * @param[in]   varName     variable's name
 * @param[in]   value       value to set
 * @param[in]   bitFieldName specifies the bit-fieldname, if such option is avaialable for the
 *      variable with varName. Can be omitted by setting to an empty string or NULL
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input variable name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL               Low-level communication operation had failed
 */
FuncResult_e spiDriver_SetSyncByName(const SpiDriver_FldName_t* const varName,
                                     uint32_t value,
                                     const SpiDriver_FldName_t* const bitFieldName);


/** Uploads the patch into the IC
 * @param[in]       offset  data initial offset
 * @param[in]       size    data size to write
 * @param[in]       dataBuf a pointer to data to upload
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input variable name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL_COMM          Low-level communication operation had failed
 */
FuncResult_e spiCom_WriteSyncPatch(uint32_t offset, uint32_t size, uint8_t* dataBuf);


/** Applies a patch */
FuncResult_e spiCom_ApplySyncPatch(void);


/** Starts sensors */
FuncResult_e spiCom_SensorSyncStart(void);


/** Stops sensors */
FuncResult_e spiCom_SensorSyncStop(void);


/** Runs standby */
FuncResult_e spiCom_SensorSyncStandby(void);


/** Sends AcquSync signal for ICs */
FuncResult_e spiCom_AcquSyncSync(void);


/** Sends Sync signals */
FuncResult_e spiCom_SyncSync(void);


/** Waits for the READY pin of currently targeted ASIC to be asserted
 * @retval   CS_SUCCESS     READY pin asserted within the set timeout period
 * @retval   CS_TIMEOUT     READY pin was not asserted within the timeout period
 */
ComStat_e spiCom_WaitSyncForReady(void);

/** Returns the IC's index looking it within the multi-IC mode configuration
 * @param[in] chip_id       The IC's ID to search for
 * @return               IC index. If the value is greater or equal to ::MAX_IC_ID_NUMBER then the ID was not found in configuration
 */
uint16_t spiDriver_GetIcIndexById(const uint16_t chip_id);

#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* SPI_DRV_SYNC_COM_H */

