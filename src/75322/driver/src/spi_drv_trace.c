/**
 * @file
 * @brief High-level SPI-driver functions
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
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "static_assert.h"
#include "spi_drv_common_types.h"
#include "spi_drv_api.h"
#include "spi_drv_data.h"
#include "spi_drv_trace.h"
#include "spi_drv_com.h"
#include "cont_mode_lib.h"
#include "spi_drv_sync_com.h"

/* Internal types */

/* Global Variables */
extern ContModeCfg_t contModeCfg;

/** Default setLayer configuration used for spiDriver_SetLayerConfig() call */
const spiDriver_LayerConfig_t spiDriver_DefaultLayerConfig = {
    .layer_nth = 0u,
    .isTrace = true,
    .samplingMode = SAMPLING_MODE_DUAL3,
    .samplingSize = SAMPLING_SIZE_78,
    .nSamples = 0x100u,
    .skipSamples = 32u,
    .averaging = 0x200u,
    .gain = 5u,
    .echoThreshold = 30u,
    .continuousEnable = false
};

extern ComStat_e spiCom_WaitForReady(void);
extern void spiDriver_UpdateCurrentData(const spiDriver_ChipData_t* const spiDriver_chipDataTmp,
                                        const uint16_t spiDriver_chipDataSizeTmp);

volatile spiDriver_ChipData_t* spiDriver_chipData;
volatile uint16_t spiDriver_chipDataSize;
volatile SpiDriver_State_t spiDriver_currentState;

static cbLightFunc_t lightControlFunction = NULL;

/* Internal helper functions */
static uint16_t spiDriver_GetEchoSize(const EchoFormatSize_e echoFormat);
static uint16_t spiDriver_GetCurrentLayer(uint16_t layerIndex);
static FuncResult_e spiDriver_SendSensorStart(SpiDriver_Params_t* params);



/** Reads necessary variables */
static FuncResult_e spiDriver_GetParam(SpiDriver_Params_t* params)
{
    FuncResult_e res;
    char name[MAX_FLD_NAME];
    char fld_name[MAX_FLD_NAME];
    uint32_t tmp32;

    res = spiDriver_GetByName("scene_param", &params->sceneParam, NULL);

    res |= spiDriver_GetByName("scene_layers_amount", &params->sceneLayersAmount, NULL);
    TRACE_PRINT("IC layers: %u\n", params->sceneLayersAmount);
    res |= spiDriver_GetByName("scene_param", &params->sceneSyncMode, "scene_sync_mode");
    TRACE_PRINT("IC SYNC mode is : %u\n", params->sceneSyncMode);

    for (uint16_t layerInd = 0u; layerInd < params->sceneLayersAmount; layerInd++ ) {
        params->layers[layerInd].layerId = spiDriver_GetCurrentLayer(layerInd);
        sprintf(name, "layer_%u_param", params->layers[layerInd].layerId);
        sprintf(fld_name, "layer_%u_raw_mode_en", params->layers[layerInd].layerId);
        res |= spiDriver_GetByName(name, &tmp32, fld_name);
        params->layers[layerInd].isTrace = (tmp32 != 0);
        if (params->layers[layerInd].isTrace) {
            /* Get n samples for current layer */
            sprintf(name, "layer_%u_n_samples", params->layers[layerInd].layerId);
            res |= spiDriver_GetByName(name, &tmp32, NULL);
            TRACE_PRINT("Layer samples: %u\n", tmp32);
            params->layers[layerInd].nSamples = tmp32;
        } else {
            sprintf(name, "layer_%u_echo_format", params->layers[layerInd].layerId);
            res |= spiDriver_GetByName(name, &tmp32, NULL);
            params->layers[layerInd].format = (EchoFormatSize_e)tmp32;
            params->layers[layerInd].nSamples = spiDriver_GetEchoSize((EchoFormatSize_e)tmp32);
        }
        TRACE_PRINT("IC %u, layer:%u => format:%s, samples:%u\n",
                    params->icIndex,
                    params->layers[layerInd].layerId,
                    params->layers[layerInd].isTrace ? "RAW" : "ECHO",
                    params->layers[layerInd].nSamples);
    }
    return res;
}


static uint16_t spiDriver_GetEchoSize(const EchoFormatSize_e echoFormat)
{
    uint16_t size = 0u;
    switch (echoFormat) {
        case FMT_ECHO_FAST:
            size = sizeof(EchoData_t) / sizeof(uint16_t);
            break;

        case FMT_ECHO_9P:
            size = sizeof(EchoData_t) / sizeof(uint16_t);
            break;

        case FMT_ECHO_SHORT:
            size = (sizeof(EchoShortData_t) + sizeof(Metadata_t)) / sizeof(uint16_t);
            break;

        case FMT_ECHO_DETAIL:
            size = sizeof(EchoData_t) / sizeof(uint16_t);
            break;

        default:  /* we normally shouldn't get in here, so this is error. Leaving size=0 */
            break;
    }
    return size;
}

static uint16_t spiDriver_GetCurrentLayer(uint16_t layerIndex)
{
    FuncResult_e res;
    uint32_t currLayer;
    char name[MAX_FLD_NAME];
    sprintf(name, "scene_layers_order_%u", layerIndex);
    res = spiDriver_GetByName(name, &currLayer, NULL);
    if (res != SPI_DRV_FUNC_RES_OK) {
        currLayer = (uint16_t)SPI_DRV_ERR_VALUE;
        (void)res; /* TODO: report on error if occured */
    }
    return currLayer;
}

