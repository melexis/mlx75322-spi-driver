/**
 * @file
 * @brief REGMAP tools data types and API prototypes
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
 * @defgroup spi_tools SPI Driver tools, types and helper functions
 * @ingroup spi_driver
 *
 * @details a certain number of libraries and sources, providing the external communication and parsing
 *     features.
 *
 */

#ifndef SPI_DRV_TOOLS_H
#define SPI_DRV_TOOLS_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

/** Reverses bits order within the 32-bits value
 * @param[in]   bitsData        input data value
 * @return      result value with inverted bits order
 */
uint32_t ReverseBits32(uint32_t bitsData);

/** Reverses bits order within the 16-bits value
 * @param[in]   bitsData        input data value
 * @return      result value with inverted bits order
 */
uint16_t ReverseBits16(uint16_t bitsData);

/** Reverses bits order within the 8-bits value
 * @param[in]   bitsData        input data value
 * @return      result value with inverted bits order
 */
uint8_t ReverseBits8(uint8_t bitsData);

/** Swaps the bytes orders in 16bit words of the buffer
 * @param[in]   buffer          byte array to swap bytes
 * @param[in]   size            buffer's size, in bytes
 */
void ReverseBytes16(uint8_t* buffer, uint16_t size);

/** Returns a minimum value from 2 input arguments
 * @param[in]   a               one of arguments to compare
 * @param[in]   b               one of arguments to compare
 * @return      the minimum value from 2 arguments provided
 */
static inline int int_min(const int a, const int b)
{
    return (a < b) ? a : b;
}

/** Returns a maximum value from 2 input arguments
 * @param[in]   a               one of arguments to compare
 * @param[in]   b               one of arguments to compare
 * @return      the maximum value from 2 arguments provided
 */
static inline int int_max(const int a, const int b)
{
    return (a > b) ? a : b;
}

#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* SPI_DRV_TOOLS_H */

