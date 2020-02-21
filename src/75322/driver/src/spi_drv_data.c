/**
 * @file
 * @brief HW and SW data support functions and structures
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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define JSMN_HEADER
#include "jsmn.h"
#undef JSMN_HEADER
#include "spi_drv_common_types.h"
#include "spi_drv_tools.h"
#include "spi_drv_data.h"
#include "regmap_tools.h"
#include "hash_lib.h"

FwFieldInfo_t* fwFields = NULL;
static uint16_t* fwNameIdx;
uint16_t fwFieldsCount = 0u;

/* Local functions declaration, to handle the input data parsing */
static int DumpFwJson(const char* js, jsmntok_t* t, size_t count, int indent);

FuncResult_e ReadFwJson(const char* const f_name)
{
    return ReadJson(f_name, DumpFwJson);
}

static void parseFwInfoBitField(FwFieldInfo_t* field, const char* js, jsmntok_t* info)
{
    strncpy(field->fldName, js + info[0].start, int_min(MAX_FLD_NAME, info[0].end - info[0].start));
    field->fldNameHash = GetHashDjb2((uint8_t*)field->fldName);
    field->bitField = strncmp("true", js + info[3].start, 4) == 0;
    field->bitOffset = atoi(js + info[5].start);
    field->bitSize = atoi(js + info[7].start);
    field->byteSize = atoi(js + info[9].start);
    field->isSigned = strncmp("true", js + info[14].start, 4) == 0;
    field->wordSize = atoi(js + info[16].start);
}

int parseFwInfo(int fwIndex, const char* js, jsmntok_t* info)
{
    FwFieldInfo_t* bfld;
    int res = 21;
    if (info->type == JSMN_OBJECT) { /* Check object's consistency */
        fwFields[fwIndex].fldAddr = atoi(js + info[2].start);
        fwFields[fwIndex].bitField = strncmp("true", js + info[4].start, 4) == 0;
        fwFields[fwIndex].bitOffset = atoi(js + info[6].start);
        fwFields[fwIndex].bitSize = atoi(js + info[8].start);
        fwFields[fwIndex].byteSize = atoi(js + info[10].start);
        fwFields[fwIndex].isSigned = strncmp("true", js + info[15].start, 4) == 0;
        fwFields[fwIndex].wordSize = atoi(js + info[17].start);
        res = 19; /* Skip subobjects */
        while (info[res].type == JSMN_OBJECT) { /* Check whether the item is an sub-object and parse the bitfield expected */
            fwFields[fwIndex].bitFieldCount++;
            fwFields[fwIndex].bitFields =
                realloc_it(fwFields[fwIndex].bitFields, sizeof(FwFieldInfo_t) * fwFields[fwIndex].bitFieldCount);
            bfld = &(fwFields[fwIndex].bitFields[fwFields[fwIndex].bitFieldCount - 1]);
            memset(bfld, 0, sizeof(FwFieldInfo_t));
            parseFwInfoBitField(bfld, js, &info[res - 1]);
            bfld->fldAddr = fwFields[fwIndex].fldAddr; /* Inherit bit-field address from parent's data structure */
            //bfld->byteSize = fwFields[fwIndex].byteSize; /* Inherit byte-size address from parent's data structure */
            bfld->bitFieldCount = 0;
            bfld->bitFields = NULL;
            res += info[res].size * 2 + 3;
        }
        fwFields[fwIndex].offset = atoi(js + info[res].start);
        for (uint8_t fldInd = 0u; fldInd < fwFields[fwIndex].bitFieldCount; fldInd++) {
            fwFields[fwIndex].bitFields[fldInd].offset = fwFields[fwIndex].offset;
        }
        res += 2;
    } else {
        return -1;
    }
    return res;
}


static bool CheckFwNamesHashUniqueness(void)
{
    bool res = true;
    uint16_t i, j;
    for (i = 0u; (i < (fwFieldsCount - 1) && res); i++) {
        for (j = (i + 1); j < fwFieldsCount; j++) {
            if (fwFields[i].fldNameHash == fwFields[j].fldNameHash) {
                res = false;
                break;
            }
        }
    }
    return res;
}


int names_compare(const void* a, const void* b)
{
    uint32_t int_a = fwFields[*(uint16_t*)a].fldNameHash;
    uint32_t int_b = fwFields[*(uint16_t*)b].fldNameHash;
    if (int_a == int_b) {
        return 0;
    } else if (int_a < int_b) {
        return -1;
    } else {
        return 1;
    }
}