FuncResult_e spiDriver_SetLayers(uint8_t nLayer,
                                 const uint16_t* const layerOrder,
                                 const TraceCfgType_t isTrace,
                                 const ProcOrder_e* const procOrder,
                                 bool cont)
{
    FuncResult_e res;
    uint32_t traceModeValue;
    uint32_t contMode;
    char name[MAX_FLD_NAME];
    char fld_name[MAX_FLD_NAME];

    /* Continuous mode ? */
    res = SPI_DRV_FUNC_RES_OK;
    if (cont) {
        contMode = 1ul;
    } else {
        contMode = 0ul;
    }
    res |= spiDriver_SetSyncByName("param", contMode, "continuous_en");
    spiDriver_currentState.continuousMode = cont;

    if (nLayer > 0u) {
        /* Set number of layers */
        TRACE_PRINT("Scene's layers amount:%u\n", nLayer);
        res |= spiDriver_SetSyncByName("scene_layers_amount", (uint32_t)nLayer, NULL); /* TODO: layers should be managed for whole multi-layers' config */

        if (layerOrder != NULL) {
            /* Set layers order */
            TRACE_PRINT("Scene's layers sequence:");
            for (uint16_t layerIndex = 0; layerIndex < nLayer; layerIndex++) {
                sprintf(name, "scene_layers_order_%u", layerIndex);
                res |= spiDriver_SetSyncByName(name, (uint32_t)(layerOrder[layerIndex]), NULL);
                TRACE_PRINT("%u, ", layerOrder[layerIndex]);
                if (isTrace != SPI_DRV_CFG_OUT_NC) {
                    sprintf(name, "layer_%u_param", layerOrder[layerIndex]);
                    sprintf(fld_name, "layer_%u_raw_mode_en", layerOrder[layerIndex]);
                    if (procOrder == NULL) {
                        /* Set order for current layer */
                        switch (isTrace) {
                            case SPI_DRV_CFG_OUT_ECHO:
                                traceModeValue = 0u;
                                break;
                            case SPI_DRV_CFG_OUT_TRACE:
                                traceModeValue = 1u;
                                break;
                            default:
                                traceModeValue = 1u;
                                break;
                        }
                    } else {
                        if (procOrder[layerIndex] == PROC_ORDER_TRACE) {
                            traceModeValue = 1;
                        } else if (procOrder[layerIndex] == PROC_ORDER_ECHO) {
                            traceModeValue = 0;
                        } else {
                            traceModeValue = 0; /* The value should be assigned anyway */
                        }
                    }
                    res |= spiDriver_SetSyncByName(name, traceModeValue, fld_name);
                }
            }
            TRACE_PRINT("\n");
        }
    }
    return res;
}


static FuncResult_e spiDriver_SetGain(const spiDriver_LayerConfig_t* const layerConfiguration)
{
    FuncResult_e res;
    char name[MAX_FLD_NAME];
    uint16_t gainPattern = layerConfiguration->gain;
    /* Gains for all channels */
    if (layerConfiguration->gain < GAIN_MAX_VALUE) {
        gainPattern = gainPattern + (gainPattern << 4) + (gainPattern << 8) + (gainPattern << 12);
        for (uint8_t i = 0u; i < 2; i++) {
            for (uint8_t j = 0u; j < 4; j++) {
                sprintf(name, "layer_%u_gains_%u_%u", layerConfiguration->layer_nth, i, j);
                res = SPI_DRV_FUNC_RES_OK;
                res |= spiDriver_SetSyncByName(name, gainPattern, NULL);
            }
        }
    } else {
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
    }
    return res;
}

FuncResult_e spiDriver_SetOutputModeConfig(const spiDriver_LayerConfig_t* const layerConfiguration)
{
    FuncResult_e res;
    char name[MAX_FLD_NAME];
    char fld_name[MAX_FLD_NAME];
    uint32_t isTraceValue;

    sprintf(name, "layer_%u_param", layerConfiguration->layer_nth);
    sprintf(fld_name, "layer_%u_raw_mode_en", layerConfiguration->layer_nth);
    res = SPI_DRV_FUNC_RES_OK;
    if (syncModeCfg.icCount >= 2) {
        res |= spiCom_SetDev(layerConfiguration->ic_id);
    }
    if (layerConfiguration->isTrace) {
        isTraceValue = 1ul;
    } else {
        isTraceValue = 0ul;
    }
    res |= spiDriver_SetByName(name, isTraceValue, fld_name);

    return res;
}

FuncResult_e spiDriver_SetLayerConfig(const spiDriver_LayerConfig_t* const layerConfiguration)
{
    FuncResult_e res;
    char name[MAX_FLD_NAME];
    char fld_name[MAX_FLD_NAME];
    uint32_t contModeValue;

    /* Continuous mode */
    res = SPI_DRV_FUNC_RES_OK;
    if (layerConfiguration->continuousEnable) {
        contModeValue = 1ul;
    } else {
        contModeValue = 0ul;
    }
    res |= spiDriver_SetSyncByName("param", contModeValue, "continuous_en");

    if (layerConfiguration->samplingMode < SAMPLING_MODE_COUNT) {

        /* Set layer's raw mode */
        res |= spiDriver_SetOutputModeConfig(layerConfiguration);

        if (syncModeCfg.icCount >= 2) {
            res |= spiCom_SetDev(layerConfiguration->ic_id);
        }

        /* Sampling mode */
        sprintf(name, "layer_%u_sampling_port_sampling_mode", layerConfiguration->layer_nth);
        res |= spiDriver_SetByName(name, layerConfiguration->samplingMode, NULL);

        /* Sampling size */
        sprintf(name, "layer_%u_sampling_PORT_SAMP_CFG", layerConfiguration->layer_nth);
        sprintf(fld_name, "layer_%u_sampling_size", layerConfiguration->layer_nth);
        res |= spiDriver_SetByName(name, layerConfiguration->samplingSize, fld_name);

        /* Samples number */
        sprintf(name, "layer_%u_n_samples", layerConfiguration->layer_nth);
        res |= spiDriver_SetByName(name, layerConfiguration->nSamples, NULL);

        /* Averaging */
        sprintf(name, "layer_%u_averaging", layerConfiguration->layer_nth);
        res |= spiDriver_SetByName(name, layerConfiguration->averaging, NULL);

        res |= spiDriver_SetGain(layerConfiguration);
        /* Threshold */
        sprintf(name, "layer_%u_threshold", layerConfiguration->layer_nth);
        res |= spiDriver_SetByName(name, layerConfiguration->echoThreshold, NULL);

    } else {
        /* samplingMode should be within the range [0..6] */
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
    }
    return res;
}


