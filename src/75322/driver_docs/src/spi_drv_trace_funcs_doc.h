/**
 * @file
 * @brief High-level SPI-driver functions documentation
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

#ifndef SPI_DRV_TRACE_FUNCS_DOC_H
#define SPI_DRV_TRACE_FUNCS_DOC_H

/**
    @ingroup spi_driver
    @addtogroup spi_trace

    SPI Getting Scene Functions
    ===========================

    The functions to configure, capture a single scene and manage its data are collected in this block.

    The normal flow of data capture should consider that the layer's configuration is always uploaded into IC.
    The SPI driver aligns it's internal buffered configuration from IC right before running the SCENE capture.
    But normally the scene and it's layers should be configured before it's run.

    Thus, the flow of capturing a single scene should be like provided in a picture:

    @dot
    digraph Get_Scene_Sequence {
        node [ shape=record, fontname=Helvetica, fontsize=10 ];
        start [ label="START" shape=box, style=filled, color=".2 .25 1.0", width=1.5]
        lconf [ label="Layers\nconfiguration" shape=box, style=filled, color=".6 .25 1.0", width=1.5, URL="@ref slayer"]
        lorder [ label="Layers order\nconfiguration" shape=box, style=filled, color=".6 .25 1.0", width=1.5, URL="@ref lorder"]
        lsource [ label="Light source\nconfiguration" shape=box, style=filled, color=".6 .25 1.0", width=1.5, URL="@ref spi_trace_light_src"]
        getscene [ label="Getting single\nscene" shape=box, style=filled, color=".6 .25 1.0", width=1.5, URL="@ref spi_trace_single"]
        proc [ label="Scene data\nprocessing" shape=box, style=filled, color=".6 .25 1.0", width=1.5, URL="@ref data_processing"]
        stop [ label="STOP" shape=box, style=filled, color=".2 .25 1.0", width=1.5]
        start -> lconf -> lorder -> lsource -> getscene -> proc -> stop
    }
    @enddot

    Potentially **Layers Configuration**, **Layers order configuration** and **Light source configuration** phases can skipped.
    For more information about phases refer to their description below [or by clicking boxes on a scheme].

    @addtogroup spi_trace_config

    @{

    @anchor slayer

    Scene layers configuration
    --------------------------

    Scene configuration describes the sequence of layers configuration and their order definition for the
    one single scene.

    Scene configuration assume the ::spiDriver_SetLayerConfig function execution for each layer involved in an
    ongoing scene aquisition. Thus, to configure NN layers to be captured in one scene, up to NN layers should be
    configured. It's up to ::LAYER_CONFIGS_N layer configurations may be used.

    @note   One scene may have NN layers, but some of them can be processed multiple times in scene's sequence.

    The following scheme shows 1-Layer configuration interaction:

    @startuml
        skinparam SequenceMessageAlign center
        skinparam handwritten false
        hide footbox

        title Setting layer configuration:

        box "Application" #LightBlue
        participant "SW" as app
        endbox

        box "Driver" #LightYellow
        participant "Driver" as drv
        endbox

        box "IC (device)" #LightYellow
        participant "SPI Slave(s)" as slave
        endbox

    group Layers configuration
 |||
        activate app

 |||
        app -> drv : Send layer's configuration N
        deactivate app
        activate drv
        note right: Application defines the configuration of layer N
 |||
        drv -> slave : Send layer's configuration to IC
        deactivate drv
        activate slave
        note right: Driver sets up the IC's multiple registers
 |||
        slave -> drv: Slave responds with assingment statuses
        deactivate slave
        activate drv
        drv -> app : Driver: responds with a sum of results
 |||
        deactivate drv
        activate app
 |||
        ...
    end
    @enduml

    The parameters of structure ::spiDriver_LayerConfig_t can be configured for each layer.

    @anchor lorder

    Layers order configuration
    --------------------------

    Layer order configuration sets the order of layers which will be gathered and processed in one scene. The
    sequence of layers may have more than one layer instance in the same scene. For instance, the sequence of
    5 layers like

        1 --> 4 --> 3 --> 0 --> 3

    is allowed. During the scene data capturing the IC cannot be stopped. Thus, the sequence should always be finished
    without an interruption. In case of such interruption, the driver cannot continue execution and the IC should be
    reset first.

    Layer order assume the ::spiDriver_SetLayers function execution for the whole scene configuration.
    Thus, to configure the sequence of NN layers to be captured in one scene, the order should have up-to
    ::LAYERS_ORDER_MAX items.

    @note   One scene may have NN layers, but some of them can be processed multiple times in scene's sequence.

    The following scheme shows the scene's sequence configuration interaction:

    @startuml
        skinparam SequenceMessageAlign center
        skinparam handwritten false
        hide footbox

        title Setting scene layers order:

        box "Application" #LightBlue
        participant "SW" as app
        endbox

        box "Driver" #LightYellow
        participant "Driver" as drv
        endbox

        box "IC (device)" #LightYellow
        participant "SPI Slave(s)" as slave
        endbox

    group Layers configuration
 |||
        activate app

 |||
        app -> drv : Send layers order and their amount (M). Specify processing mode if needed
        deactivate app
        activate drv
        note right: Application defines the orider of M layers in scene
 |||
        drv -> slave : Send layers order configuration to IC
        deactivate drv
        activate slave
        note right: Driver sets up the IC's multiple registers
 |||
        slave -> drv: Slave responds with assingment statuses
        deactivate slave
        activate drv
        drv -> app : Driver: responds with a sum of results
 |||
        deactivate drv
        activate app
 |||
        ...
    end
    @enduml

    @anchor data_processing

    Data processing
    ---------------

    After calling the function of single-scene aquisition the data is returned by array of structures
    ::spiDriver_ChipData_t. The memory for this array was allocated be SPI driver, and this buffer is valid till
    the end-of the next getting single trace operation is performed. During the next get single scene function called,
    the SPI driver will allocated the new buffer of it's start and will overwrite the current buffer's pointer only at
    the end of data capture. Additional releasing memory procedure is also performed. The following scheme describes
    the sequence:

    @dot
    digraph get_scene_buf_data_flow {
        node [ shape=record, fontname=Helvetica, fontsize=10 ];
        start [ label="START" shape=box, style=filled, color=".2 .25 1.0", width=1.5]
        bufalloc [ label="Buffer allocation" shape=box, style=filled, color=".6 .25 1.0", width=1.5]
        getlayers [ label="Reading layers" shape=box, style=filled, color=".6 .25 1.0", width=1.5]
        bufrepl [ label="Buffer pointer\nreplacement" shape=box, style=filled, color=".6 .25 1.0", width=1.5]
        freeoldbuf [ label="Old buffer release" shape=box, style=filled, color=".6 .25 1.0", width=1.5]
        stop [ label="STOP" shape=box, style=filled, color=".2 .25 1.0", width=1.5]
        start -> bufalloc -> getlayers -> bufrepl -> freeoldbuf -> stop
    }
    @enddot

    Thus, if processing is assured to be faster than the data aquisition and reading - these two processes may
    rUn simultaneously.

    Incoming data structure is defined by two global variables as well:

    - ::spiDriver_chipData - which represents the current pointer of the data captured;
    - ::spiDriver_chipDataSize - which represents the number of records (layers) grabbed in the last scene captured;

    The data items in a ::spiDriver_chipData array themselfs are the layer's output, returned by an IC. They have a
    format taken from the current IC configuration. Layers go in a sequence configured prior to the getting scene
    sequence call.

    Available data types of layer's records are described by ::ChipDataFormat_e type. Each layer creates an output
    according to it's mode (raw/trace mode OR echo mode) and its configuration.

    So, the type ::spiDriver_ChipData_t describes the layer's data output, while the ::spiDriver_ChipData_t.data should
    be parsed according the ::spiDriver_ChipData_t.dataFormat. An additional helper functions can be useful for that:

    - ::EchoParseAsFast - for Echo in FAST mode;
    - ::EchoParseAs9P - for Echo in 9P mode;
    - ::EchoParseAsShort - for Echo in SHORT mode;
    - ::EchoParseAsDetail - for Echo in DETAIL mode;

    @}

 */

#endif /* SPI_DRV_TRACE_FUNCS_DOC_H */

