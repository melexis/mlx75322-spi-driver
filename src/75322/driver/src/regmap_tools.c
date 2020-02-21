/**
 * @file
 * @brief REGMAP tools
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "jsmn.h"
#include "regmap_tools.h"

int DumpJson(const char* js, jsmntok_t* t, size_t count, int indent)
{
    int i, j, k;
    jsmntok_t* key;
    if (count == 0) {
        return 0;
    }
    if (t->type == JSMN_PRIMITIVE) {
        printf("%.*s", t->end - t->start, js + t->start);
        return 1;
    } else if (t->type == JSMN_STRING) {
        printf("'%.*s'", t->end - t->start, js + t->start);
        return 1;
    } else if (t->type == JSMN_OBJECT) {
        printf("\n");
        j = 0;
        for (i = 0; i < t->size; i++) {
            for (k = 0; k < indent; k++) {
                printf("  ");
            }
            key = t + 1 + j;
            j += DumpJson(js, key, count - j, indent + 1);
            if (key->size > 0) {
                printf(": ");
                j += DumpJson(js, t + 1 + j, count - j, indent + 1);
            }
            printf("\n");
        }
        return j + 1;
    } else if (t->type == JSMN_ARRAY) {
        j = 0;
        printf("\n");
        for (i = 0; i < t->size; i++) {
            for (k = 0; k < indent - 1; k++) {
                printf("  ");
            }
            printf("   - ");
            j += DumpJson(js, t + 1 + j, count - j, indent + 1);
            printf("\n");
        }
        return j + 1;
    }
    return 0;
}


FuncResult_e ReadJson(const char* const f_name, const jsonParserFunc_t jsonParser)
{
    long int r;
    int eof_expected = 0;
    char* js = NULL;
    size_t jslen = 0;
    uint32_t f_size;
    char* buf;
    bool op_complete;
    FILE* fp;

    jsmn_parser p;
    jsmntok_t* tok;
    size_t tokcount = 2;

    /* Prepare parser */
    jsmn_init(&p);

    /* Allocate some tokens as a start */
    tok = malloc(sizeof(*tok) * tokcount);
    if (tok == NULL) {
        fprintf(stderr, "malloc(): errno=%d\n", errno);
        return SPI_DRV_FUNC_RES_FAIL_MEMORY;
    }

    fp = fopen(f_name, "r");
    fseek(fp, 0L, SEEK_END);
    f_size = ftell(fp);
    rewind(fp);
    if (fp != NULL) {
        buf = malloc(f_size);
        if (buf == NULL) {
            return SPI_DRV_FUNC_RES_FAIL_MEMORY;
        }
        for ( ; ;) {
            /* Read another chunk */
            r = fread(buf, 1, f_size, fp);
            if (r < 0) {
                fprintf(stderr, "fread(): %lu, errno=%d\n", r, errno);
                return SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
            }
            if (r == 0) {
                if (eof_expected != 0) {
                    break;
                } else {
                    fprintf(stderr, "fread(): unexpected EOF\n");
                    return SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
                }
            }

            js = realloc_it(js, jslen + r + 1);
            if (js == NULL) {
                return SPI_DRV_FUNC_RES_FAIL_MEMORY;
            }
            strncpy(js + jslen, buf, r);
            jslen = jslen + r;

            op_complete = false;
            while (!op_complete) {
                r = jsmn_parse(&p, js, jslen, tok, tokcount);
                if (r < 0) {
                    if (r == JSMN_ERROR_NOMEM) {
                        tokcount = tokcount * 2;
                        tok = realloc_it(tok, sizeof(*tok) * tokcount);
                        if (tok == NULL) {
                            return SPI_DRV_FUNC_RES_FAIL_MEMORY;
                        }
                        op_complete = false;
                    } else {
                        if (r == JSMN_ERROR_PART) {
                            break;
                        } else {
                            op_complete = false;
                        }
                    }
                } else {
                    jsonParser(js, tok, p.toknext, 0);
                    free(tok);
                    eof_expected = 1;
                    op_complete = true;
                }
            }
        }
        fclose(fp);
        free(buf);
        return SPI_DRV_FUNC_RES_OK;
    } else {
        fprintf(stderr, "fopen(): Cannot open file %s, errno=%d\n", f_name, errno);
        return SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
    }
}

