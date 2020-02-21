/**
 * @file
 * @brief HASH library support
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
 * @defgroup hash_lib The HASH library support
 * @ingroup spi_tools
 *
 * @details HASH library provides some HASH-generation functions allowing to "code" the string into 32-bit ID
 *
 */

#ifndef HASH_LIB_H
#define HASH_LIB_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/* HASH helper functions */

/** Calculates the 32bit HASH (by DJB's algorithm) for the string
 * @param[in]   str     defines the string to calculate the HASH for
 * @return      32bit HASH calculated
 */
uint32_t GetHashDjb2(uint8_t* str);

/** Calculates the 32bit HASH (by sdbm algorithm) for the string
 * @param[in]   str     defines the string to calculate the HASH for
 * @return      32bit HASH calculated
 */
uint32_t GetHashSdbm(uint8_t* str);

#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* HASH_LIB_H */