static void spiDriver_AppendChipData(volatile SpiDriver_Params_t* params,
                                     spiDriver_ChipData_t** chipDataArrayP,
                                     uint16_t* chipDataArraySize,
                                     ChipDataFormat_e dataFormat,
                                     void* data,
                                     uint16_t layerInd,
                                     Metadata_t* metaData,
                                     FuncResult_e* status,
                                     const uint16_t icCount)
{
    uint16_t samples;
    uint8_t* data8bit = data;
    uint16_t dataSize;
    uint16_t dataIndex = *chipDataArraySize;
    spiDriver_ChipData_t* chipDataArray;

    *chipDataArrayP = realloc(*chipDataArrayP, sizeof(chipDataArray[0u]) * (dataIndex + icCount));
    chipDataArray = *chipDataArrayP;
    for (uint16_t ic = 0u; ic < icCount; ic++) {
        samples = params->layers[layerInd + ic].nSamples;
        if (dataFormat == CHIP_DATA_META_ONLY) {
            dataSize = 0;
            chipDataArray[dataIndex].data = NULL;
        } else {
            if (dataFormat == CHIP_DATA_TRACE) {
                dataSize = sizeof(spiDriver_TraceData_t) * N_CHANNELS;
            } else {
                dataSize = spiDriver_GetEchoSize((EchoFormatSize_e)dataFormat) * sizeof(uint16_t);
            }
            chipDataArray[dataIndex].data = malloc(dataSize);
        }
        chipDataArray[dataIndex].dataFormat = dataFormat;
        memcpy(chipDataArray[dataIndex].data, data8bit, dataSize);
        data8bit += dataSize;
        chipDataArray[dataIndex].metaData = malloc(sizeof(Metadata_t));
        memcpy(chipDataArray[dataIndex].metaData, metaData, sizeof(Metadata_t));
        metaData += 1;
        chipDataArray[dataIndex].samples = samples;
        chipDataArray[dataIndex].status = status[ic];
        chipDataArray[dataIndex].chip_id = params->icIndex;
        dataIndex++;
    }
    *chipDataArraySize = dataIndex;
}


void spiDriver_CleanChipData(spiDriver_ChipData_t* chipDataArray, uint16_t* chipDataArraySize)
{
    for (uint16_t ind = 0u; ind < *chipDataArraySize; ind++) {
        free(chipDataArray[ind].data);
        free(chipDataArray[ind].metaData);
    }
    *chipDataArraySize = 0u;
}


static void spiDriver_CombineTraces(volatile SpiDriver_Params_t* params,
                                    uint16_t* evenTraceData,
                                    uint16_t* oddTraceData,
                                    uint16_t* traceData,
                                    const uint16_t layerInd,
                                    const uint16_t icCount)
{
    uint32_t offs = 0u;
    uint16_t sizeToCopy;
    uint16_t nSamples;
    for (uint16_t ic = 0u; ic < icCount; ic++) {
        nSamples = params->layers[layerInd + ic].nSamples;
        sizeToCopy = nSamples * sizeof(uint16_t);
        for (uint16_t cnt = 0; cnt < (N_CHANNELS / 2); cnt++) {
            memcpy(&traceData[2 * offs], &evenTraceData[offs], sizeToCopy);
            /* Copy the odd traces, making the very first item be the latest in a sequence */
            memcpy(&traceData[(2 * offs) + nSamples],
                   &oddTraceData[(offs + nSamples) % ((N_CHANNELS / 2) * nSamples)],
                   sizeToCopy);
            offs += nSamples;
        }
    }
}


/** Runs the Sync process according the (frame)-phase used, and settings */
static FuncResult_e spiDriver_MakeSync(const uint8_t nth_part)
{
    volatile SpiDriver_Params_t* params;
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
        params = &spiDriver_currentState.params[syncModeCfg.icCount - 1u - ic];
        uint16_t layerInd = params->sceneCurrentLayer;
        if (params->contState != CONT_MODE_STATE_IDLE) {
            if (syncModeCfg.icCount >= 2) {
                res |= spiCom_SetDev(params->icIndex);
            }
            if ((layerInd != (params->sceneLayersAmount - 1u)) || /* Not for the last layer, or FOR continuous mode */
                (spiDriver_currentState.continuousMode)) {
                if ((params->sceneSyncMode == ACQU_SYNC_FRAME) ||    /* For per-frame sync [1] mode */
                    ((params->sceneSyncMode == ACQU_SYNC_LAYER) && (nth_part == 0u))) { /* OR For per-frame mode, for the first iteration only */
                    SYNC_PRINT("IC%u Make ACQU & SYNC before layer %u\n", params->icIndex, layerInd);
                    res |= spiDriver_SyncPinWaitForReady(); /* TODO: do we need to wait the others? */
                    if (syncModeCfg.icCount >= 2) {
                        res |= spiCom_SetDev(params->icIndex);
                    }
                    res |= spiCom_AcquSync();
                    if (lightControlFunction != NULL) {
                        spiCom_WaitSyncForReady(); /* TODO: do we need to wait the others? */
                        if (layerInd >= (params->sceneLayersAmount - 1)) {
                            lightControlFunction(params->layers[0].layerId);
                        } else {
                            lightControlFunction(params->layers[layerInd + 1].layerId);
                        }
                    }
                    if (syncModeCfg.icCount >= 2) {
                        res |= spiCom_SetDev(params->icIndex);
                    }
                    res |= spiCom_Sync();
                }
            }
        } else {
            SYNC_PRINT("Do not SYNC IC%u. It's out of order (mode %u)\n", params->icIndex, params->contState);
        }
    }
    return res;
}

/** Gets the layer data in trace(raw) mode
 *
 */
static FuncResult_e spiDriver_GetSingleTrace(const uint16_t icInd,
                                             volatile SpiDriver_Params_t* params,
                                             spiDriver_ChipData_t** spiDriver_chipDataTmp,
                                             uint16_t* spiDriver_chipDataSizeTmp)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    spiDriver_TraceData_t* traceData;
    spiDriver_TraceData_t* evenTraceData;
    spiDriver_TraceData_t* oddTraceData;
    Metadata_t* evenEchoMetadata;
    Metadata_t* oddEchoMetadata;
    FuncResult_e* oddStatus;
    FuncResult_e* evenStatus;
    uint16_t layerInd = params->sceneCurrentLayer;

    evenEchoMetadata = malloc(sizeof(Metadata_t) * (N_CHANNELS / 2u));
    oddEchoMetadata = malloc(sizeof(Metadata_t) * (N_CHANNELS / 2u));
    evenTraceData = malloc(sizeof(spiDriver_TraceData_t) * (N_CHANNELS / 2u));
    oddTraceData = malloc(sizeof(spiDriver_TraceData_t) * (N_CHANNELS / 2u));
    traceData = malloc(sizeof(spiDriver_TraceData_t) * N_CHANNELS);
    oddStatus = malloc((N_CHANNELS / 2u) * sizeof(FuncResult_e));
    evenStatus = malloc((N_CHANNELS / 2u) * sizeof(FuncResult_e));

    TRACE_PRINT("IC[%u] Layer samples: %u\n", icInd, params->layers[layerInd].nSamples);
    if (syncModeCfg.icCount >= 2) {
        SYNC_PRINT("Switch to IC%u\n", params->icIndex);
        res |= spiCom_SetDev(params->icIndex);
    }

