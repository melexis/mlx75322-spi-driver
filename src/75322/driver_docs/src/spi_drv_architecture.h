/**
 * @file
 * @brief SPI driver architecture
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

#ifndef SPI_DRV_ARCHITECTURE_H
#define SPI_DRV_ARCHITECTURE_H

/** @mainpage MLX75322 SPI driver manual

   SPI driver architecture
   =======================

   This SPI driver is intended to support the MLX75322 product for application-level data communication layer. It is
   split in components and provides flexible build-structure, allowing to combine the components involved, for certain
   target platform.

   Modules
   -------

   These are the main modules in the SPI driver:

   - **Driver's API**
    This module provides a set of functions for the application, which cover all the following roles:

    - SPI driver configuration;
    - Connected IC system configuration via setting/getting register values;
    - Start and read data collected by IC;
    - Reading the data stream (set of scenes in continuous mode);
    - diagnostic functions;

    Beside the functions the API provides a sufficient set of data structures needed to parse the data received.

   - **Driver's communication level**

    This module provides mainly data communication layer of data protocol exchange between the host and slave.
    It's common for all platform and interacts with platform-specific Hardware Abstraction Layer (HAL).

   - **Driver's HAL**
    This module is purely related to certain HW platform used for SPI driver run. This low-level module implements
    the HW initialization and data transfer through the PHY-layer interface(s). It deals with the data exchange
    between SW and IC through the PHY-layer interface available.

   All modules of SPI driver including their platform-specific parts are packed into the one library for the system,
   allowing its use as a standalone list of features, according their API interface from set of "\*.h" header files.


   Interaction of modules
   ----------------------

   All modules of the SPI driver can be called independently from the code, but it's usage may be limited and require
   from user an advanced knowledge about the processes in IC.

   Generally only the API-level is the correct level of driver interaction.

   Through the API-level the user can access all the features available from IC, control it, get data and diagnostic
   information and run the special continuous mode to get the data flow.

   According the proper way of execution the functions for the application should be called from the API-level:

   Modules relations:

   @dot
   digraph Modules_interaction {
    node [ shape=record, fontname=Helvetica, fontsize=10 ];
    SPI [ label="SPI BUS" shape=box, style=filled, color=".6 .25 1.0" ]
    GPIO [ label="GPIO" shape=box, style=filled, color=".6 .25 1.0" ]
    COM [ label="COM Layer" URL="@ref spi_com" shape=box, style=filled, color=".45 .25 0.9" ];
    HAL_SPI_RASPI [ label="Raspberry\nPi SPI HAL" URL="@ref spi_hal_api_spi_raspi" shape=box, style=filled, color=".5 .3 1.0" ]
    HAL_GPIO_RASPI [ label="Raspberry\nPi GPIO HAL" URL="@ref spi_hal_api_gpio_raspi" shape=box, style=filled, color=".5 .3 1.0" ]
    API [ label="SPI DRIVER\nAPI" URL="@ref spi_api", style=filled, color=".45 .25 0.9" ];
    APP [ label="APPLICATION" URL="" shape=box, style=filled, color=".7 .3 1.0" ];
    HAL_SPI_RASPI -> COM [ style="dashed" ]
    HAL_GPIO_RASPI -> COM [ style="dashed" ]
    SPI -> HAL_SPI_RASPI [ arrowhead="open", style="bold", dir="both" ];
    GPIO -> HAL_GPIO_RASPI [ arrowhead="open", style="bold" ];
    COM -> API [ arrowhead="open", style="solid" ];
    API -> APP [ arrowhead="open", style="solid", dir="both" ];
    CONT [ label="Continuous mode\nlibrary" URL="@ref spi_cont_mode" shape=box, style=filled, color=".45 .25 0.9" ]
    CONT -> API  [ arrowhead="open", style="dashed" ];
    CONT -> APP
   }
   @enddot

   All SPI driver modules may be restructured and replaced or expanded with some other additional functionality and/or wrappers.
   Using the common interface of modules interaction it's possible to create the platform-specific and application specific
   implementations.

   Use cases
   =========

   The certain use cases are shown below:

   @startuml
    left to right direction
    title Top level use case
    actor User
    actor :75322: as IC
    actor :75322-n: as IC2
    rectangle Application {
        (Data\nProcessing) as APP
        note right of APP: Third-party\napplication
    }
    rectangle SPI_DRIVER {
        (Scenes\nInput) as INPUT
        (Command\nDriver) as CMD
        (Obtain\nDiagnostic\nInformation) as DEBUG_INFO
        (Configure\nSPI Driver) as DRV_CFG
        (Configure\nIC) as IC_CFG
        User -d- DRV_CFG
        User -d- IC_CFG
        User -d- DEBUG_INFO
        INPUT -d- IC
        INPUT -d- IC2
        CMD .-> APP : <<extend>>
        APP .-> INPUT : <<include>>
    }
   @enduml

   In the structure provided there are main actors shown:

   - **User** - is the main actor, which uses the system including the application, driver and ICs.
   - **75322** - the "first" IC providing the data from its sensor. This actor forms the data and communicates with the
        driver through SPI interface.
   - **75322-n** - optional actor, which may represent "one-of-many" ICs connected via the SPI HAL driver layer. The
        actors in such configuration are intended to work in synchronous mode, producing one singe "scene" at once.

   Thus, interaction between parts of the scheme is the following:

   - The **User** can **Configure SPI driver**. He can initialize and configure the HW and SW settings within the driver,
    making it work as desired.
   - The **User** can **Configure IC**. All settings of 75322 are available to be configured before running the data
    acquisition. Thus, user can setup the chip(s) in a desired mode of operation.
   - the **User** can **Obtain Diagnostic Information** by reading the SPI driver's return values content and reading
    IC's diagnostic registers.

   The **Application** is a standalone member, which uses the **SPI DRIVER** library.

   - The **Application** includes the **Scenes input** which comes from **75322** and optionally, from some set of ICs.
   - The **Application** gets control over SPI DRIVER by extending their functions with SPI DRIVER control commands. Thus,
    driver's control and control over IC's state is possible through these commands.

   Multi-platform principle
   ========================

   SPI driver applies the multi-platform principle of development, allowing to be run on any HW-SW platform. Most of
   SW code is developed for GCC C-compiler. The multi-platform principle is implemented in two levels:

   - **The SW code-level**. The SW code uses strict-size data types wherever it's possible to cover all possible
    platforms with fixed data structures for data.
   - **The Component's level**. This SPI driver is split in several components. Some of them are purely multi-platform
    and used for all platforms while some of them are platform-specific, like GPIO or SPI components. Their
    implementation varies within different HW platforms, but the common API (defined as driver's API) should be
    implemented in any platform-specific implementation.
   - **The Build-system level**. The SPI driver's build-system allows to adjust the platform's dependencies in terms of
    driver's components control, providing the compile-time flexibility for that. It also provides a flexibility for the
    end-customer to adjust the environment and be able to:

    - change the compiler;
    - change the target operating system (if applicable);
    - adjust the set of libraries used to compile the driver and all its components;
    - adjust the type of an output for the packet and it's target destination;

 */

#endif /* SPI_DRV_ARCHITECTURE_H */

