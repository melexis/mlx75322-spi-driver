/**
 * @file
 * @brief SPI C-driver compile-time checks
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
 * @ingroup spi_tools
 *
 * @details
 *
 */

#ifndef STATIC_ASSERT_
#define STATIC_ASSERT_

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define ASSERT_CONCAT2_(a, b) a ## b
#define ASSERT_CONCAT1_(a, b) ASSERT_CONCAT2_(a, b)
/**
 * Static (compile time) assertion
 *
 * @note
 * If assert condition fails will try to create a dummy array
 * with a negative size which will be caught by compiler
 *
 * @note
 * Change which includes macro concatenation was implemented
 * so that we have uniquely named assertion_failed arrays which
 * is required to avoid warnings from Eclipse indexer. There is
 * extensive discussion about the solution on Gitlab mergerequest 5
 */
#define ASSERT(condition)   \
    extern int8_t ASSERT_CONCAT1_(assertion_failed, __LINE__)[(condition) ? 1 : -1]

#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* STATIC_ASSERT_ */