#if (SYNC_TEST_FLOW == 1)
    SYNC_PRINT("Getting the trace[1 from 2] from IC %u\n", params->icIndex);
    evenStatus[params->icIndex] = SPI_DRV_FUNC_RES_OK;
#else
    /* Get trace of even channels */
    evenStatus[params->icIndex] = spiCom_GetRaw(params->layers[layerInd].nSamples + 8,
                                                (uint16_t*)evenTraceData,
                                                (uint16_t*)evenEchoMetadata);
#endif /* SYNC_TEST_FLOW */

    if (icInd == 0u) {
        spiDriver_MakeSync(0u);
    }

    /* Get trace of odd channels */
    if (syncModeCfg.icCount >= 2) {
        SYNC_PRINT("Switch to IC%u\n", params->icIndex);
        res |= spiCom_SetDev(params->icIndex);
    }

#if (SYNC_TEST_FLOW == 1)
    SYNC_PRINT("Getting the trace[2 from 2] from IC %u\n", params->icIndex);
    oddStatus[params->icIndex] = SPI_DRV_FUNC_RES_OK;
#else
    oddStatus[params->icIndex] = spiCom_GetRaw(params->layers[layerInd].nSamples + 8, (uint16_t*)oddTraceData,
                                               (uint16_t*)oddEchoMetadata);
#endif /* SYNC_TEST_FLOW */
    spiDriver_CombineTraces(params,
                            (uint16_t*)evenTraceData,
                            (uint16_t*)oddTraceData,
                            (uint16_t*)traceData,
                            layerInd,
                            1u);

    if (icInd == 0u) {
        spiDriver_MakeSync(1u);
    }

    spiDriver_AppendChipData(params,
                             spiDriver_chipDataTmp,
                             spiDriver_chipDataSizeTmp,
                             CHIP_DATA_TRACE,
                             traceData,
                             layerInd,
                             evenEchoMetadata,
                             evenStatus,
                             1u);
    spiDriver_AppendChipData(params,
                             spiDriver_chipDataTmp,
                             spiDriver_chipDataSizeTmp,
                             CHIP_DATA_META_ONLY,
                             NULL,
                             0u,
                             oddEchoMetadata,
                             oddStatus,
                             1u);

    /* free assigned memory buffers */
    free(evenEchoMetadata);
    free(oddEchoMetadata);
    free(evenTraceData);
    free(oddTraceData);
    free(traceData);
    free(evenStatus);
    free(oddStatus);
    return res;
}

/** Gets the single layer data in echo mode
 *
 */
static inline FuncResult_e spiDriver_GetSingleEcho(const uint16_t icInd,
                                                   volatile SpiDriver_Params_t* params,
                                                   spiDriver_ChipData_t** spiDriver_chipDataTmp,
                                                   uint16_t* spiDriver_chipDataSizeTmp)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    ChannelEchoAll_t* echoesData;
    Metadata_t* echoMetadata;
    FuncResult_e* echoStatus;
    uint16_t layerInd = params->sceneCurrentLayer;
    EchoFormatSize_e layerEchoFormat = params->layers[layerInd].format;

    /** Create buffers */
    echoMetadata = malloc(sizeof(Metadata_t) * N_CHANNELS);
    echoesData = malloc(sizeof(ChannelEchoAll_t));
    echoStatus = malloc(N_CHANNELS * sizeof(FuncResult_e));

    if (icInd == 0u) {
        spiDriver_MakeSync(0u);
        spiDriver_MakeSync(1u);
    }

    TRACE_PRINT("Requesting %u words\n", params->layers[layerInd].nSamples);
    if (syncModeCfg.icCount >= 2) {
        SYNC_PRINT("Switch to IC%u\n", params->icIndex);
        res |= spiCom_SetDev(params->icIndex);
    }
#if (SYNC_TEST_FLOW == 1)
    SYNC_PRINT("Getting the echo from IC %u\n", params->icIndex);
    echoStatus[params->icIndex] = SPI_DRV_FUNC_RES_OK;
#else
    echoStatus[params->icIndex] = spiCom_GetEcho(params->layers[layerInd].nSamples,
                                                 (uint16_t*)echoesData,
                                                 (uint16_t*)echoMetadata);
#endif /* SYNC_TEST_FLOW */
    spiDriver_AppendChipData(params,
                             spiDriver_chipDataTmp,
                             spiDriver_chipDataSizeTmp,
                             layerEchoFormat,
                             echoesData,
                             layerInd,
                             echoMetadata,
                             echoStatus,
                             1u);

    /* free assigned memory buffers */
    free(echoMetadata);
    free(echoesData);
    free(echoStatus);
    return res;
}

/** Performs the single "tick" of the read state-machine for the specified IC.
 * These ticks should be called in the order of ICs connected with SYNC wire. The latest IC in a sequence should
 * be in "master" mode, who will driver the syncronous mode of execution
 *
 */
static inline FuncResult_e spiDriver_MakeSingleTick(const uint16_t icIdx,
                                                    volatile SpiDriver_Params_t* params,
                                                    spiDriver_ChipData_t** spiDriver_chipDataTmp,
                                                    uint16_t* spiDriver_chipDataSizeTmp)
{
    uint16_t layerInd = params->sceneCurrentLayer;
    bool isTrace = params->layers[layerInd].isTrace;
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    if ((spiDriver_currentState.params[icIdx].contState == CONT_MODE_STATE_STARTED) ||
        (spiDriver_currentState.params[icIdx].contState == CONT_MODE_STATE_FINISHED)) {
        spiDriver_currentState.params[icIdx].contState = CONT_MODE_STATE_WORKING;
    }

    if (spiDriver_currentState.params[icIdx].contState == CONT_MODE_STATE_WORKING) {
        TRACE_PRINT("IC %u, current layer ID: %u, mode: %s\n",
                    params->icIndex,
                    params->layers[layerInd].layerId,
                    isTrace ? "TRACE" : "ECHO");

        if (isTrace) {
            res = spiDriver_GetSingleTrace(icIdx, params, spiDriver_chipDataTmp, spiDriver_chipDataSizeTmp);
        } else {
            res = spiDriver_GetSingleEcho(icIdx, params, spiDriver_chipDataTmp, spiDriver_chipDataSizeTmp);
        }
    }
    return res;
}

