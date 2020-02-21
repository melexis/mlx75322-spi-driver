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
 * @defgroup spi_trace SPI driver Configure and Get Scene component
 * @ingroup spi_driver
 *
 * @details
 *
 * Combines the read/write function calls in a SPI protocol data flow for selected mode of
 * operation. This component provides SPI protocol's layer implementation and allows to configure and read the
 * **SCENE** according the expected format. It also provides the functions and number of settings for continous
 * mode of data flow. This component can be used directly from application, to configure and get the data from IC,
 * in a single mode.
 *
 */

#ifndef SPI_DRV_TRACE_H
#define SPI_DRV_TRACE_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "static_assert.h"
#include "spi_drv_common_types.h"
#include "spi_drv_sync_com.h"

/**
 * @ingroup spi_trace
 * @defgroup spi_trace_types SPI protocol types and definitions
 *
 * @details
 *
 * This group of settings and types maps IC's protocol definitions and sizes onto output data types used in
 * SPI driver. It helps to manage the data handling for both - application and driver instances.
 * @{
 */

#define LAYERS_ORDER_MAX 32u        /**< Maximum amount of "layer configs" that may be executed during one scene */
#define LAYER_CONFIGS_N 16u         /**< Maximum amount of layer configs */
#define N_CHANNELS 32u              /**< Total number of channels per layer */

/** Specifies the number of channels in trace data for */
/* TODO: Refactor to remove these settings */
#define ECHO_NUM_CHANNEL ((uint16_t)N_CHANNELS - 2u)
/** Specifies the number of objects in echo data */
#define ECHO_NUM_OBJS ((uint16_t)4u)

/* --- */
#define MAX_ECHOES_PER_CHANNEL ECHO_NUM_OBJS    /**< Maximum amount of objects per channel */
#define MAX_ACTIVE_CHANNELS ((uint16_t)N_CHANNELS - 2u)       /**< Maximum amount of active channels in the system */
#define MAX_SAMPLES_N 312u             /**< Maximum number of samples in one channel */
#define MAX_LAYER_SAMPLES (9984u) /**< (MAX_SAMPLES_N * N_CHANNELS) explicitly defined as calculated value */

#ifndef TRACE_DEBUG
#define TRACE_DEBUG 0
#endif /* TRACE_DEBUG */

#if (TRACE_DEBUG == 1)
#define TRACE_PRINT printf
#else
#define TRACE_PRINT(...)
#endif

#ifndef SYNC_TEST_FLOW
#define SYNC_TEST_FLOW 0
#endif /* SYNC_TEST_FLOW */

/** Echo format type */
typedef enum {
    FMT_ECHO_FAST = 0u,     /**< FAST mode format */
    FMT_ECHO_9P = 1u,       /**< 9 points mode format */
    FMT_ECHO_SHORT = 2u,    /**< Short mode format */
    FMT_ECHO_DETAIL = 3u    /**< Detail mode format */
} EchoFormatSize_e;

/** Chip data format type */
typedef enum {
    CHIP_DATA_FAST = FMT_ECHO_FAST,         /**< FAST mode data format */
    CHIP_DATA_9P = FMT_ECHO_9P,             /**< 9 points mode data format */
    CHIP_DATA_SHORT = FMT_ECHO_SHORT,       /**< Short mode data format */
    CHIP_DATA_DETAIL = FMT_ECHO_DETAIL,     /**< Detail mode data format */
    CHIP_DATA_TRACE,                        /**< Trace data format */
    CHIP_DATA_META_ONLY,                    /**< Meta-data format */
} ChipDataFormat_e;

/** Processing order selection */
typedef enum {
    PROC_ORDER_NONE = 0u,                   /**< No processing */
    PROC_ORDER_TRACE,                       /**< Trace processing */
    PROC_ORDER_ECHO,                        /**< Echo processing */
} ProcOrder_e;


