/**
 * @file
 * @brief SPI driver for MLX75322 Continuous mode support
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
 * @defgroup spi_cont_mode SPI driver continuous mode component
 * @ingroup spi_driver
 *
 * @details provides the continuous mode of data gathering, on top of **spi_trace** layer. This
 *     component implements an asynchronous threads and allows to get the data from an IC in asyncronous event-based
 *     mode. It should be directly controlled from an application, assuming the data gathering flow will be run in a
 *     separate threads.
 *
 * @defgroup spi_cont_lib SPI driver continuous mode major thread
 * @ingroup spi_cont_mode
 *
 * @details provides the major continous mode thread, which drives the continuous mode process - receives the
 *      application's commands, sends the data requests and calls the application's callback function. This component
 *      also provides the application to initialize the continous mode, manage the memory structures for data buffers
 *      and make their safe replacements according the code flow.
 *
 */

#ifndef CONT_MODE_LIB_H
#define CONT_MODE_LIB_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "spi_drv_trace.h"

/** Continuous mode unique message queue identifier */
#define CONT_MODE_MSQ_KEY ((key_t)0x01CAFE10)

/* Use CON_MODE_DEBUG=1 option to add continuous mode debug output */
#ifndef CONT_MODE_DEBUG
#define CONT_MODE_DEBUG 0
#endif /* CONT_MODE_DEBUG */

#if (CONT_MODE_DEBUG == 1)
#define CONT_PRINT printf
#else
#define CONT_PRINT(...)
#endif

/** Specifies the buffer's size for holding the field's name in a structure */
#define CONT_MODE_MAX_PENDING 100

/** Continuous mode internal state-machine state */
typedef enum {
    CONT_MODE_EMPTY = 0,        /**< Empty command, recognized as something wrong */
    CONT_MODE_CTRL,             /**< Control command type. Used to change the mode */
    CONT_MODE_REQUEST_DATA,     /**< Request data command. */
    CONT_MODE_DATA_READY,       /**< Data ready command. Used to indicate the data is received and can be handled */
    CONT_MODE_FEEDBACK,         /**< The command or signal used as an additional flags flow */
} ContMessageType_e;

/** Callback function result type */
typedef enum {
    CB_RET_OK,                  /**< Callback is processed and needs more data */
    CB_RET_STOP,                /**< Callback wants to stop the data acquisition */
    CB_RET_EXIT                 /**< Callback want to exit the continuous mode completely */
} ContModeCbRet_t;

/** Continuous mode control commands and modes */
typedef enum {
    CONT_MODE_IDLE = 0,         /**< Idle state. Configured but not ran mode */
    CONT_MODE_WORK,             /**< Working mode. If pending reqs >= ::CONT_MODE_MAX_PENDING then we continuously work */
    CONT_MODE_STOP,             /**< The mode when the stop command has been received, waiting appropriate phase to stop */
    CONT_MODE_ERROR,            /**< Some error has occurred and was not handled yet. */
    CONT_MODE_EXIT,             /**< Exiting the continuous mode threads */
    CONT_MODE_NOT_INITED        /**< Initial state, when no configuration was done yet */
} ContModeCmd_e;

/** The message structure for threads interchange */
typedef struct {
    long mtype;             /**< Message type */
    ContModeCmd_e cmd;      /**< Message command */
} contModeInterface_t;

/** The size of the interchange message's payload, used for message sending in OS */
#define contModeInterface_size (sizeof(contModeInterface_t) - sizeof(long))

/** Callback function type definition.
 * The callback function receives the data structure of one scene received. This data is buffered and will be updated when
 * the callback will finish its execution
 * @param[in]   chipData        The pointer to a scene's data
 */
typedef ContModeCbRet_t (* cbFunc_t)(spiDriver_ChipData_t* chipData);

/** The Continuous mode configuration structure */
typedef struct {
    cbFunc_t callback;          /**< The callback function that should be called for data processing after all scene will be collected or layer collected if ContModeCfg_t::useAsyncSequence is set */
    spiDriver_LayerConfig_t* layerConfigurations; /**< Set of layer's configurations, that should be used.
                                                     Only raw/trace option is initialized from this structure, for the continuous mode */
    uint16_t layerConfigCount;  /**< The number of layers in the "layerConfigurations" array */
    bool useAsyncSequence;    /**< When enabled - the multi-IC mode uses the separated flows for the ICs and calls the ContModeCfg_t::callback function per each layer */
    uint16_t* layerOrder;       /**< Array of layer orders, to be used in a scene */
    uint16_t layerCount;        /**< Layers number in "layerOrder" array */
} ContModeCfg_t;

extern volatile int msqid;

/** Initiates the continuous mode by the information provided
 * The function sets up the continuous mode threads and sets up the IC registers to
 * run the continuous mode.
 * @param[in]       cfg layers and layers' order configuration
 */
void spiDriver_InitContinuousMode(const ContModeCfg_t* const cfg);

/** Runs the continuous mode threads
 * This function reads back the current layer's configuration (which impacts on the data type and data flow) and
 * executes the continuous mode. Once stopped continuous mode, it can be resumed by this function
 * @retval false returned when continuous mode cannot be ran right now. The state-machine should be in ::CONT_MODE_IDLE state
 * @retval true returned when continuous mode threads receive the messages to run
 */
bool spiDriver_RunContinuousMode(void);

/** Stop the continuous mode
 * Pauses the continuous mode execution, stopping the IC's data acquisition and gathering all pending information from it.
 * @note    the callback function can be called after this function, despite the mode change.
 * @retval false returned when continuous mode cannot be stopped.
 * @retval true returned when continuous mode was completely stopped.
 */
bool spiDriver_StopContinuousMode(void);

/** Exits from the continuous mode
 * This function frees the continuous mode threads and callbacks. Once exited, the continuous mode can run again with
 * ::spiDriver_InitContinuousMode function called first.
 * @retval false returned when continuous mode cannot be exited right now. The state-machine should be in ::CONT_MODE_IDLE state
 * @retval true returned when continuous mode threads receive the exit messages. Still needs to stop threads and exit from the mode.
 */
bool spiDriver_ExitContinuousMode(void);

#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* CONT_MODE_INIT_H */