/** Gets the single scene for the single chip
 * This function uses the loop inside to make the single-chip getting data possible. It allocates the memory
 * for the buffer and fills-in received data into the buffers for all layers of a single IC */
FuncResult_e spiDriver_getSingleSceneAsync(const uint16_t icIdx,
                                           volatile SpiDriver_Params_t* params,
                                           spiDriver_ChipData_t** spiDriver_chipDataTmp,
                                           uint16_t* spiDriver_chipDataSizeTmp)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    if (params == NULL) { /* Use default global setting if the params are not defined explicitly */
        params = &spiDriver_currentState.params[0u]; /* TODO: Use corresponding IC state-machine */
    }
    uint16_t icCount = syncModeCfg.icCount;
    if (icCount < 2u) {
        icCount = 1u;
    }
    TRACE_PRINT("Read data of %u layers, continuous mode: %u, sync_mode: %u\n",
                params->sceneLayersAmount,
                spiDriver_currentState.continuousMode, params->sceneSyncMode);
    if ((params->contState == CONT_MODE_STATE_STARTED) || (params->contState == CONT_MODE_STATE_FINISHED)) {
        params->contState = CONT_MODE_STATE_WORKING;
    }
    params->sceneCurrentLayer = 0u;
    while ((params->sceneCurrentLayer < params->sceneLayersAmount) &&
           (res == SPI_DRV_FUNC_RES_OK)) {
        res = spiDriver_MakeSingleTick(icIdx,
                                       params,
                                       spiDriver_chipDataTmp,
                                       spiDriver_chipDataSizeTmp);
        params->sceneCurrentLayer++;
    }
    params->contState = CONT_MODE_STATE_FINISHED;
    return res;
}

/** Makes a single synchro step in a synchronous mode
 * This function uses per-IC loop inside to make the single tick (in layers) for all chips
 * Returns the layers from all ICs in a structure
 * @return
 */
FuncResult_e spiDriver_getSingleSyncStep(spiDriver_ChipData_t** spiDriver_chipDataTmp,
                                         uint16_t* spiDriver_chipDataSizeTmp)
{
    volatile SpiDriver_Params_t* params;
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;

    for (uint16_t ic = 0u; (ic < syncModeCfg.icCount) && (res == SPI_DRV_FUNC_RES_OK); ic++) { /* TODO: the sync should be triggered for (ic==0) only */
        params = &spiDriver_currentState.params[ic];
        if ((params->contState == CONT_MODE_STATE_STARTED) || (params->contState == CONT_MODE_STATE_FINISHED)) {
            params->contState = CONT_MODE_STATE_WORKING;
        }
        if (params->contState == CONT_MODE_STATE_WORKING) {
            TRACE_PRINT("Step: Read data of IC: %u, layer %u[%u], continuous mode: %u, sync_mode: %u\n",
                        params->icIndex, params->sceneCurrentLayer + 1u, params->sceneLayersAmount,
                        spiDriver_currentState.continuousMode, params->sceneSyncMode);
            res = spiDriver_MakeSingleTick(ic,
                                           params,
                                           spiDriver_chipDataTmp,
                                           spiDriver_chipDataSizeTmp);
            params->sceneCurrentLayer++;
            if (params->sceneCurrentLayer >= params->sceneLayersAmount) {
                params->sceneCurrentLayer = 0u;
                params->contState = CONT_MODE_STATE_FINISHED;
            }
        } else {
            TRACE_PRINT("Skipping data read of IC%u because it's in mode %u\n", params->icIndex, params->contState);
        }
    }
    return res;
}

/** Gets the single scene for the single chip
 * This function uses the loop inside to make the single-chip getting data possible. It allocates the memory
 * for the buffer and fills-in received data into the buffers for all layers of a single IC */