static void CreateNameIndexes(void)
{
    uint16_t ind;
    for (ind = 0; ind < fwFieldsCount; ind++) {
        fwNameIdx[ind] = ind;
    }
    qsort(fwNameIdx, fwFieldsCount, sizeof(uint16_t), names_compare);
#ifdef DEBUG_DATA
    printf("Sorted hashes:\n");
    for (ind = 0; ind < fwFieldsCount; ind++) {
        printf("HASH[%5u]: 0x%08x\n", ind, fwFields[fwNameIdx[ind]].fldNameHash);
    }
#endif
}


static int DumpFwJson(const char* js, jsmntok_t* t, size_t count, int indent)
{
    int i, j, offset;
    jsmntok_t* fld;
    jsmntok_t* info;
    (void)indent;
    fwFields = NULL;
    int res = SPI_DRV_FUNC_RES_OK;
    if ((count != 0) && (t->type == JSMN_OBJECT)) {
        j = 0;
        fwFields = malloc(sizeof(FwFieldInfo_t) * t->size);
        fwNameIdx = malloc(sizeof(uint16_t) * t->size);
        for (i = 0; i < t->size; i++) {
            fld = t + 1 + j;
            info = fld + 1;
            fwFields[i].bitFieldCount = 0u;
            fwFields[i].bitFields = NULL;
            memset(fwFields[i].fldName, 0, MAX_FLD_NAME);
            strncpy(fwFields[i].fldName, js + fld->start, int_min(MAX_FLD_NAME, fld->end - fld->start));
            fwFields[i].fldNameHash = GetHashDjb2((uint8_t*)fwFields[i].fldName);
            offset = parseFwInfo(i, js, info);
            if (offset > 0) {
                j += offset;
            } else {
                res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
                fprintf(stderr, "Error while parsing field %d in FW ports configuration\n", i);
            }
        }
    } else {
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
    }
    if (res == SPI_DRV_FUNC_RES_OK) {
        printf("%d FW ports have been successfully parsed\n", t->size);
        fwFieldsCount = t->size;
        if (CheckFwNamesHashUniqueness()) {
            printf("All port names are checked as unique\n");
            CreateNameIndexes();
        } else {
            fprintf(stderr, "Error! The field-names hashes are NOT unique! Please, change the hash generation\n");
            res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
        }
    } else {
        fwFieldsCount = 0u;
    }
    return res;
}

/* Search the variable by hash, using the index */
static int qsearch(const uint32_t hash)
{
    int res = -1;
    uint32_t l_hash;
    uint32_t r_hash;
    uint32_t r_board = fwFieldsCount - 1;
    uint32_t l_board = 0u;
    l_hash = fwFields[fwNameIdx[l_board]].fldNameHash;
    r_hash = fwFields[fwNameIdx[r_board]].fldNameHash;
    if ((hash >= l_hash) && (hash <= r_hash)) {
        while (true) {
            l_hash = fwFields[fwNameIdx[l_board]].fldNameHash;
            r_hash = fwFields[fwNameIdx[r_board]].fldNameHash;
            if (hash == l_hash) {
                res = fwNameIdx[l_board];
                break;
            } else if (hash == r_hash) {
                res = fwNameIdx[r_board];
                break;
            } else if ((r_board - l_board) < 2ul) {
                break;
            } else {
                uint32_t ind = (r_board + l_board) / 2;
                if (hash < fwFields[fwNameIdx[ind]].fldNameHash) {
                    r_board = ind;
                } else {
                    l_board = ind;
                }
            }
        }
    }
    return res;
}

FwFieldInfo_t* GetFwVariableByName(const SpiDriver_FldName_t* const var_name)
{
    FwFieldInfo_t* res = NULL;
    if ((var_name != NULL) && (fwFieldsCount > 0u)) {
        uint32_t hash = GetHashDjb2((uint8_t*)var_name);
        int ind = qsearch(hash);
        if (ind >= 0) {
            res = &fwFields[ind];
        } else {
            fprintf(stderr, "Variable %s is not found\n", var_name);
        }
    }
    return res;
}


FwFieldInfo_t* GetFwBitFieldByName(const FwFieldInfo_t* const fwField, const SpiDriver_FldName_t* const field_name)
{
    FwFieldInfo_t* res = NULL;
    for (uint16_t ind = 0u; ind < fwField->bitFieldCount; ind++) {
        if (strncmp((const char*)field_name, fwField->bitFields[ind].fldName, MAX_FLD_NAME) == 0) {
            res = &fwField->bitFields[ind];
            break;
        }
    }
    if (res == NULL) {
        fprintf(stderr, "Variable bit-field %s is not found\n", field_name);
    }
    return res;
}


