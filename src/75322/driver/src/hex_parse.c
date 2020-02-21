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
 * @ingroup intel_hex
 *
 * @details
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "hex_parse.h"

/** Line header data size */
#define IHEX_INFO_LEN_MIN 10

/** Intel-hex line-types used in parser */
typedef enum {
    IHEX_SIGN_DATA = 0x00u,       /**< Data Intel-hex line sign */
    IHEX_SIGN_EOF = 0x01u,        /** End-of-line Intel-hex line sign */
    IHEX_SIGN_CRC = 0x03u,        /** CRC checksum Intel-hex line sign */
} IHex_Sign_e;


/** Parses specified sub-string from a string, converting it into unsigned long value
 * @param[in]   str     character string to parse
 * @param[in]   length  number of desired characters being parsed
 * @param[in]   value   a buffer to write result into
 * @retval  true    HEX value parsing was successful
 * @retval  false   HEX value parsing had some errors
 */
static bool strntohex(const char* const str, uint8_t length, uint32_t* value)
{
#define HEX_MAX_STR 9
    char* res;
    uint32_t val;
    bool ret_res;
    char buf[HEX_MAX_STR];
    if (length >= HEX_MAX_STR) {
        length = HEX_MAX_STR;
    }
    strncpy(buf, str, length);
    buf[length] = '\0';
    val = strtoul(buf, &res, 16);
    if (*res == '\0') { /* Check parse correctness */
        *value = val;
        ret_res = true;
    } else {
        ret_res = false;
    }
    return ret_res;
}


/** Parses one line from Intel-HEX file
 * The function parses the input text string into a set of bytes, when the line contains data.
 * It expands the buffer if it's needed.
 * If line does not contain any data, the parser skips it (with positive response, if the content is correct).
 *
 * @param[in]   ihex    pointer to a structure with the current Intel-hex parser state
 * @param[in]   string  the line read from Intel-HEX file
 *
 * @retval  true    Intel-HEX line parsing was successful
 * @retval  false   Intel-HEX line parsing had some errors
 */
static bool ihex_ParseStr(IHexInfo_t* ihex, char* string)
{
    uint32_t cnt, offs, ib;
    uint8_t ind, bb, cs, org_cs, str_type;
    bool res = false;

    if ((string[0] == ':') && (strlen(string) > IHEX_INFO_LEN_MIN)) {
        if ( (strntohex(&(string[1]), 2u, &cnt)) &&
             (strntohex(&(string[3]), 4u, &offs)) &&
             (strntohex(&(string[7]), 2u, &ib))) {
            str_type = ib;
            if (ihex->buffer_size == 0UL) { /* Data array init  */
                ihex->start_offset = offs;
                ihex->current_offset = offs;
            }
            cs = 0u;
            cs = cs - cnt - (offs & 0xFF) - ((offs >> 8) & 0xFF);
            ind = 0u;
            if (str_type == IHEX_SIGN_DATA) {
                if (offs == ihex->current_offset) { /* Expect uninterrupted data sequence */
                    offs -= ihex->start_offset;
                    ihex->data_buffer = realloc(ihex->data_buffer, ihex->buffer_size + cnt);
                    ihex->buffer_size += cnt;
                    while (ind < cnt) {
                        if (strntohex(&(string[(ind * 2) + 9]), 2u, &ib)) {
                            bb = (uint8_t)ib;
                            ihex->data_buffer[ind + offs] = bb;
                            cs -= bb;
                            ind++;
                        } else {
                            /* Error in data */
                            return res;
                        }
                    }
                    ihex->current_offset += ind;
                } else {
                    /* Error: splitted hex areas are not supported. Fill-in missing areas with some data */
                    return res;
                }
            } else if (str_type == IHEX_SIGN_EOF) {
                /* EOF line detected. Do nothing */
                res = true;
            } else if (str_type == IHEX_SIGN_CRC) {
                /* CRC line's detected. Skip */
                res = true;
            } else {
                /* Error in string type. Not supported */
            }
            if ((strntohex(&(string[(ind * 2) + 9]), 2u, &ib)) && (!res)) { /* Parse and check the CS */
                org_cs = (uint8_t)ib;
                if (org_cs == cs) {
                    res = true;
                } else {
                    /* Error: wrong checksum */
                }
            }
        } else {
            /* Error in info header */
        }
    } else {
        /* Wrong line header */
    }
    if (!res) {
        printf("Error in HEX file\n");
    }
    return res;
}


bool ihex_LoadFile(const char* const fileName, IHexInfo_t* ihex_info)
{
    FILE* fp;
    char line[256];
    bool res = false;
    if ((fileName != NULL) && (ihex_info != NULL)) {
        memset(ihex_info, 0, sizeof(*ihex_info));
        fp = fopen(fileName, "r");
        if (fp != NULL) {
            res = true;
            while (fgets(line, sizeof(line), fp) && res) {
                res = ihex_ParseStr(ihex_info, line);
                ihex_info->cur_line_num++;
                if (!res) {
                    printf("HEX Error in line %u\n", ihex_info->cur_line_num);
                }
            }
            fclose(fp);
        } else {
            /* Error cannot open file for reading */
        }
    }
    return res;
}

