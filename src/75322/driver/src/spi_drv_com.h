/**
 * @file
 * @brief API for low-level SPI driver interaction
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
 * @addtogroup spi_com
 * @ingroup spi_api
 *
 * @details is the communication layer protocol. SPI communication provides the abstraction for IC(s)
 *     communication for selected platform. Thus, this part is platform-depended. These functions are allowed but
 *     preferred to be used from upper driver's layers and not from an application. Using them directly from an
 *     application should be used with an additional care and require good understanding of the data exchange flow.
 *
 */

#ifndef SPI_DRV_COM_H
#define SPI_DRV_COM_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "spi_drv_common_types.h"
#include "spi_drv_com_tools.h"
#include "spi_drv_hal_spidev.h"
#include "spi_drv_hal_gpio.h"

/* ---------------- External Functions ---------------- */

/** Calls platform-specific Host GPIO and SPI Communication subsystem initialialization functions.
 *
 * @param[in] comCfg Configuration structure which sets initial setting values
 * for the platform specific GPIO and SPI Comm components.  If NULL - the
 * default configuration will be used. See platform-specific spec about default
 * configuration
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_COMM               Low-level initialization had failed
 */
FuncResult_e spiCom_Init(const SpiComConfig_t* const comCfg);

/** Calls platform-specific function to apply a reset sequence on a Host pin
 * which connects to the RST_B pin of all attached 75322 ASICs.
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 */
FuncResult_e spiCom_ResetASIC(void);

/** For use in a multi-sensor application. This function calls platform-specific
 * functions to select one sensor to be the current target for subsequent SPI
 * transactions. Only call this function when the application needs to switch
 * the currently selected target ASIC. The platform-specific functions use
 * global variables to store the current selection.
 *
 * @param[in]   devId      Id of device for all subsequent packet transfers
 * @retval   CS_SUCCESS     Always success, no checks performed in this release
 */
FuncResult_e spiCom_SetDev(uint16_t devId);

/** Sets selection of light source via GPIO pins **/
FuncResult_e spiCom_SetSel(uint16_t fourBits);

/** Waits for the READY pin of currently targeted ASIC to be asserted
 * @retval   CS_SUCCESS     READY pin asserted within the set timeout period
 * @retval   CS_TIMEOUT     READY pin was not asserted within the timeout period
 */
ComStat_e spiCom_WaitForReady(void);

/** Gets the value via its offset
 * @param[in]   offset     variable's name
 * @param[in]   wordSize   variable's size, in bytes
 * @param[in]   read_words value's buffer (pointer) to read and store the data
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input variable name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL_COMM          Low-level communication operation had failed
 */
FuncResult_e spiCom_Read(const uint16_t offset, const uint16_t wordSize, uint16_t* read_words);

/** Sets the value through its offset
 * @param[in]   offset     variable's name
 * @param[in]   wordSize   variable's size, in bytes
 * @param[in]   write_words pointer to a data to write
 * @param[in]   patch
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input variable name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL_COMM          Low-level communication operation had failed
 */
FuncResult_e spiCom_Write(const uint16_t offset,const uint16_t wordSize,uint16_t* write_words,const bool patch);

/** Uploads the patch into the IC
 * @param[in]       offset  data initial offset
 * @param[in]       size    data size to write
 * @param[in]       dataBuf a pointer to data to upload
 *
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input variable name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL_COMM          Low-level communication operation had failed
 */
FuncResult_e spiCom_WritePatch(uint32_t offset, uint32_t size, uint8_t* dataBuf);

/** Applies a patch */
FuncResult_e spiCom_ApplyPatch(void);

/** Starts sensor acquisition stream */
FuncResult_e spiCom_SensorStart(void);

/** Sends AcquSync packet as a synchronization event */
FuncResult_e spiCom_AcquSync(void);

/** Sends Sync packet as a synchronization event */
FuncResult_e spiCom_Sync(void);

/** Gets 1 Frame (16 channels) of Raw Trace data and corresponding Metadata
 * @param[in]   layersAndSamples Indicates size (in words) of each channel's Raw Trace data set, plus 8 (for channel rawMetaData), set for the Layer
 * @param[out]  trace            16 Raw Trace data sets, maximum of 312 x 16-bit words x 32 channels
 * @param[out]  rawMetaData      16 Raw Metadata structures, one for each channel of the Frame. 8 x 16-bit words
 * @retval  SPI_DRV_FUNC_RES_OK  Operation is successful
 */
FuncResult_e spiCom_GetRaw(uint16_t layersAndSamples, uint16_t* trace, uint16_t* rawMetaData);

/** Gets 1 Layer (30 channels) of Echoes and corresponding Metadata
 * @param[in]   echoByte         Indicates size (in words) of the payload of the 2nd packet of the transaction (ECHO_DATA_RESP), set according to the current Echo Format configuration.
 *                                   If Echo Format = FMT_ECHO_FAST,     SIZE = 1208 dec
 *                                   If Echo Format = FMT_ECHO_9P,       SIZE = 1208 dec
 *                                   If Echo Format = FMT_ECHO_SHORT,    SIZE =  368 dec
 *                                   If Echo Format = FMT_ECHO_DETAIL_1, SIZE = 1208 dec
 * @param[out]  EchoesData       1 Layer of Echoes. maximum of 1200 x 16-bit words = 4 echo structures per channel with maximum echo struct size set (10 words per struct)
 * @param[out]  echoMetaData     1 Echo Metadata structure for the Layer. 8 x 16-bit words
 * @retval  SPI_DRV_FUNC_RES_OK  Operation is successful
 */
FuncResult_e spiCom_GetEcho(uint16_t echoByte, uint16_t* EchoesData, uint16_t* echoMetaData);

/** Stops sensor aquisition stream */
FuncResult_e spiCom_SensorStop(void);

/** Places sensor into standby mode */
FuncResult_e spiCom_SensorStandby(void);

#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* SPI_DRV_COM_H */

