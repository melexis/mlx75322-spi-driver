/**
 * @file
 * @brief REGMAP tools interface
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
 * @ingroup spi_data
 *
 * @details Provides helper functions for reading the *.json database of IC data variables
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#define  JSMN_HEADER
#include "jsmn.h"
#undef JSMN_HEADER
#include "spi_drv_common_types.h"


/** A wrapper function for standard realloc()
 * ... with one difference - it frees old memory pointer in case of realloc
 * failure. Thus, DO NOT use old data pointer in anyway after call to
 * realloc_it(). If your code has some kind of fallback algorithm if
 * memory can't be re-allocated - use standard realloc() instead.
 *
 */
static inline void* realloc_it(void* ptrmem, size_t size)
{
    void* p = realloc(ptrmem, size);
    if (!p) {
        free(ptrmem);
        fprintf(stderr, "realloc(): errno=%d\n", errno);
    }
    return p;
}

/** Callback function type which parses the incoming JSON info, read from a file
 * Returns the number of accepted items.
 */
typedef int (* jsonParserFunc_t)(const char* js, jsmntok_t* t, size_t count, int indent);

/** Reads the FW configuration JSON file into the structure provided
 * @param[in]  f_name  JSON filename
 * @param[in]  jsonParser parser structure, which returns the data content
 * @return  result of an operation
 */
FuncResult_e ReadJson(const char* const f_name, const jsonParserFunc_t jsonParser);

/** Prints JSON content to stdout.
 * The output looks like YAML, but It's not proven to be compatible.
 * @note: the function is recurrent.
 */
int DumpJson(const char* js, jsmntok_t* t, size_t count, int indent);

#ifdef __cplusplus
}
#endif

