/**
 * @file
 * @brief SPI driver synchronous mode documentation
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

#ifndef SPI_DRV_SYNC_MODE_DOC_H
#define SPI_DRV_SYNC_MODE_DOC_H

/**
    @ingroup spi_driver
    @defgroup spi_drv_multi_ic Synchronous Multi-IC mode support

    Synchronous multi-IC mode of operation
    ======================================

    Synchronous mode for multi-IC configuration operation allows to manage the several IC layers data gathering efficiently
    organized in time. It controls up to 6 ICs in a sequence, making their acquisition process synchronous.

    The synchronous mode is organized with the special IC pin connected between several ICs. This pin is managed by selected [MLX75322]
    IC in a system, called "MASTER". This IC generates the synchronous pulse on this pin allowing all other "SLAVE"s to synchronize
    their operation.

    The synchronous mode principle
    ==============================

    Synchronous mode of execution has the same flow as a standalone configuration, except the synchronization commands. The flow of each single
    chip looks exactly like the one IC control. Only when doing the SYNC and ACQU SYNC commands - the SPI driver should send them for
    slave IC's and then send them to the MASTER IC. Once received, the slaves in synchronous mode do not start their operation, waiting for
    master's pulse. In the master mode, the IC triggers the 'SYNC' pin for all devices connected, triggering their execution. After triggering
    the proper sequence, slaves and master can be read by the host device with the new layers gathered.

    Synchronous mode configuration
    =============================

    For synchronous mode of operation the configuration is available with two ways:

    - the driver has internal configuration set which is written into the ICs;
    - the ICs have its own configuration ready to execute the synchronous mode (e.g. with script- or set-files) and driver reads them before
        executing the synchronous mode;

    For the first case, when the driver controls the initialization process - the configuration has equal settings for all ICs, including:

        - layers amount per IC scenes;
        - layers order in each IC;
        - equal sync_mode (should be per FRAME SYNC);
        - equal recharge led settings;
        - equal per each layers' settings:

            - equal averaging;
            - equal trigger_period;
            - layer->sampling.mode;
            - layer->dark_frame_en;
            - layer-switching-microframe configuration;

    In this case the driver ensures the configuration may be used as a "synchronized" sequence, meaning there's a way to provide each "scene"
    instance for all layers of all ICs at once.

    The case with the settings read from ICs - the driver just checks the mentioned above parameters' consistency. The only difference is that
    the layers sequence and their amount can differ from one IC to another. This means the sequence may have **asynchronous** principle in terms
    of layers order, independent for each IC.

    In this case the sequence cannot have the 'scene' instance. Hence, the callback function returns the data for a single layer for a single IC
    continuously.

    Synchronous mode for 'synchronized' configuration
    =================================================

    In the synchronised configuration, the driver performs the following scheme of reading and delivering the data to subscriber:

    @dot
    digraph structs {
        node [shape=record, fontname=Helvetica, fontsize=10 ];
        IC1 [shape=record,label="{IC1(MASTER)|{IC2}|{IC3}}|{...|{...}|{...}}|{{ }|{ }|{AQCU SYNC + SYNC}}|{ |{AQCU SYNC + SYNC}|{ }}|{AQCU SYNC + SYNC|{ }|{ }}|{...|{...}|{...}}|{LAYER1||}|{|LAYER1|}|{||<last> LAYER1}|{...|{...}|{...}}"];
        IC1:last->CALLBACK;
    }
    @enddot

    After starting all ICs in synchronous mode and perform all required ACQU SYNC and SYNC requests - the driver starts collecting
    all ICs data in a single layers' sequence. Each of 2 frames per layer is triggered by the same ACQU SYNC and SYNC sequence.
    The very first IC is treated as a **MASTER** and goes the latest in ACQU SYNC and SYNC requests.

    When all ICs in spared mode have the same amount of layers in their scenes and these layers have compliant configuration - the driver can easily
    deliver the data as one whole scene, w/o any splitting.

    Driver and system initialization in Synchronous mode
    ====================================================

    To configure the system in synchronous mode, the following sequence is expected:

    - **Configure the IC's amount**. The function ::spiDriver_SyncModeInit() normally sets-up an amount of IC under control. It may have more
        parameters if they will be required.
    - **Configure ICs names**. To read the configuration from script-files, the driver should have IC's names. These names are configured by
        function ::spiDriver_SetupMultiICs(). This step is not required if application will not use script-files to configure the ICs.
    - **Configure IC order in a sequence**.  ::spiDriver_SetSyncIcOrder configures the driver to follow desired IC sequence in synchronous
        communication mode.

    All the rest driver configuration is performed in a usual sequence, starting from function ::spiDriver_Initialize() call.

    When the ICs number is configured more than 1, the SPI driver uses grouped calls for all ICs in a list. To see the full list of
    functions used, please read list of function in the group @ref spi_com_sync.

    Optionally, the function ::spiDriver_CheckSyncConfig() is used to check the current IC configuration consistency, to know if all ICs
    can be executed in synchronous mode.

 */

#endif /* SPI_DRV_SYNC_MODE_DOC_H */