FwFieldInfo_t* GetFwVariableByOffset(const uint16_t offset)
{
    FwFieldInfo_t* res = NULL;
    for (uint16_t ind = 0u; ind < fwFieldsCount; ind++) {
        if (fwFields[ind].offset == offset) {
            res = &fwFields[ind];
            break;
        }
    }
    return res;
}


uint32_t spiDriver_GetBit(const uint32_t value, const uint8_t bitOffset, const uint8_t bitSize, const uint8_t byteSize)
{
    uint32_t rev_mask;
    uint8_t rev_offset;

    rev_offset = ((uint8_t)8 * byteSize) - bitOffset - bitSize; /* Reversed LSB -> MSB offset */
    rev_mask = ((1ul << bitSize) - 1u) << rev_offset;
    return value & rev_mask;
}


uint32_t spiDriver_GetBitByVar(const FwFieldInfo_t* fwField, const uint32_t value)
{
    uint32_t rev_mask;
    uint8_t rev_offset;

    rev_offset = ((uint8_t)8 * fwField->byteSize) - fwField->bitOffset - fwField->bitSize; /* Reversed LSB -> MSB offset */
    rev_mask = ((1ul << fwField->bitSize) - 1u);
    return (value >> rev_offset) & rev_mask;

}


uint16_t spiDriver_SetBit(const uint32_t value,
                          const uint32_t newValue,
                          const uint8_t bitOffset,
                          const uint8_t bitSize,
                          const uint8_t byteSize)
{
    uint32_t rev_mask;
    uint8_t rev_offset;

    rev_offset = ((uint8_t)8 * byteSize) - bitOffset - bitSize; /* Reversed LSB -> MSB offset */
    rev_mask = ((1ul << bitSize) - 1u) << rev_offset;
    /* Do Read-modify-write operation */
    return (value & ~rev_mask) | ((newValue << rev_offset) & rev_mask);
}


uint32_t spiDriver_SetBitByVar(const FwFieldInfo_t* fwField, const uint32_t value, const uint32_t newValue)
{
    uint32_t rev_mask;
    uint8_t rev_offset;

    rev_offset = ((uint8_t)8 * fwField->byteSize) - fwField->bitOffset - fwField->bitSize; /* Reversed LSB -> MSB offset */
    rev_mask = ((1ul << fwField->bitSize) - 1u) << rev_offset;
    /* Do Read-modify-write operation */
    return (value & ~rev_mask) | ((newValue << rev_offset) & rev_mask);
}

uint32_t spiDriver_GetByteByVar(const FwFieldInfo_t* fwField, const uint32_t bufValue)
{
    uint32_t res;
    if (fwField->byteSize == 1u) {
        if (fwField->fldAddr & 0x01u) {
            res = bufValue >> 8;
        } else {
            res = bufValue & (uint16_t)0xFF;
        }
    } else {
        res = bufValue;
    }
    return res;
}

uint32_t spiDriver_SetByteByVar(const FwFieldInfo_t* fwField, const uint32_t bufValue, const uint32_t fieldValue)
{
    uint32_t res;
    if (fwField->byteSize == 1) {
        if (fwField->fldAddr & 0x01u) {
            res = (bufValue & (uint32_t)0x00FFu) | (fieldValue << 8);
        } else {
            res = (bufValue & (uint32_t)0xFF00u) | (fieldValue);
        }
    } else {
        res = fieldValue;
    }
    return res;
}


uint16_t spiDriver_GetByteByName(const SpiDriver_FldName_t* const varName, const uint16_t bufValue)
{
    uint16_t res = SPI_DRV_ERR_VALUE;
    FwFieldInfo_t* fwField = GetFwVariableByName(varName);
    if (fwField != NULL) {
        res = spiDriver_GetByteByVar(fwField, bufValue);
    } else {
        fprintf(stderr, "Variable '%s' is not found in configuration\n", varName);
    }
    return res;
}


uint16_t spiDriver_SetByteByName(const SpiDriver_FldName_t* const varName,
                                 const uint16_t bufValue,
                                 const uint16_t fieldValue)
{
    uint16_t res = SPI_DRV_ERR_VALUE;
    FwFieldInfo_t* fwField = GetFwVariableByName(varName);
    if (fwField != NULL) {
        res = spiDriver_SetByteByVar(fwField, bufValue, fieldValue);
    } else {
        fprintf(stderr, "Variable '%s' is not found in configuration\n", varName);
    }
    return res;
}


uint16_t spiDriver_CalcAddress(const uint16_t address)
{
    return address & ~(uint16_t)1u;
}