FuncResult_e spiDriver_getSingleScene(volatile SpiDriver_Params_t* params,
                                      spiDriver_ChipData_t** spiDriver_chipDataTmp,
                                      uint16_t* spiDriver_chipDataSizeTmp)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    if (params == NULL) { /* Use default global setting if the params are not defined explicitly */
        params = &spiDriver_currentState.params[0u]; /* TODO: Use corresponding IC state-machine */
    }

    spiDriver_TraceData_t* traceData;
    ChannelEchoAll_t* echoesData;
    Metadata_t* echoMetadata;

    Metadata_t* evenEchoMetadata;
    Metadata_t* oddEchoMetadata;
    spiDriver_TraceData_t* evenTraceData;
    spiDriver_TraceData_t* oddTraceData;
    FuncResult_e* oddStatus;
    FuncResult_e* evenStatus;
    FuncResult_e* echoStatus;

    uint16_t icCount = syncModeCfg.icCount;
    if (icCount < 2u) {
        icCount = 1u;
    }
    /** Create buffers */
    evenEchoMetadata = malloc(sizeof(Metadata_t) * (N_CHANNELS / 2u) * icCount);
    oddEchoMetadata = malloc(sizeof(Metadata_t) * (N_CHANNELS / 2u) * icCount);
    echoMetadata = malloc(sizeof(Metadata_t) * N_CHANNELS * icCount);
    evenTraceData = malloc(sizeof(spiDriver_TraceData_t) * (N_CHANNELS / 2u) * icCount);
    oddTraceData = malloc(sizeof(spiDriver_TraceData_t) * (N_CHANNELS / 2u) * icCount);
    traceData = malloc(sizeof(spiDriver_TraceData_t) * N_CHANNELS * icCount);
    echoesData = malloc(sizeof(ChannelEchoAll_t) * icCount);
    oddStatus = malloc((N_CHANNELS / 2u) * icCount * sizeof(FuncResult_e));
    evenStatus = malloc((N_CHANNELS / 2u) * icCount * sizeof(FuncResult_e));
    echoStatus = malloc(N_CHANNELS * icCount * sizeof(FuncResult_e));

    TRACE_PRINT("Read data of %u layers, continuous mode: %u, sync_mode: %u\n",
                params->sceneLayersAmount,
                spiDriver_currentState.continuousMode, params->sceneSyncMode);
    for (uint16_t ic = 0u; ic < icCount; ic++) {
        if ((spiDriver_currentState.params[ic].contState == CONT_MODE_STATE_STARTED) ||
            (spiDriver_currentState.params[ic].contState == CONT_MODE_STATE_FINISHED)) {
            spiDriver_currentState.params[ic].contState = CONT_MODE_STATE_WORKING;
        }
    }
    params->sceneCurrentLayer = 0u;
    while (params->sceneCurrentLayer < params->sceneLayersAmount) {
        bool layerMode = params->layers[params->sceneCurrentLayer].isTrace;
        TRACE_PRINT("Current layer: %u\n", params->layers[params->sceneCurrentLayer].layerId);
        TRACE_PRINT("Layer mode: %s\n", layerMode ? "TRACE" : "ECHO");
        if (layerMode) {
            TRACE_PRINT("Layer samples: %u\n", params->layers[params->sceneCurrentLayer].nSamples);
            /* Get trace of even channels */
            for (uint16_t ic = 0u; ic < icCount; ic++) {
                if (syncModeCfg.icCount >= 2) {
                    res |= spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
                }
                evenStatus[ic] = spiCom_GetRaw(params->layers[params->sceneCurrentLayer + ic].nSamples + 8,
                                               (uint16_t*)&evenTraceData[ic],
                                               (uint16_t*)&evenEchoMetadata[ic]);
            }
        }

        if ((params->sceneCurrentLayer != (params->sceneLayersAmount - icCount)) ||
            (spiDriver_currentState.continuousMode)) {
            if ((params->sceneSyncMode == 1u) ||
                (params->sceneSyncMode == 0u)) {
                res |= spiDriver_SyncPinWaitForReady();
                res |= spiCom_AcquSyncSync();
                if (lightControlFunction != NULL) {
                    spiCom_WaitSyncForReady();
                    if (params->sceneCurrentLayer >= (params->sceneLayersAmount - icCount)) {
                        lightControlFunction(params->layers[0].layerId);
                    } else {
                        lightControlFunction(params->layers[params->sceneCurrentLayer + 1].layerId);
                    }
                }
                res |= spiCom_SyncSync();
            }
        }

        if (layerMode) {
            /* Get trace of odd channels */
            for (uint16_t ic = 0u; ic < icCount; ic++) {
                if (syncModeCfg.icCount >= 2) {
                    res |= spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
                }
                oddStatus[ic] |= spiCom_GetRaw(params->layers[params->sceneCurrentLayer + ic].nSamples + 8,
                                               (uint16_t*)&oddTraceData[ic],
                                               (uint16_t*)&oddEchoMetadata[ic]);
            }
            spiDriver_CombineTraces(params,
                                    (uint16_t*)evenTraceData,
                                    (uint16_t*)oddTraceData,
                                    (uint16_t*)traceData,
                                    params->sceneCurrentLayer,
                                    icCount);
        }

        if (((params->sceneCurrentLayer != (params->sceneLayersAmount - icCount)) ||
             (spiDriver_currentState.continuousMode)) &&
            (params->sceneSyncMode == 1u)) {
            res |= spiDriver_SyncPinWaitForReady();
            res |= spiCom_AcquSyncSync();
            res |= spiCom_SyncSync();
        }

        if (!layerMode) {

            EchoFormatSize_e layerEchoFormat = params->layers[params->sceneCurrentLayer].format;
            for (uint16_t ic = 0u; ic < icCount; ic++) {
                TRACE_PRINT("Requesting %u words\n", params->layers[params->sceneCurrentLayer + ic].nSamples);
                if (syncModeCfg.icCount >= 2) {
                    res |= spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
                }
                echoStatus[ic] |=
                    spiCom_GetEcho(params->layers[params->sceneCurrentLayer + ic].nSamples,
                                   (uint16_t*)&echoesData[ic],
                                   (uint16_t*)&echoMetadata[ic]);
            }
            spiDriver_AppendChipData(params,
                                     spiDriver_chipDataTmp,
                                     spiDriver_chipDataSizeTmp,
                                     layerEchoFormat,
                                     echoesData,
                                     params->sceneCurrentLayer,
                                     echoMetadata,
                                     echoStatus,
                                     icCount);
        } else {
            spiDriver_AppendChipData(params,
                                     spiDriver_chipDataTmp,
                                     spiDriver_chipDataSizeTmp,
                                     CHIP_DATA_TRACE,
                                     traceData,
                                     params->sceneCurrentLayer,
                                     evenEchoMetadata,
                                     evenStatus,
                                     icCount);
            spiDriver_AppendChipData(params,
                                     spiDriver_chipDataTmp,
                                     spiDriver_chipDataSizeTmp,
                                     CHIP_DATA_META_ONLY,
                                     NULL,
                                     0u,
                                     oddEchoMetadata,
                                     oddStatus,
                                     icCount);
        }
        params->sceneCurrentLayer += icCount;
    }
    for (uint16_t ic = 0u; ic < icCount; ic++) {
        spiDriver_currentState.params[ic].contState = CONT_MODE_STATE_FINISHED;
    }

    /* free assigned memory buffers */
    free(evenEchoMetadata);
    free(oddEchoMetadata);
    free(echoMetadata);
    free(evenTraceData);
    free(oddTraceData);
    free(traceData);
    free(echoesData);
    free(echoStatus);
    free(evenStatus);
    free(oddStatus);
    return res;
}