/** Common to some Echo Formats flags */
typedef union {
    uint16_t all_flags;                         /**< All flags. */
    struct {
        uint16_t valid                  : 1;    /**< Object is valid */
        uint16_t low_amplitude          : 1;    /**< Low amplitude */
        uint16_t issue_width            : 1;    /**< Issue with width */
        uint16_t saturated              : 1;    /**< Saturation indicator */
        uint16_t merge_suspicion        : 1;    /**< Merge suspicion */
        uint16_t xtalk                  : 1;    /**< Cross Talk suspicion */
        uint16_t undershoot             : 1;    /**< Undershoot detected */
        uint16_t : 9;                           /**< [not used] */
    };
    struct {
        uint16_t /* valid            */ : 1;    /**< [not used] */
        uint16_t check_amplitude        : 1;    /**< Check amplitude */
        uint16_t check_width            : 1;    /**< Check width */
        uint16_t check_saturation       : 1;    /**< Check on saturation */
        uint16_t check_merge_suspicion  : 1;    /**< Check merge suspicions */
        uint16_t check_xtalk            : 1;    /**< Check cross talk suspicions */
        uint16_t check_undershoot       : 1;    /**< Check on undershoots */
        uint16_t led_p_comp_en          : 1;    /**< Distance compensation by LED power enable/disable */
        uint16_t dist_off_comp_en       : 1;    /**< Distance compensation by distance offset enable/disable */
        uint16_t temp_comp_en           : 1;    /**< Distance compensation by temperature enable/disable */
        uint16_t : 6;                           /**< [not used] */
    };
} EchoFlags;
ASSERT(sizeof(EchoFlags) == 2u);

/** Echo Format 0. FMT_ECHO_FAST */
typedef struct {
    uint16_t minLi;  /**< Left point's index (dec) */
    uint16_t minL;   /**< Left point's amplitude */
    uint16_t maxi;   /**< Maximum point's index (dec) */
    uint16_t max;    /**< Maximum point's amplitude */
    uint16_t minRi;  /**< Right point's index (dec) */
    uint16_t minR;   /**< Right point's amplitude  */
    uint16_t maxSLi; /**< Rising edge inflection point's index (dec) */
    uint16_t maxSL;  /**< Rising edge inflection point's amplitude */
    uint16_t maxSRi; /**< Falling edge inflection point's index (dec) */
    uint16_t maxSR;  /**< Falling edge inflection point's amplitude */
} EchoFastDataItem_t;

/** Echo Format 1. FMT_ECHO_9P */

typedef struct {
    uint16_t index;     /**< Object's left (first) point */
    uint16_t data[9u];  /**< Nine consecutive amplitudes since left (starting) point */
} Echo9PDataItem_t;


typedef struct {
    uint16_t distance;
    uint16_t amplitude;
    uint16_t flags;
} EchoShortDataItem_t;


/** Echo Format 3. FMT_ECHO_DETAIL_1 */
typedef struct {
    union {
        uint16_t minLi;     /**< Left point (dec)  */
        uint16_t start_i;   /**< Left point (dec)  */
    };
    union {
        uint16_t minL;      /**< Left point's amplitude = baseline  (AmpFormat) */
        uint16_t baseline;  /**< Left point's amplitude = baseline  (AmpFormat) */
    };
    union {
        uint16_t maxi;      /**< Maximum point (DistFormat) */
        uint16_t peak;      /**< Maximum point (DistFormat) */
    };
    union {
        uint16_t max;       /**< Maximum point's amplitude */
        uint16_t amplitude; /**< Maximum point's amplitude */
    };
    union {
        uint16_t minRi;     /**< Right point (distance, dec) */
        uint16_t stop_i;    /**< Right point (distance, dec) */
    };
    union {
        uint16_t minR;      /**< Right point's amplitude (AmpFormat)) */
        uint16_t baseline_fall; /**< Right point's amplitude (AmpFormat)) */
    };
    union {
        uint16_t maxSLi;    /**< Rising edge inflection point (DistFormat) */
        uint16_t inflection; /**< Rising edge inflection point (DistFormat) */
    };
    union {
        uint16_t maxSRi;    /**< Falling edge inflection point (DistFormat) */
        uint16_t infl_fall; /**< Falling edge inflection point (DistFormat) */
    };
    uint16_t distance;      /**< Distance to object according to DISTANCE_METHOD (DistFormat) */
    EchoFlags flags; /**< Flags */
} EchoDetailDataItem_t;

