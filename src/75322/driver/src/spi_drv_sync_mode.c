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
 * @ingroup spi_drv_multi_ic
 *
 * @{
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "spi_drv_common_types.h"
#include "spi_drv_api.h"
#include "spi_drv_sync_mode.h"
#include "spi_drv_trace.h"

/** Global synchronous mode configuration data, used in driver's API functions */
SyncModeCfg_t syncModeCfg = {
    .icCount = 0u,                  /**< Actual number of IC in a sequence */
};

/** Specific layer's parameters taken into account in syncronous mode */
typedef struct {
    uint16_t layer_index;       /**< layer's index used as a base configuration. Used to get an access to some layer's parameters */
    uint16_t averaging;         /**< Sample averaging which should match for the layer in each IC */
    uint16_t trigger_period;    /**< Trigger period. Should match for the layer in each IC */
    uint16_t sampling_mode;     /**< Sampling mode. Should match for the layer in each IC */
    uint16_t dark_frame_en;     /**< Dark frame used flag. Should match for the layer in each IC */
    uint16_t dark_averaging;    /**< Dark averaging. Should match for the layer in each IC if the dark frame is used */
    uint16_t lsm_config;        /**< LSM configuration. Should match for the layer in each IC */
} SyncLayerConfig_t;

/** Specific scene's parameters taken into account in syncronous mode */
typedef struct {
    SyncLayerConfig_t layer_cfg[LAYERS_ORDER_MAX];  /**< The of layer's configurations */
    uint16_t layer_count;                           /**< layer's count. Taken into account by per-IC independent state-machine */
    uint16_t sync_mode;                             /**< IC's sync_mode. Should match in each IC, shouldn't be NONE */
    uint16_t recharge_led_en;                       /**< Reacharge led flag. Should match for the layer in each IC */
    uint16_t slave_mode;                            /**< SLAVE or MASTER mode selection. Should be disabled only for the first IC (so the first IC is always master) */
} SyncSceneConfig_t;


extern volatile SpiDriver_State_t spiDriver_currentState;


FuncResult_e spiDriver_SyncModeInit(const SyncModeCfg_t* cfg)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    if (cfg == NULL) {
        fprintf(stderr, "Error: Syncronous mode configuration is NULL\n");
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
    } else {
        if (cfg->icCount == 0u) {
            /* init internal default configuration */
            syncModeCfg.icCount = 1u;
            spiDriver_currentState.params[0u].icIndex = 0u; /* Init the single IC config with the single IC id = 0 */
        } else {
            /* Intentionally skip the normal case */
        }
        syncModeCfg = *cfg;
    }
    return res;
}


FuncResult_e spiDriver_SyncPinWaitForReady(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t ic;
    for (ic = 0u; (ic < syncModeCfg.icCount) && (res == SPI_DRV_FUNC_RES_OK); ic++) {
        spiDriver_SpiSetDev(spiDriver_currentState.params[ic].icIndex);
        res = (spiDriver_PinWaitForReady() == CS_SUCCESS) ? SPI_DRV_FUNC_RES_OK : SPI_DRV_FUNC_RES_FAIL_COMM;
    }
    return res;
}


FuncResult_e spiDriver_SetSyncIcOrder(const uint16_t* const icIds, const uint16_t icCount)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    if ((icIds != NULL) || (icCount == 0u)) {
        if (icCount < MAX_IC_ID_NUMBER) {
            syncModeCfg.icCount = icCount;
            for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
                spiDriver_currentState.params[ic].icIndex = icIds[ic];
            }
        } else {
            fprintf(stderr, "Error: ICs number %u cannot be more than %u\n", icCount, MAX_IC_ID_NUMBER);
            res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
        }
    } else {
        fprintf(stderr, "Error: ICs order array is null\n");
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
    }
    return res;
}


/** Reads the IC's configuration
 * Perform IC configuration reading according to sync-mode significant settings. Gets these settings into the current internal buffer
 * @param[in] ic specifies the IC configurtion index
 * @param[in] sync_config specifies the pointer to ICs parameters to read into
 * @return the value of reading result
 */