static FuncResult_e spiDriver_GetScene(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    volatile SpiDriver_Params_t* params = &spiDriver_currentState.params[0u];
    spiDriver_ChipData_t* spiDriver_chipDataTmp;
    uint16_t spiDriver_chipDataSizeTmp;

    spiDriver_chipDataSizeTmp = 0u;
    spiDriver_chipDataTmp = NULL;

    res |= spiDriver_SendSensorStart(NULL);

    if (contModeCfg.useAsyncSequence) {
        res |= spiDriver_getSingleSceneAsync(0u, params, &spiDriver_chipDataTmp, &spiDriver_chipDataSizeTmp);
    } else {
        res |= spiDriver_getSingleScene(params, &spiDriver_chipDataTmp, &spiDriver_chipDataSizeTmp);
    }

    res |= spiCom_SensorSyncStop();

    for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
        spiDriver_currentState.params[ic].contState = CONT_MODE_STATE_IDLE;
    }

    /* Replace old shared data with the new created */
    spiDriver_UpdateCurrentData(spiDriver_chipDataTmp, spiDriver_chipDataSizeTmp);

    return res;
}


/* Shared functions */

FuncResult_e spiDriver_GetTrace(const uint16_t* const layerOrder,
                                const uint16_t layerCount,
                                spiDriver_ChipData_t** chipDataResult,
                                uint16_t* chipDataSizeResult)
{
    FuncResult_e res;
    for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
        spiDriver_currentState.params[ic].contState = CONT_MODE_STATE_IDLE;
    }
    spiDriver_currentState.continuousMode = false;
    res = spiDriver_SetLayers(layerCount, layerOrder, SPI_DRV_CFG_OUT_TRACE, NULL, false);
    res |= spiDriver_GetScene();
    *chipDataResult = (spiDriver_ChipData_t*)spiDriver_chipData;
    *chipDataSizeResult = spiDriver_chipDataSize;
    return res;
}


FuncResult_e spiDriver_GetEcho(const uint16_t* const layerOrder,
                               const uint16_t layerCount,
                               spiDriver_ChipData_t** chipDataResult,
                               uint16_t* chipDataSizeResult)
{
    FuncResult_e res;
    for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
        spiDriver_currentState.params[ic].contState = CONT_MODE_STATE_IDLE;
    }
    spiDriver_currentState.continuousMode = false;
    res = spiDriver_SetLayers(layerCount, layerOrder, SPI_DRV_CFG_OUT_ECHO, NULL, false);
    res |= spiDriver_GetScene();
    *chipDataResult = (spiDriver_ChipData_t*)spiDriver_chipData;
    *chipDataSizeResult = spiDriver_chipDataSize;
    return res;
}


FuncResult_e spiDriver_GetMixed(const uint16_t* const layerOrder,
                                const uint16_t layerCount,
                                const ProcOrder_e* const procOrder,
                                spiDriver_ChipData_t** chipDataResult,
                                uint16_t* chipDataSizeResult)
{
    FuncResult_e res;
    for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
        spiDriver_currentState.params[ic].contState = CONT_MODE_STATE_IDLE;
    }
    spiDriver_currentState.continuousMode = false;
    res = spiDriver_SetLayers(layerCount, layerOrder, SPI_DRV_CFG_OUT_NC, procOrder, false);
    res |= spiDriver_GetScene();
    *chipDataResult = (spiDriver_ChipData_t*)spiDriver_chipData;
    *chipDataSizeResult = spiDriver_chipDataSize;
    return res;
}


static FuncResult_e spiDriver_SendSensorStart(SpiDriver_Params_t* params)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    if (params == NULL) {
        params = (SpiDriver_Params_t*)&spiDriver_currentState.params[0u];
        res |= spiDriver_GetParam(params);
    }
    res |= spiCom_SensorSyncStart();
    if ((params->sceneSyncMode == ACQU_SYNC_FRAME) || (params->sceneSyncMode == ACQU_SYNC_LAYER)) {
        res |= spiCom_AcquSyncSync();
        if (lightControlFunction != NULL) {
            spiCom_WaitSyncForReady();
            lightControlFunction(params->layers[0].layerId);
        }
        res |= spiCom_SyncSync();
    }
    if (params->sceneSyncMode == ACQU_SYNC_FRAME) {
        res |= spiCom_AcquSyncSync();
        res |= spiCom_SyncSync();
    }
    if (res == SPI_DRV_FUNC_RES_OK) {
        for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
            spiDriver_currentState.params[ic].contState = CONT_MODE_STATE_STARTED;
        }
    }
    return res;
}



/** Executes the Continuous Mode
 * @param[in]   layerConfigurations array of layers configurations. If Configs are not provided, they
 *                                    will not be configured.
 * @param[in]   layerConfigCount      items count in `layerConfigurations` array. If its Count is zero,
 *                                    the layer's configuration is not set by this function
 * @param[in]   layerOrder      array of layers to read
 * @param[in]   layerCount      items count in `layerOrder` array
 * @return      result of an operation
 */
FuncResult_e spiDriver_StartContinuousMode(const spiDriver_LayerConfig_t* const layerConfigurations,
                                           const uint16_t layerConfigCount,
                                           const uint16_t* const layerOrder,
                                           const uint16_t layerCount)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    for (uint16_t layerInd = 0; layerInd < layerConfigCount; layerInd++) {
        res = spiDriver_SetOutputModeConfig(&layerConfigurations[layerInd]);
        if (res != SPI_DRV_FUNC_RES_OK) {
            TRACE_PRINT("Error setting layers params\n");
            break;
        }
    }

    if (syncModeCfg.icCount >= 1u) {
        for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
#if (SYNC_TEST_FLOW != 1)
            spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
#endif
            if (!contModeCfg.useAsyncSequence) {
                res |= spiDriver_SetLayers(layerCount, layerOrder, SPI_DRV_CFG_OUT_NC, NULL, true);
            }
            if (res == SPI_DRV_FUNC_RES_OK) {
                TRACE_PRINT("IC%u getting params...\n", spiDriver_currentState.params[ic].icIndex);
                res |= spiDriver_GetParam((SpiDriver_Params_t*)&spiDriver_currentState.params[ic]);
            }
        }
        /* Assume we're in continuous mode. Check? */
        spiDriver_currentState.continuousMode = true;
        res |= spiDriver_SendSensorStart((SpiDriver_Params_t*)&spiDriver_currentState.params[0u]); /* Pass the first ic as parameter to avoid params reading */
    } else {
        res |= spiDriver_SetLayers(layerCount, layerOrder, SPI_DRV_CFG_OUT_NC, NULL, true);
        if (res == SPI_DRV_FUNC_RES_OK) {
            TRACE_PRINT("Sensor getting params...\n");
            res |= spiDriver_GetParam((SpiDriver_Params_t*)&spiDriver_currentState.params[0u]);
            TRACE_PRINT("Sensor sending start...\n");
            res |= spiDriver_SendSensorStart(NULL);
        }
    }
    return res;
}