typedef EchoFastDataItem_t EchoFastDataObject_t[ECHO_NUM_OBJS];
typedef Echo9PDataItem_t Echo9PDataObject_t[ECHO_NUM_OBJS];
typedef EchoShortDataItem_t EchoShortDataObject_t[ECHO_NUM_OBJS];
typedef EchoDetailDataItem_t EchoDetailDataObject_t[ECHO_NUM_OBJS];

typedef EchoFastDataObject_t EchoFastData_t[ECHO_NUM_CHANNEL];
typedef Echo9PDataObject_t Echo9PData_t[ECHO_NUM_CHANNEL];
typedef EchoShortDataObject_t EchoShortData_t[ECHO_NUM_CHANNEL];
typedef EchoDetailDataObject_t EchoDetailData_t[ECHO_NUM_CHANNEL];

/**
 * Echoes array[4] for a single channel
 *
 * When allocating memory, use ChannelEcho,
 * when accessing according to format, use concrete format type.
 */
typedef union {
    EchoFastDataObject_t format_fast;         /**< FMT_ECHO_FAST */
    Echo9PDataObject_t format_9p;             /**< FMT_ECHO_9P */
    EchoShortDataObject_t format_short;       /**< FMT_ECHO_SHORT */
    EchoDetailDataObject_t format_detail_1;   /**< FMT_ECHO_DETAIL_1 */
} ChannelEcho_t;
ASSERT(sizeof(ChannelEcho_t) == 80u);

typedef union {
    EchoFastData_t format_fast;         /**< FMT_ECHO_FAST */
    Echo9PData_t format_9p;             /**< FMT_ECHO_9P */
    EchoShortData_t format_short;       /**< FMT_ECHO_SHORT */
    EchoDetailData_t format_detail_1;   /**< FMT_ECHO_DETAIL_1 */
} ChannelEchoAll_t;
ASSERT(sizeof(ChannelEchoAll_t) == (80u * ECHO_NUM_CHANNEL));

/** Raw-specific metadata */
typedef struct {
    uint16_t reserved_meta_raw_w[4];     /**< Reserved for future ideas and requests */
} MetadataRaw_t;

/** Echo-specific metadata */
typedef struct {
    uint16_t reserved_meta_echo_w[2];    /**< Reserved for future ideas and requests */
    uint8_t reserved_meta_echo_b;        /**< Reserved for future ideas and requests */
    uint8_t n_objects;                   /**< Number of objects found */
    uint16_t max_amplitude;              /**< Maximum amplitude found */
} MetadataEcho_t;

/** Metadata */
typedef struct {
    union {
        MetadataRaw_t r;
        MetadataEcho_t e;
    } u;
    uint16_t layer_counter; /**< Incremental layer counter */
    uint16_t noise_level;   /**< Noise level (Q8.8) */
    uint16_t led_power;     /**< Illumination power record */
    uint8_t channel;        /**< Channel record */
    uint8_t layer;          /**< Layer record */
} Metadata_t;

/** Size of MetaData structure in words */
#define METADATA_SIZE (sizeof(Metadata_t) / 2u)
ASSERT(METADATA_SIZE == 8u);

/** Echo structure for a single Layer */
typedef struct {
    Metadata_t meta;                           /**< Metadata */
    ChannelEchoAll_t data;                     /**< All echoes */
} EchoData_t;
ASSERT(sizeof(EchoData_t) == 2416u);