static FuncResult_e ReadSyncConfig(const uint8_t ic, SyncSceneConfig_t* sync_config)
{
    FuncResult_e res;
    char param_str[256u];
    char bit_field_str[256u];
    uint8_t layer_name_offset;
    char* param_name;
    char* bit_field_name;
    uint16_t layer_index;
    uint32_t u32_buf;
    SYNC_PRINT("Read sync-mode configuration for ic %u\n", spiDriver_currentState.params[ic].icIndex);
    res = spiCom_SetDev(spiDriver_currentState.params[ic].icIndex);
    res |= spiDriver_GetByName("scene_layers_amount", &u32_buf, "");
    sync_config->layer_count = u32_buf;
    spiDriver_currentState.params[ic].sceneLayersAmount = u32_buf;
    res |= spiDriver_GetByName("scene_param", &u32_buf, "scene_sync_mode");
    sync_config->sync_mode = u32_buf;
    spiDriver_currentState.params[ic].sceneSyncMode = u32_buf;
    res |= spiDriver_GetByName("scene_param", &u32_buf, "scene_recharge_led_en");
    sync_config->recharge_led_en = u32_buf;
    res |= spiDriver_GetByName("hws_PORT_HWS_CTRL", &u32_buf, "hws_slave");
    sync_config->slave_mode = u32_buf;
    for (uint8_t layer = 0u; layer < sync_config->layer_count; layer++) {
        sprintf(param_str, "scene_layers_order_%u", layer);
        res |= spiDriver_GetByName(param_str, &u32_buf, "");
        layer_index = u32_buf;
        sprintf(param_str, "layer_%u_", layer_index);
        strcpy(bit_field_str, param_str);
        sync_config->layer_cfg[layer].layer_index = layer_index;
        layer_name_offset = strlen(param_str);
        param_name = param_str + layer_name_offset;
        bit_field_name = bit_field_str + layer_name_offset;
        sprintf(param_name, "averaging");
        res |= spiDriver_GetByName(param_str, &u32_buf, "");
        sync_config->layer_cfg[layer].averaging = u32_buf;
        sprintf(param_name, "dark_averaging");
        res |= spiDriver_GetByName(param_str, &u32_buf, "");
        sync_config->layer_cfg[layer].dark_averaging = u32_buf;
        sprintf(param_name, "trigger_period");
        res |= spiDriver_GetByName(param_str, &u32_buf, "");
        sync_config->layer_cfg[layer].trigger_period = u32_buf;
        sprintf(param_name, "sampling_port_sampling_mode");
        sprintf(bit_field_name, "sampling_mode");
        res |= spiDriver_GetByName(param_str, &u32_buf, bit_field_str);
        sync_config->layer_cfg[layer].sampling_mode = u32_buf;
        sprintf(param_name, "param");
        sprintf(bit_field_name, "dark_frame_en");
        res |= spiDriver_GetByName(param_str, &u32_buf, "");
        sync_config->layer_cfg[layer].dark_frame_en = u32_buf;
        sprintf(param_name, "scene_reserved_scene_w_%u", (layer / 2) + 2); /* scene_reserved_scene_w_2 is the very first lsm configuration. TODO: adjust this to stay compatible for xAA, xAB and further ROMs */
        res |= spiDriver_GetByName(param_name, &u32_buf, "");
        u32_buf = (u32_buf >> (8 * (layer & 1))) & 0xFFu; /* Detach the byte-size lsm-config for each layer configuration */
        sync_config->layer_cfg[layer].lsm_config = u32_buf;
    }
    return res;
}


/** This function checks the ICs configurations consistency to be ran in syncronous mode
 *  It checks the:
      - layer->averaging should match;
      - layer->trigger_period should match;
      - layer->sampling.mode should match;
      - layer->dark_frame_en should match;
      - scene->lsm_configs[layer_index].lsm_enable should match
      - scene->sync_mode should match;
      - scene->recharge_led_en should match;
      - the first IC should be configured as master. All others should be slaves;
 * In terms of layers flow the function checks each layers consistency in a sequence by IC's layers order. Thus, even
 *      if ICs scene have different amount of layers - their configuration should match on each sequence step.
 * @param[in] ref_config specifies the pointer to ICs parameters to compare
 * @param[in] sync_config specifies the pointer to ICs parameters to compare
 * @retval true the configuration is consistent
 * @retval false the configuration is not consistent
 */