FuncResult_e spiDriver_InitContinuousModeInt(void)
{
    for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
        spiDriver_currentState.params[ic].contState = CONT_MODE_STATE_IDLE;
    }
    return SPI_DRV_FUNC_RES_OK;
}


FuncResult_e spiDriver_StopContinuousModeInt(const uint16_t icIdx)
{
    FuncResult_e res;
    if (spiDriver_currentState.params[icIdx].contState == CONT_MODE_STATE_FINISHED) {
        uint16_t prev_dev = spiDriver_SpiGetDev();
        res = SPI_DRV_FUNC_RES_OK;
        spiCom_SetDev(spiDriver_currentState.params[icIdx].icIndex);
        res |= spiCom_SensorStop();
        spiCom_SetDev(prev_dev);
        if (spiDriver_currentState.params[icIdx].sceneLayersAmount > 1) {
            spiDriver_currentState.continuousMode = false; // switch to single mode to finish the sequence after STOP command
        }
    } else {
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
    }
    return res;
}


__attribute__((weak))
void spiDriver_UpdateCurrentData(const spiDriver_ChipData_t* const spiDriver_chipDataTmp,
                                 const uint16_t spiDriver_chipDataSizeTmp)
{
    spiDriver_ChipData_t* spiDriver_chipDataOld = (spiDriver_ChipData_t*)spiDriver_chipData;
    uint16_t spiDriver_chipDataSizeOld = spiDriver_chipDataSize;
    spiDriver_chipData = (spiDriver_ChipData_t*)spiDriver_chipDataTmp;
    spiDriver_chipDataSize = spiDriver_chipDataSizeTmp;
    spiDriver_CleanChipData(spiDriver_chipDataOld, &spiDriver_chipDataSizeOld);
}


void spiDriver_AssignLightControl(const cbLightFunc_t lightFunction)
{
    lightControlFunction = lightFunction;
}

FuncResult_e spiDriver_ReadLayerConfig(const uint16_t icIdx,
                                       const uint16_t layerIdx,
                                       spiDriver_LayerConfig_t* const layerCfg)
{
    FuncResult_e res;
    char name[MAX_FLD_NAME];
    char fld_name[MAX_FLD_NAME];
    uint32_t tmp32;
    uint16_t layer_id;

    layerCfg->ic_id = spiDriver_currentState.params[icIdx].icIndex;
    spiCom_SetDev(layerCfg->ic_id);

    sprintf(name, "scene_layers_order_%u", layerIdx);
    res = spiDriver_GetByName(name, &tmp32, NULL);
    layerCfg->layer_nth = tmp32;
    layer_id = tmp32;

    sprintf(name, "layer_%u_param", layer_id);
    sprintf(fld_name, "layer_%u_raw_mode_en", layer_id);
    res |= spiDriver_GetByName(name, &tmp32, fld_name);
    layerCfg->isTrace = (tmp32 != 0u);

    sprintf(name, "layer_%u_sampling_port_sampling_mode", layer_id);
    res |= spiDriver_GetByName(name, &tmp32, NULL);
    layerCfg->samplingMode = tmp32;

    sprintf(name, "layer_%u_sampling_PORT_SAMP_CFG", layer_id);
    sprintf(fld_name, "layer_%u_sampling_size", layer_id);
    res |= spiDriver_GetByName(name, &tmp32, fld_name);
    layerCfg->samplingSize = tmp32;

    sprintf(name, "layer_%u_n_samples", layer_id);
    res |= spiDriver_GetByName(name, &tmp32, NULL);
    layerCfg->nSamples = tmp32;

    sprintf(name, "layer_%u_skip_samples", layer_id);
    res |= spiDriver_GetByName(name, &tmp32, NULL);
    layerCfg->skipSamples = tmp32;

    sprintf(name, "layer_%u_averaging", layer_id);
    res |= spiDriver_GetByName(name, &tmp32, NULL);
    layerCfg->averaging = tmp32;

    sprintf(name, "layer_%u_gains_0_0", layer_id);
    res |= spiDriver_GetByName(name, &tmp32, NULL);
    layerCfg->gain = tmp32;

    sprintf(name, "layer_%u_threshold", layer_id);
    res |= spiDriver_GetByName(name, &tmp32, NULL);
    layerCfg->echoThreshold = tmp32;

    sprintf(name, "param");
    sprintf(fld_name, "continuous_en");
    res |= spiDriver_GetByName(name, &tmp32, fld_name);
    layerCfg->continuousEnable = (tmp32 != 0u);

    return res;
}

FuncResult_e spiDriver_ReadSceneConfig(spiDriver_LayerConfig_t** layerConfigurations, uint16_t* layerConfigCount)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint32_t layers_amount[MAX_IC_ID_NUMBER];
    uint32_t layers_amount_max = 0u;
    uint32_t layers_amount_sum = 0u;
    uint16_t idx;
    /* Read amount of layers to read from IC */
    for (uint16_t ic = 0u; (ic < syncModeCfg.icCount) && (res == SPI_DRV_FUNC_RES_OK); ic++) {
        spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
        res = spiDriver_GetByName("scene_layers_amount", &layers_amount[ic], NULL);
        if (layers_amount_max < layers_amount[ic]) {
            layers_amount_max = layers_amount[ic];
        }
        layers_amount_sum += layers_amount[ic];
    }

    *layerConfigurations = realloc(*layerConfigurations, layers_amount_sum * sizeof(spiDriver_LayerConfig_t));
    idx = 0u;
    /* Read the layer's configuration as they appear in multiIC configuration */
    for (uint16_t layer = 0u; layer < layers_amount_max; layer++) {
        for (uint16_t ic = 0u; (ic < syncModeCfg.icCount) && (res == SPI_DRV_FUNC_RES_OK); ic++) {
            if (layer < layers_amount[ic]) {
                res = spiDriver_ReadLayerConfig(ic, layer, &((*layerConfigurations)[idx++]));
            }
        }
    }
    *layerConfigCount = layers_amount_sum;

    return res;
}

