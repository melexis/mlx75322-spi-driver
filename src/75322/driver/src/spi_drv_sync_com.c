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
 * @ingroup spi_com_sync
 */


/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "spi_drv_com.h"
#include "spi_drv_api.h"
#include "spi_drv_sync_mode.h"
#include "spi_drv_sync_com.h"

extern volatile SpiDriver_State_t spiDriver_currentState;

FuncResult_e spiDriver_SetMultiByName(const uint16_t id,
                                      const SpiDriver_FldName_t* const varName,
                                      uint32_t value,
                                      const SpiDriver_FldName_t* const bitFieldName)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    SYNC_PRINT("Set parameter %s[%s] to %u for IC(s) %u\n", varName, bitFieldName, value, id);
    if (id == IC_ID_BROADCAST) {
        uint16_t ind;
        for (ind = 0u; ind < icIntNamesNumber; ind++) {
            res |= spiCom_SetDev(ind);
            res |= spiDriver_SetByName(varName, value, bitFieldName);
        }
    } else {
        res |= spiCom_SetDev(id);
        res |= spiDriver_SetByName(varName, value, bitFieldName);
    }
    return res;
}


FuncResult_e spiDriver_GetMultiByName(const uint16_t id,
                                      const SpiDriver_FldName_t* const varName,
                                      uint32_t* values,
                                      const SpiDriver_FldName_t* const bitFieldName)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    SYNC_PRINT("Get parameter %s[%s] from IC(s) %u\n", varName, bitFieldName, id);
    if (id == IC_ID_BROADCAST) {
        uint16_t ind;
        for (ind = 0u; ind < icIntNamesNumber; ind++) {
            res |= spiCom_SetDev(ind);
            res |= spiDriver_GetByName(varName, values, bitFieldName);
            values++;
        }
    } else {
        res |= spiCom_SetDev(id);
        res |= spiDriver_GetByName(varName, values, bitFieldName);
    }
    return res;
}

/* *** Set of functions used for sending multiple commands by ICs order *** */

FuncResult_e spiDriver_SetSyncByName(const SpiDriver_FldName_t* const varName,
                                     uint32_t value,
                                     const SpiDriver_FldName_t* const bitFieldName)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    SYNC_PRINT("Set var %s[%s] = 0x%04x for %u ICs\n", varName, bitFieldName, value, syncModeCfg.icCount);
#if (SYNC_TEST_FLOW != 1)
    uint16_t ind;
    uint16_t ic;
    if (syncModeCfg.icCount > 1u) {
        for (ind = 0u; ind < syncModeCfg.icCount; ind++) {
            ic = syncModeCfg.icCount - ind - 1;
            spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
            res = spiDriver_SetByName(varName, value, bitFieldName);
        }
    } else
#else
    spiCom_SetDev(0u);
#endif
    {
        res = spiDriver_SetByName(varName, value, bitFieldName);
    }
    return res;
}


FuncResult_e spiCom_WriteSyncPatch(uint32_t offset, uint32_t size, uint8_t* dataBuf)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    SYNC_PRINT("Write PATCH for %u ICs\n", syncModeCfg.icCount);
#if (SYNC_TEST_FLOW != 1)
    uint16_t ind;
    uint16_t ic;
    if (syncModeCfg.icCount > 1u) {
        for (ind = 0u; ind < syncModeCfg.icCount; ind++) {
            ic = syncModeCfg.icCount - ind - 1;
            spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
            SYNC_PRINT("Writing the patch to IC %u\n", spiDriver_currentState.params[ic].icIndex);
            res = spiCom_WritePatch(offset, size, dataBuf);
        }
    } else
#endif
    {
        res = spiCom_WritePatch(offset, size, dataBuf);
    }
    return res;
}


FuncResult_e spiCom_ApplySyncPatch(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    SYNC_PRINT("Apply PATCH for %u ICs\n", syncModeCfg.icCount);
#if (SYNC_TEST_FLOW != 1)
    uint16_t ind;
    uint16_t ic;
    if (syncModeCfg.icCount > 1u) {
        for (ind = 0u; ind < syncModeCfg.icCount; ind++) {
            ic = syncModeCfg.icCount - ind - 1;
            spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
            SYNC_PRINT("Applying the patch to IC %u\n", spiDriver_currentState.params[ic].icIndex);
            res = spiCom_ApplyPatch();
        }
    } else
#endif
    {
        res = spiCom_ApplyPatch();
    }
    return res;
}


