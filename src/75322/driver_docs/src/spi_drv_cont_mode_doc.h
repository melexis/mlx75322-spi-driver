/**
 * @file
 * @brief SPI driver Continuous mode documentation
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

#ifndef SPI_DRV_CONT_MODE_DOC_H
#define SPI_DRV_CONT_MODE_DOC_H

/**
    @ingroup spi_driver
    @addtogroup spi_cont_mode

    Continuous mode of operation
    ============================

    Continuous mode of operation provides the best IC's and SPI driver efficiency, and allows to get the data for IC(s)
    with the best performance. During the continuous mode of operation the IC triggers the data acquisitions
    automatically, only when its buffers are available to receive, process and transfer the new data portion.

    The execution flow
    ------------------

    In continuous mode the data has the following flow:

    @dot
    digraph Continuous_mode_flow {
        node [ shape=record, fontname=Helvetica, fontsize=10 ];
        start [ label="START" shape=box, style=filled, color=".2 .25 1.0", width=1.5]
        ic_capt [ label="IC layer capture" shape=box, style=filled, color=".6 .25 1.0", width=1.5, URL="@ref slayer"]
        hst_read [ label="Reading the data" shape=box, style=filled, color=".7 .9 1.0", width=1.5, URL="@ref slayer"]
        app_cb [ label="Application\nprocessing" shape=box, style=filled, color=".9 1.0 0.8", width=1.5, URL="@ref slayer"]
        stop [ label="STOP" shape=box, style=filled, color=".2 .25 1.0", width=1.5]
        start -> ic_capt -> hst_read -> app_cb -> ic_capt
        app_cb -> stop [ style=dashed ]
    }
    @enddot

    - IC is capturing, processing and preparing the one layer of scene. It also starts the new data acquisition and waits
        the buffer to be read by master.
    - The host reads the buffer from an IC and stores it in the local buffer;
    - right after the data data arrives, the driver starts writing the new data portion and calls the application's
        callback function which should handle the new data portion.
    - after the application will finish the data processing, the driver will wait for the new data portion and will
        run the data acquisition and data processing in a loop.

    For this flow the SPI driver creates two additional threads:

    - **data trigger thread** - the thread to control getting data by the low-level routines.
    - **continuous mode thread** - the thread to drive the continuous mode process and deliver the data to an
        application. This thread calls an application's callback function to process incoming data.

    Starting the continuous mode
    ----------------------------

    Continuous mode of data acquisition starts from the scene configuration and setting the continuous mode bit, to
    allow IC start its data capture automatically. After the configuration the driver sends commands "chip start"
    which triggers the sequence.

    Technically the process, within the SPI driver follows the algorithm in two phases:

 **Initialization phase**

    - Configure the scene;
    - Start the continuous mode lib tread:
        - init message queue;
        - wait for commands;
    - Init the trigger thread;
        - initial configuration and wait for command;

    So, after the initialization phase everything's prepared to run the sequence;

 **Run phase**

    This phase is triggered by command ::spiDriver_RunContinuousMode. The SPI driver sends the command ::SENSOR_START to IC,
    starting the sequence and sends the command ::CONT_MODE_WORK to continuous mode thread. The thread switches it's mode
    to wait for any of the following events:

    - some control command has been sent;
    - the ::CONT_MODE_DATA_READY message have been received;

    The thread also sends a request message to trigger_data thread to start the data capture.

    After these operations the IC and SPI driver goes in looped mode of execution;

    Stopping the continuous mode
    ----------------------------

    The cancellation of the Continuous mode is implemented in non-blocking mode to make a proper stop of continuous mode sequence.

    Thus, there are several rules that should be assured during the continuous mode stop:

    - the whole scene should be passed. This means the latest data received should be the latest layer's data in a scene.
    - The IC should have sufficient number of data acquisition requests **AFTER** sending it the ::CONT_MODE_STOP command. This number
        of requests should cover all layers in the scene.

    Hence, to stop the continuous mode by calling the function ::spiDriver_StopContinuousMode the working system does the following:

    - The continuous mode thread receives the message to stop the mode. It also estimates amount of pending "steps" to get pending
        data from an IC;
    - The trigger thread finishes the scene's data capture and send the current scene's data to the continuous mode thread;
    - When the cont-mode thread receives the data, it:

        - calls the application's callback function;
        - sends the ::SENSOR_STOP command to an IC;
        - requests the data from the IC, for the lass time (because of IC's internal data sequence);
    - After receiving the last data the library calls the application's callback with the last scene data;
    - If there's only one layer in a scene, the driver sends one more dummy data request and gets the scene's (with 1 layer only) data.
        This data is just dropped and not callback functions will be triggered.
    - The continuous mode is switching to ::CONT_MODE_IDLE.

    Thus, depending on the execution phase, the driver will finish the continuous mode with 1 or 2 additional callback function calls after
    ::SENSOR_STOP command.

    After stopping the continuous mode, there's an opportunity to start it again with the same characteristics as before, by function
    ::spiDriver_RunContinuousMode without additional initialization by ::spiDriver_InitContinuousMode.

 */

#endif /* SPI_DRV_CONT_MODE_DOC_H */

