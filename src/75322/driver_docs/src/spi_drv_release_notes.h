/**
 * @file
 * @brief SPI driver release notes history
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

#ifndef SPI_DRV_RELEASE_NOTES_H
#define SPI_DRV_RELEASE_NOTES_H

/**
    @defgroup releases Releases

    SPI driver releases
    ===================

    Strategy of releasing
    ---------------------

    The releases of SPI driver are provided as a source code on Git, with the build-system, which is capable to build the driver as
    the library on the target platform. Each release has corresponding TAG on Git, so all changes a trackable during the
    development flow.

    After driver's build the output is placed in "build/" folder containing the following groups of files:

    - **doc** - documentation
    - **include** - driver's API
    - **all_other_files** - are the library

    Places to find the sources
    --------------------------

    The main defelopment is performed in `here <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/network/master>`_.
    All releases can be found by corresponding tags.

    How to check / define version of certain release
    ------------------------------------------------

    Driver's version for certain instance is placed into appropriate 75322-*product*.mk file, in a group of "RELEASE_*" variables.

    Modes of releases (with / without sources in doc, debug-modes etc.)
    -------------------------------------------------------------------

    According to the settings mentioned in product-specific *.mk file, the output may differ in terms of:

    - adjusting groups of components being included in driver's build;
    - adjusting the product-specific libs and compile-time flags in required;
    - adjusting the debug flags for more traceable work flow;

    For more information refer to the product-specific *.mk file for more comments in-place.


    Release name meaning ( letters "IR"/"RC" and numbers )
    ------------------------------------------------------

    For **intermediate releases** and **release candidates** the same numbering scheme is applied: IR/RC_X_Y_Z:

    - X is bound to the major releases,
    - Y is incremented on every IR/RC, when moving from IR to RC stage, the Y-number just keeps on incrementing
    on every release,
    - Z is incremented when a release needs to be amended with e.g. documentation or extra test cases. These releases
    can be partial releases (e.g. no new significant features added, only documentation update).


    SPI driver releases history
    ===========================

    Release RC 1.3.0
    ----------------

    Tag used for sources:
    `SW75322_SPI_DRV_RC_1_3_0 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/tags/SW75322_SPI_DRV_RC_1_3_0>`_

    Changes:

    - `!100 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/151>`_ Fixed issue with variables'
        initialization which caused data uncertainty;

    Release RC 1.2.0
    ----------------

    Tag used for sources:
    `SW75322_SPI_DRV_RC_1_2_0 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/tags/SW75322_SPI_DRV_RC_1_2_0>`_

    Changes:

    - Syncronous mode support added;
    - Communication-level Debug capabilities improved;
    - Continuous mode initialization improved (empty-configuration is supported);
    - Continuous mode STOP sequence updated;
    - Continuous mode start/stop/exit functions return results;
    - Continuous mode STOP function became blocking;
    - JSMN library became traceble;
    - Script-files parsing improved (correct spaces handling before comments and in empty-lines);
    - 32bit variables read/write support added;
    - GetFwVariableByOffset() function added;
    - HAL&SPI API documentation improved;

    Release RC 1.1.0
    ----------------

    Tag used for sources:
    `SW75322_SPI_DRV_RC_1_1_0 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/tags/SW75322_SPI_DRV_RC_1_1_0>`_

    Changes:

    - Documentation improvements over the sources code and descriptions. Continuous mode description;
    - Multi-IC support improvements;
    - Improved failure analisys. Use of *stderr* to handle the error messages;
    - Script-files support;
    - Example improvements for continuous mode;
    - loadPatch() function return flags bugfix;
    - getting by bit-field name function update. Now, the result bitfield is shifted right to its native offset from LSB.

    Release RC 1.0.0
    ----------------

    Tag used for sources:
    `SW75322_SPI_DRV_RC_1_0_0 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/tags/SW75322_SPI_DRV_RC_1_0_0>`_

    Changes:

    - Generic low-level cleaning.
    - `!100 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/100>`_ Initialization function has been expanded with patches and setting the configuration.
    - `!98 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/98>`_ Documentation of Release Notes has been added.
    - `!94 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/94>`_ Documentation of driver's API.
    - `!89 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/89>`_ Add an option for getting scene to disable layer's mode configuration.
    - `!87 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/87>`_ Low-level refactor bytes handling.
    - `!88 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/88>`_ Adapt the Traces' output format in data structure.
    - `!83 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/83>`_ Documentation of Release Notes added.
    - `!80 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/80>`_ Adjust input configuration API.
    - `!79 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/79>`_ Documentation of architecture added.
    - `!78 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/78>`_ Communication level prints improved.
    - `!74 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/74>`_ Documentation build system.
    - `!63 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/merge_requests/63>`_ Add libraries for Raspi to be used.


    Release IR 0.0.2
    ----------------

    Tag used for sources:
    `SW75322_SPI_DRV_IR_0_0_2 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/tags/SW75322_SPI_DRV_IR_0_0_2>`_

    Changes:

    - Support for light source switching;
    - Several significant changes for continous mode;
    - Several significant changes for single mode;
    - Several signigicant changes in low_level mode;
    - APPLY_PATCH transaction used when patch's uploading is successfully performed.
    - GPIO activity support (for driving SELn for leds).
    - Several input configuration for layers improvements;


    Release IR 0.0.1
    ----------------

    Tag used for sources:
    `SW75322_SPI_DRV_IR_0_0_1 <https://gitlab.melexis.com/m75322-host-support/mlx75322_driver_c/tags/SW75322_SPI_DRV_IR_0_0_1>`_

    Changes:

    - Driver structure maintenance in components;
    - Build-system maintenance;
    - CI jobs maintenance;
    - Low-level functions maintenance in a scope of earlier implementation. Few checks on it.
    - High-level functions maintenance in a scope of earlier implementation. Few checks on it.
    - Continuous mode first implementation.
    - Files configuration, patches and tables reading maintenance.

 */

#endif /* SPI_DRV_RELEASE_NOTES_H */