/** Raw Data structure for a single Layer */
typedef struct {
    Metadata_t meta;                  /**< Metadata */
    uint16_t data[MAX_SAMPLES_N];     /**< Raw Data */
} RawData_t;
ASSERT(sizeof(RawData_t) == 640u);


typedef uint16_t spiDriver_TraceData_t[MAX_SAMPLES_N + 2];
typedef float spiDriver_OutTraceData_t[sizeof(spiDriver_TraceData_t) / sizeof(uint16_t)];
typedef uint16_t spiDriver_EchoData_t[sizeof(EchoDetailDataObject_t)];

/** Desired data type used to access the data in spiDriver_ChipData_t */
typedef union {
    uint16_t trace[MAX_LAYER_SAMPLES];  /**< data treated as a set of traces in order [N_CHANNELS][n_samples]
                                           @warning the real data size is N_samples * N_CHANNELS. We use hardcoded value since the CFFI [for Python]
                                           is not able to work with defines "MAX_SAMPLES_N * N_CHANNELS" */
    ChannelEchoAll_t echo;           /**< data treated as echo */
} spiDriver_Data_t;

/** The Chip data structure. Holds the data records, meta-data records, and statuses in array. The data has selected format. */
typedef struct {
    spiDriver_Data_t* data;             /**< The data gathered. In echo mode its an array of objects per each channel.
                                             In case of trace mode, it's an array of data[channel][samples]. */
    uint16_t samples;                   /**< Data amount in the structure. In case of Echo represents a size of CHANS * OBJS + 8 words of metadata.
                                             In case of trace they represent amount of samples for each channel.*/
    Metadata_t* metaData;               /**< The meta-data gathered */
    ChipDataFormat_e dataFormat;        /**< Data format for the data */
    FuncResult_e status;                /**< The status returned */
    uint16_t chip_id;                   /**< The Сhip ID of the layer data provided */
} spiDriver_ChipData_t;



/** Maximum gain value, to check the constraints in configuration */
#define GAIN_MAX_VALUE 8u
/** Maximum sampling mode value, to check the constraints in configuration */
#define SAMPLING_MODE_COUNT 7u

/** Sampling mode */
typedef enum {
    SAMPLING_MODE_DUAL0 = 0u,   /**< Dual sampling mode 0 */
    SAMPLING_MODE_DUAL1 = 1u,   /**< Dual sampling mode 1 */
    SAMPLING_MODE_DUAL2 = 2u,   /**< Dual sampling mode 2 */
    SAMPLING_MODE_DUAL3 = 3u    /**< Dual sampling mode 3 */
} spiDriver_SamplingMode_e;

/** Input parameter for assigning the type of layer's data */
typedef enum TraceCfgOutType_e {
    SPI_DRV_CFG_OUT_TRACE = 0,  /**< Used when config sets the output in raw mode */
    SPI_DRV_CFG_OUT_ECHO,       /**< Used when config sets the output in echo mode */
    SPI_DRV_CFG_OUT_NC,         /**< Used when config should bypass the output mode setting, leaving the current config */
} TraceCfgType_t;


/** Sampling size */
typedef enum {
    SAMPLING_SIZE_78 = 0u,  /**< 78 samples */
    SAMPLING_SIZE_52 = 1u,  /**< 52 samples */
    SAMPLING_SIZE_26 = 2u,  /**< 26 samples */
} spiDriver_SamplingSize_e;


