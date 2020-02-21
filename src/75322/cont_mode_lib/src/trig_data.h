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
 * @defgroup spi_cont_get_data SPI driver continuous mode data acquisition thread
 * @ingroup spi_cont_mode
 *
 * @details provides the driven continuous mode thread, which drives the data acquisition process - calls the driver's
 *      data to get the data, forms the scene's data and provides it to the callers side. This thread is driven by
 *      the major thread, using messages queue.
 *
 */

#ifndef TRIG_DATA_H
#define TRIG_DATA_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

/** Initiates the trigger data thread
 * This function creates a trigger data thread and starts it waiting for data requests
 * @note    This function is normally driven from the continuous mode library
 */
void spiDriver_InitTrigData(void);

/** Runs the trigger data thread
 * This function sends a message to a trigger thread to start waiting for request messages
 * @note    This function is normally driven from the continuous mode library
 */
void spiDriver_RunTrigData(void);

/** Stops the trigger data thread
 * This function sends a message to a trigger thread to stop waiting for request messages
 * @note    This function is normally driven from the continuous mode library
 */
void spiDriver_StopTrigData(void);

/** Exits the trigger data thread
 * This function sends a message to finish the trigger thread
 * @note    This function is normally driven from the continuous mode library
 */
void spiDriver_ExitTrigData(void);

#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* TRIG_DATA_H */