static bool CompareSyncConfig(const SyncSceneConfig_t* const ref_config, const SyncSceneConfig_t* const sync_config)
{
    bool res = false;
    SYNC_PRINT("Compare sync-mode configuration\n");
#if 0
    if (ref_config->slave_mode == 1u) {
        fprintf(stderr, "Error in Sync config: the first IC in configuration should be in master mode\n");
    } else
#endif /* TODO: enable this right before merging into the master */
    if (sync_config->slave_mode == 0u) {
        fprintf(stderr, "Error in Sync config: all but first IC in configuration should be in slave mode\n");
    } else if (ref_config->recharge_led_en != sync_config->recharge_led_en) {
        fprintf(stderr, "Error in Sync config: Recharge Led Enable flag should be equal\n");
    } else if (ref_config->sync_mode != sync_config->sync_mode) {
        fprintf(stderr, "Error in Sync config: The `sync_mode` should be equal\n");
    } else {
        uint16_t max_layer_ind;
        uint16_t ref_ind;
        uint16_t sync_ind;
        uint16_t layer_ind = 0u;
        if (ref_config->layer_count > sync_config->layer_count) {
            max_layer_ind = ref_config->layer_count;
        } else {
            max_layer_ind = sync_config->layer_count;
        }
        res = true;
        while ((layer_ind < max_layer_ind) && res) {
            ref_ind = layer_ind % ref_config->layer_count;
            sync_ind = layer_ind % sync_config->layer_count;
            if (ref_config->layer_cfg[ref_ind].averaging != sync_config->layer_cfg[sync_ind].averaging) {
                fprintf(stderr,
                        "Error in Sync config: layer`s averaging should match for each sync step for all ICs\n");
                res = false;
            } else if (ref_config->layer_cfg[ref_ind].trigger_period !=
                       sync_config->layer_cfg[sync_ind].trigger_period) {
                fprintf(stderr,
                        "Error in Sync config: layer`s trigger period should match for each sync step for all ICs\n");
                res = false;
            } else if (ref_config->layer_cfg[ref_ind].sampling_mode != sync_config->layer_cfg[sync_ind].sampling_mode) {
                fprintf(stderr,
                        "Error in Sync config: layer`s sampling mode should match for each sync step for all ICs\n");
                res = false;
            } else if (ref_config->layer_cfg[ref_ind].dark_frame_en != sync_config->layer_cfg[sync_ind].dark_frame_en) {
                fprintf(stderr,
                        "Error in Sync config: layer`s dark frame enable flag should match for each sync step for all ICs\n");
                res = false;
            } else if (ref_config->layer_cfg[ref_ind].lsm_config != sync_config->layer_cfg[sync_ind].lsm_config) {
                fprintf(stderr,
                        "Error in Sync config: layer`s lsm_config should match for each sync step for all ICs\n");
                res = false;
            } else if ((ref_config->layer_cfg[ref_ind].dark_averaging !=
                        sync_config->layer_cfg[sync_ind].dark_averaging) &&
                       (ref_config->layer_cfg[ref_ind].dark_frame_en != 0u)) {
                fprintf(stderr,
                        "Error in Sync config: layer`s dark frame averaging should match for each sync step for all ICs\n");
                res = false;
            } else {
                /* Positive case. Just bypass */
            }
            layer_ind++;
        }
    }
    return res;
}

FuncResult_e spiDriver_CheckSyncConfig(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t ic;
    SyncSceneConfig_t ref_cfg; /* Reference configuration to be compared with */
    SyncSceneConfig_t cmp_cfg; /* The configuration to compare with the reference */

    SYNC_PRINT("Checking sync-mode configuration\n");
    if (syncModeCfg.icCount >= 1u) { /* For single IC it's everything ok always. There's nothing to check */
        /* Check the IDs consistency */
        for (ic = 0u; ic < syncModeCfg.icCount; ic++) {
            if (spiDriver_currentState.params[ic].icIndex >= MAX_IC_ID_NUMBER) {
                fprintf(stderr, "Error in Sync config: the IC ID[%u] is greater than the MAX: %u from %u\n",
                        ic, spiDriver_currentState.params[ic].icIndex, MAX_IC_ID_NUMBER);
                res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
                break;
            }
            if (ic == 0u) { /* Use the IC0 as a reference to compare */
                res |= ReadSyncConfig(ic, &ref_cfg);
                if (res != SPI_DRV_FUNC_RES_OK) {
                    fprintf(stderr,
                            "Error reading IC0 configuration: 0x%u\n", res);
                    break;
                }
            } else {
                res |= ReadSyncConfig(ic, &cmp_cfg);
                if (res != SPI_DRV_FUNC_RES_OK) {
                    fprintf(stderr,
                            "Error reading IC0 configuration: 0x%u\n", res);
                    break;
                }
                if (!CompareSyncConfig(&ref_cfg, &cmp_cfg)) {
                    fprintf(stderr,
                            "Error in Sync config: IC Configuration %u and %u are not compatible to run in syncronous mode\n",
                            0u,
                            ic);
                    res |= SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
                    break;
                }
            }
        }
    }
    return res;
}

/** @}*/