/** Specifies the layer's configuration of data aquisition */
typedef struct {
    uint16_t layer_nth;                     /**< Layer's index */
    uint16_t ic_id;                         /**< IC id for multi-IC synchronous mode of operation.
                                                Not used if the IC's number ::SyncModeCfg_t.icCount less than 2 */
    bool isTrace;                           /**< Default = false */
    spiDriver_SamplingMode_e samplingMode;  /**< Default = ::SAMPLING_MODE_DUAL3 */
    spiDriver_SamplingSize_e samplingSize;  /**< Default = ::SAMPLING_SIZE_78 */
    uint16_t nSamples;                      /**< Default = 304 */
    uint16_t skipSamples;                   /**< Default = 0 */
    uint16_t averaging;                     /**< Default = 64 */
    uint8_t gain;                           /**< Default = 5 */
    uint16_t echoThreshold;                 /**< Default = 65 */
    bool continuousEnable;                  /**< Default = false */
} spiDriver_LayerConfig_t;

/** Used to hold all information about the current scene's layers to read them */
typedef struct {
    uint16_t icId;                          /**< IC ID for this layer. This value is copied from the input data configuration */
    uint16_t layerId;                       /**< layer ID */
    bool isTrace;                           /**< Default = true */
    uint16_t nSamples;                      /**< Default = 256 */
    EchoFormatSize_e format;
} spiDriver_LayerCurrentConfig_t;

typedef enum {
    CONT_MODE_STATE_IDLE = 0,
    CONT_MODE_STATE_STARTED,
    CONT_MODE_STATE_WORKING,
    CONT_MODE_STATE_FINISHED,
} contModeState_e;

/** The structure used to get and hold the scene's parameters for the device */
typedef struct {
    uint16_t icIndex; /** TODO: it's not an index but ID */
    uint32_t sceneParam;
    uint32_t sceneLayersAmount;
    uint32_t sceneSyncMode;
    spiDriver_LayerCurrentConfig_t layers[LAYERS_ORDER_MAX];
    uint16_t sceneCurrentLayer;
    contModeState_e contState;
} SpiDriver_Params_t;

typedef struct {
    SpiDriver_Params_t params[MAX_IC_ID_NUMBER];
    bool continuousMode;
} SpiDriver_State_t;

/** @}*/

/**
 * @ingroup spi_trace
 * @defgroup spi_trace_data Data handling structures and functions
 *
 * @details
 *
 * This group allows to manage the scene's data in one scope. It allows to split scene's data in layers, perform
 * desired typecasts, manage data buffers
 *
 * @{
 */

/** Chip-data information set, updated "at-once", in high-level functions (see below)
 * This pointer is updated after each high-level function call (like spiDriver_GetTrace(), spiDriver_GetEcho(), spiDriver_GetMixed() ).
 * This is done to make the data bufferred meaning a way of FIFO'ing it. During the high-level function execution the elder data is still available,
 * and the pointer spiDriver_chipData is replaced at-once, when the data is read.
 * @note this pointer is automatically allocated within the high-level function, and should be freed at the end (if needed).
 */
extern volatile spiDriver_ChipData_t* spiDriver_chipData;

/** Holds the size of a data array, stored in `spiDriver_chipData` structure */
extern volatile uint16_t spiDriver_chipDataSize;

/** @}*/


/**
 * @ingroup spi_trace
 * @addtogroup spi_trace_data
 * @{
 */

/** Free the chip-data defined by pointer
 * @param[in]   chipDataArray   not used data pointer
 * @param[in]   chipDataArraySize   data size golded by the pointer
 */
extern void spiDriver_CleanChipData(spiDriver_ChipData_t* chipDataArray, uint16_t* chipDataArraySize);

/** Treates the input data as EchoFastData_t structure (by its cast)
 * @param[in]   echoData     pointer to data
 * @return      pointer of data as EchoFastData_t structure
 */
static inline EchoFastData_t* EchoParseAsFast(const void* echoData)
{
    return (EchoFastData_t*)echoData;
}


/** Treates the input data as Echo9PData_t structure (by its cast)
 * @param[in]   echoData     pointer to data
 * @return      pointer of data as Echo9PData_t structure
 */