FuncResult_e spiCom_SensorSyncStart(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    SYNC_PRINT("START for %u ICs\n", syncModeCfg.icCount);
#if (SYNC_TEST_FLOW != 1)
    uint16_t ind;
    uint16_t ic;
    if (syncModeCfg.icCount > 1u) {
        for (ind = 0u; ind < syncModeCfg.icCount; ind++) {
            ic = syncModeCfg.icCount - ind - 1;
            spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
            spiDriver_currentState.params[ic].sceneCurrentLayer = 0u;
            SYNC_PRINT("Start IC %u\n", spiDriver_currentState.params[ic].icIndex);
            res = spiCom_SensorStart();
        }
    } else
#else
    spiCom_SetDev(0u);
#endif
    {
        spiDriver_currentState.params[0u].sceneCurrentLayer = 0u;
        res = spiCom_SensorStart();
    }
    return res;
}


FuncResult_e spiCom_SensorSyncStop(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    SYNC_PRINT("STOP for %u ICs\n", syncModeCfg.icCount);
#if (SYNC_TEST_FLOW != 1)
    uint16_t ind;
    uint16_t ic;
    if (syncModeCfg.icCount > 1u) {
        for (ind = 0u; ind < syncModeCfg.icCount; ind++) {
            ic = syncModeCfg.icCount - ind - 1;
            spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
            SYNC_PRINT("Stop IC %u\n", spiDriver_currentState.params[ic].icIndex);
            res = spiCom_SensorStop();
        }
    } else
#endif
    {
        res = spiCom_SensorStop();
    }
    return res;
}


FuncResult_e spiCom_SensorSyncStandby(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t ind;
    uint16_t ic;
    if (syncModeCfg.icCount > 1u) {
        for (ind = 0u; ind < syncModeCfg.icCount; ind++) {
            ic = syncModeCfg.icCount - ind - 1;
            spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
            SYNC_PRINT("Standby IC %u\n", spiDriver_currentState.params[ic].icIndex);
            res = spiCom_SensorStandby();
        }
    } else {
        res = spiCom_SensorStandby();
    }
    return res;
}


FuncResult_e spiCom_AcquSyncSync(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    SYNC_PRINT("AcquSync for %u ICs\n", syncModeCfg.icCount);
#if (SYNC_TEST_FLOW != 1)
    uint16_t ind;
    uint16_t ic;
    if (syncModeCfg.icCount > 1u) {
        for (ind = 0u; ind < syncModeCfg.icCount; ind++) {
            ic = syncModeCfg.icCount - ind - 1;
            spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
            SYNC_PRINT("Acqu sync IC %u\n", spiDriver_currentState.params[ic].icIndex);
            res = spiCom_AcquSync();
        }
    } else
#endif
    {
        res = spiCom_AcquSync();
    }
    return res;
}


FuncResult_e spiCom_SyncSync(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    SYNC_PRINT("Sync for %u ICs\n", syncModeCfg.icCount);
#if (SYNC_TEST_FLOW != 1)
    uint16_t ind;
    uint16_t ic;
    if (syncModeCfg.icCount > 1u) {
        for (ind = 0u; ind < syncModeCfg.icCount; ind++) {
            ic = syncModeCfg.icCount - ind - 1;
            spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
            SYNC_PRINT("Sync IC %u\n", spiDriver_currentState.params[ic].icIndex);
            res = spiCom_Sync();
        }
    } else
#endif
    {
        res = spiCom_Sync();
    }
    return res;
}


ComStat_e spiCom_WaitSyncForReady(void)
{
    ComStat_e res = SPI_DRV_FUNC_RES_OK;
#if (SYNC_TEST_FLOW != 1)
    uint16_t ind;
    uint16_t ic;
    if (syncModeCfg.icCount > 1u) {
        for (ind = 0u; ind < syncModeCfg.icCount; ind++) {
            ic = syncModeCfg.icCount - ind - 1;
            spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
            res = spiCom_WaitForReady();
        }
    } else
#endif
    {
        res = spiCom_WaitForReady();
    }
    return res;
}


uint16_t spiDriver_GetIcIndexById(const uint16_t chip_id)
{
    uint16_t ic = 0u;
    while ((ic < syncModeCfg.icCount) && (spiDriver_currentState.params[ic].icIndex != chip_id)) {
        ic++;
    }
    if (ic >= syncModeCfg.icCount) {
        ic = MAX_IC_ID_NUMBER;
    }
    return ic;
}

#ifdef __cplusplus
}
#endif

/** @}*/

