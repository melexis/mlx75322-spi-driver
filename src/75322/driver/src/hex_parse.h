/**
 * @file
 * @brief Intel-HEX file parser
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
 * @defgroup intel_hex Intel HEX files library support
 * @ingroup spi_tools
 *
 * @details
 */

#ifndef HEX_PARSE_H
#define HEX_PARSE_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

/** Intel_HEX parser current/result state */
typedef struct {
    uint32_t start_offset;      /**< Initial data offset in data parsed */
    uint32_t cur_line_num;      /**< Current line number (The latest line parsed) */
    uint32_t current_offset;    /**< Current data offset in data parsed (The latest data parsed) */
    uint32_t buffer_size;       /**< Current data buffer size of data buffer */
    uint8_t* data_buffer;       /**< Data buffer */
} IHexInfo_t;

/** Loads the Intel HEX-file into the buffer
 * Allocates the buffer and parses the Intel HEX-file into it, checking all constraints. Allocated memory should be freed by caller.
 * The caller should not allocate buffer for the data. Thus, all data inside the ihex_info structure is not used by this function as an input.
 *
 * @note    the parser checks per-line data area integrity. Thus, the data in HEX file should not be interrupted or shuffled.
 * @note    each line is checked with its checksum, but the CRC test is skipped.
 *
 * @param[in]       fileName    The file in Intel-HEX format to load
 * @param[in]       ihex_info   The data structure filled by the function
 * @retval  true    Intel-HEX file parsing was successful
 * @retval  false   Intel-HEX file parsing had some errors
 */
bool ihex_LoadFile(const char* const fileName, IHexInfo_t* ihex_info);

#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* HEX_PARSE_H */