static inline Echo9PData_t* EchoParseAs9P(const void* echoData)
{
    return (Echo9PData_t*)echoData;
}


/** Treates the input data as EchoParseAsShort structure (by its cast)
 * @param[in]   echoData     pointer to data
 * @return      pointer of data as EchoParseAsShort structure
 */
static inline EchoShortData_t* EchoParseAsShort(const void* echoData)
{
    return (EchoShortData_t*)echoData;
}


/** Treates the input data as EchoParseAsDetail structure (by its cast)
 * @param[in]   echoData     pointer to data
 * @return      pointer of data as EchoParseAsDetail structure
 */
static inline EchoDetailData_t* EchoParseAsDetail(const void* echoData)
{
    return (EchoDetailData_t*)echoData;
}

/** @}*/


/**
 * @ingroup spi_trace
 * @defgroup spi_trace_config Scene configuration
 *
 * @details
 *
 * This group of functions and helpers allow to configure and set-up the scene's layers and their processing order
 *
 * @{
 */

/** Default configuration for spiDriver_SetLayerConfig() function */
extern const spiDriver_LayerConfig_t spiDriver_DefaultLayerConfig;


/** Configures layers order in a scene
 * @param[in]   nLayer      Number of layers in a sequence
 * @param[in]   layerOrder  Array of layers' indexes to be processed sequencially
 * @param[in]   isTrace     specifies the processing type selected for all layers. Mode assignment may be skipped.
 * @param[in]   procOrder   specifies the processing type for each layer (in a sequence) independently. This array
 *                          should be in size of nLayer
 * @param[in]   cont        continuous mode flag to be set
 *
 * @return      result of an operation. See ::FuncResult_e for details
 */
FuncResult_e spiDriver_SetLayers(uint8_t nLayer,
                                 const uint16_t* const layerOrder,
                                 const TraceCfgType_t isTrace,
                                 const ProcOrder_e* const procOrder,
                                 bool cont);


/** Configures the layer's output mode - echo or trace
 * @param[in]   layerConfiguration      set of parameters required for layers configuration (only *isTrace* field is considered)
 * @return      result of an operation. See ::FuncResult_e for details
 */
FuncResult_e spiDriver_SetOutputModeConfig(const spiDriver_LayerConfig_t* const layerConfiguration);


/** Configures the layer according desired mode and settings
 * @param[in]   layerConfiguration      set of parameters required for layers configuration
 * @return      result of an operation. See ::FuncResult_e for details
 */
FuncResult_e spiDriver_SetLayerConfig(const spiDriver_LayerConfig_t* const layerConfiguration);


/** Reads the layer's configuration from certain IC in order they will be captured
 *
 */
FuncResult_e spiDriver_ReadLayerConfig(const uint16_t icIdx,
                                       const uint16_t layerIdx,
                                       spiDriver_LayerConfig_t* const layerCfg);

/** Read the scene's configuration for single-/multi-IC configuration
 * This function 'refreshes' the "layerConfigurations" array with an amout of layers which will be read from the ICs configured.
 * It allows to read the configuration "as is" without assigning it with the driver initialization parameters.
 * @param[out] layerConfigurations      The new (reallocated) array of layers, per IC.
 * @param[out] layerConfigCount         Amount of configuration read from ICs
 */
FuncResult_e spiDriver_ReadSceneConfig(spiDriver_LayerConfig_t** layerConfigurations, uint16_t* layerConfigCount);

/** @}*/


/**
 * @ingroup spi_trace
 * @defgroup spi_trace_single Single Scene capture functions
 *
 * @details
 *
 * This group of functions allows to configure and capture a single scene of layers in desired processing mode.
 *
 * @{
 */

/** Reads Traces Only
 *
 * This function selects the RAW mode for all layers in their current configuration. It executes the data capture and
 * reads the result in blocking mode.
 * Function allocates a buffer for an output structure. Each call re-assigns the pointer of an output structure. And
 * after the latest function call, the pointer should be freed by standard c-function `free()`.
 *
 * @param[in]   layerOrder      array of layers to read
 * @param[in]   layerCount      items count in `layerOrder` array
 * @param[out]  chipDataResult  a pointer to an external pointer which will receive the data from scene
 * @param[out]  chipDataSizeResult  a pointer to an external value which will receive the data amount from scene
 * @return      result of an operation
 */
FuncResult_e spiDriver_GetTrace(const uint16_t* const layerOrder,
                                const uint16_t layerCount,
                                spiDriver_ChipData_t** chipDataResult,
                                uint16_t* chipDataSizeResult);


/** Reads Echoes Only
 *
 * This function selects the ECHO mode for all layers in their current configuration. It executes the data capture and
 * reads the result in blocking mode.
 * Function allocates a buffer for an output structure. Each call re-assigns the pointer of an output structure. And
 * after the latest function call, the pointer should be freed by standard c-function `free()`.
 *
 * @param[in]   layerOrder      array of layers to read
 * @param[in]   layerCount      items count in `layerOrder` array
 * @param[out]   chipDataResult  a pointer to an external pointer which will receive the data from scene
 * @param[out]   chipDataSizeResult  a pointer to an external value which will receive the data amount from scene
 * @return      result of an operation. See ::FuncResult_e for details
 */
FuncResult_e spiDriver_GetEcho(const uint16_t* const layerOrder,
                               const uint16_t layerCount,
                               spiDriver_ChipData_t** chipDataResult,
                               uint16_t* chipDataSizeResult);


/** Reads Traces and Echoes at once
 *
 * This function does not affect on the mode for layers in their current configuration. It executes the data capture and
 * reads the result in blocking mode.
 * Function allocates a buffer for an output structure. Each call re-assigns the pointer of an output structure. And
 * after the latest function call, the pointer should be freed by standard c-function `free()`.
 *
 * @param[in]   layerOrder      array of layers to read
 * @param[in]   layerCount      items count in `layerOrder` array
 * @param[in]   procOrder       array of processing types, according to layerOrder array provided
 * @param[out]   chipDataResult  a pointer to an external pointer which will receive the data from scene.
 * @param[out]   chipDataSizeResult  a pointer to an external value which will receive the data amount from scene.
 * @return      result of an operation. See ::FuncResult_e for details
 */
FuncResult_e spiDriver_GetMixed(const uint16_t* const layerOrder,
                                const uint16_t layerCount,
                                const ProcOrder_e* const procOrder,
                                spiDriver_ChipData_t** chipDataResult,
                                uint16_t* chipDataSizeResult);

/** @}*/

/**
 * @ingroup spi_trace
 * @defgroup spi_trace_light_src Light source control
 *
 * @details
 *
 * This part of SPI driver allows to configure the light's source control. By default the light control is not used.
 *
 * @{
 */

/** Callback function type for the light source layer's control. An input parameter specifies the layer's index
 * for which the light source should be driven. */
typedef void (* cbLightFunc_t)(const uint16_t layer);


/** Assigns the callback function for light-source control
 *
 * The callback function assigned by this function should drive the external light sources, by the layer indexes,
 * provided as an input parameter. The SPI-driver calls this callback function being synchronized with the data
 * capturing sequence. Thus, the light source is always linked to certain layer of scene sequence.
 *
 * @param[in]   lightFunction   the function's address to be called for light control for the next layer. Use
 *                              NULL to disable the light source control.
 * @warning     Note that the callback function will be called in any synchronous mode except SYNC_MODE_NONE
 *              (aka synchonisation mode "2"). Use modes SYNC_MODE_LAYER or SYNC_MODE_FRAME to get the led callback
 *              work.
 */
void spiDriver_AssignLightControl(const cbLightFunc_t lightFunction);

/** @}*/

/** @}*/



#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* SPI_DRV_TRACE_H */

