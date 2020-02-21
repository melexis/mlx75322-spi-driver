/**
 * @file
 * @brief HW and SW data API for high-level application interaction
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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "spi_drv_common_types.h"
#include "spi_drv_tools.h"
#include "spi_drv_api.h"
#include "spi_drv_data.h"
#include "spi_drv_com.h"
#include "spi_drv_sync_com.h"
#include "hex_parse.h"
#include "spi_drv_trace.h"
#include "cont_mode_lib.h"

static spiDriver_InputConfiguration_t* spiDriver_Configuration = NULL;
static char delimiters[16] = SPI_DRV_TEXT_DEFAULT_DELIMITERS;
static char defaultIcName[] = "M75322";
static char* icIntNames[MAX_IC_ID_NUMBER] = {defaultIcName};
uint16_t icIntNamesNumber = 1;

const char* spiDriverScriptCommandStrings[SUPPORTED_SCRIPT_COMMANDS] = {
    "nop",
    "write",
    "read",
    "sleep",
    "import"
};

extern ContModeCfg_t contModeCfg;
extern volatile SpiDriver_State_t spiDriver_currentState;

void spiDriver_SetDelimiter(const char* const tableDelimiters)
{
    if (tableDelimiters != NULL) {
        memset(delimiters, 0, (sizeof(delimiters) / sizeof(char)));
        strncpy(delimiters, tableDelimiters, (sizeof(delimiters) / sizeof(char)) - 1u);
    } else {
        fprintf(stderr, "Error. Delimiter cannot be set because the input pointer is\n");
    }
}

spiDriver_Status_t spiDriver_Initialize(const spiDriver_InputConfiguration_t* const spiDriver_InputCfg)
{
    spiDriver_Status_t res = SPI_DRV_FALSE;
    FuncResult_e drv_res;
    char* fname;


    if (syncModeCfg.icCount == 0u) {
        syncModeCfg.icCount = 1u;
        contModeCfg.useAsyncSequence = false;
        memset((void*)spiDriver_currentState.params, 0u, sizeof(spiDriver_currentState.params[0u]) * MAX_IC_ID_NUMBER);
    }

    spiDriver_Configuration = (spiDriver_InputConfiguration_t*)spiDriver_InputCfg;
    free(fwFields);
    fwFieldsCount = 0;
    if (spiDriver_Configuration != NULL) {
        drv_res = ReadFwJson(spiDriver_Configuration->fwFileName);
        if (drv_res == SPI_DRV_FUNC_RES_OK) {
            drv_res = spiCom_Init(spiDriver_InputCfg->spiComCfg);
            if (drv_res != SPI_DRV_FUNC_RES_OK) {
                fprintf(stderr, "Error [%u] when configuring the communication layer\n", drv_res);
                res = SPI_DRV_FALSE;
            } else {
                res = SPI_DRV_TRUE;
            }
            fname = (char*)spiDriver_InputCfg->patchFileName;
            if (fname != NULL) {
                if (fname[0] != '\0') {
                    res = spiDriver_LoadPatch(fname);
                    if (res != SPI_DRV_TRUE) {
                        fprintf(stderr, "Error when loading the patch file %s\n", fname);
                        res = SPI_DRV_FALSE;
                    }
                }
            }
            fname = (char*)spiDriver_InputCfg->scriptFileName;
            if (fname != NULL) {
                if (fname[0] != '\0') {
                    drv_res = spiDriver_RunScript(fname);
                    if (drv_res != SPI_DRV_FUNC_RES_OK) {
                        fprintf(stderr, "Error (%d) when loading the script file %s\n", drv_res, fname);
                        res = SPI_DRV_FALSE;
                    }
                }
            }
            fname = (char*)spiDriver_InputCfg->configFileName;
            if (fname != NULL) {
                if (fname[0] != '\0') {
                    drv_res = spiDriver_WriteVariablesFromFile(fname);
                    if (drv_res != SPI_DRV_FUNC_RES_OK) {
                        fprintf(stderr, "Error (%d) when loading the configuration file %s\n", drv_res, fname);
                        res = SPI_DRV_FALSE;
                    }
                }
            }
        } else {
            fprintf(stderr, "Error (%d) when read the file %s\n", drv_res, spiDriver_Configuration->fwFileName);
        }
    } else {
        fprintf(stderr, "Input configuration pointer is NULL\n");
    }
    return res;
}


spiDriver_Status_t spiDriver_LoadPatch(const char* const patchFileName)
{
    IHexInfo_t hex;
    spiDriver_Status_t res = SPI_DRV_TRUE;
    FuncResult_e comRes = SPI_DRV_FUNC_RES_OK;
    if (ihex_LoadFile(patchFileName, &hex)) {
        printf("Patch file (%s) has initial offset 0x%04X. 0x%04X(%u) bytes in %u lines\n",
               patchFileName, hex.start_offset, hex.buffer_size, hex.buffer_size, hex.cur_line_num);
        ReverseBytes16(hex.data_buffer, hex.buffer_size);
        comRes |= spiCom_WriteSyncPatch(0u, hex.buffer_size, hex.data_buffer);
        free(hex.data_buffer);
        if (comRes == SPI_DRV_FUNC_RES_OK) {
            comRes |= spiCom_ApplySyncPatch();
        }
        if (comRes != SPI_DRV_FUNC_RES_OK) {
            res = SPI_DRV_FALSE;
        }
    } else {
        fprintf(stderr, "Cannot load the patch file %s\n", patchFileName);
        res = SPI_DRV_FALSE;
    }
    return res;
}


FuncResult_e spiDriver_ReadVariables(uint32_t* valuesBuffer,
                                     SpiDriver_FldName_t** varsList,
                                     SpiDriver_FldName_t** fldsList,
                                     uint16_t varsNumber)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    SpiDriver_FldName_t* varName;
    SpiDriver_FldName_t* fldName;
    uint32_t valueBuffer;

    if ((varsList == NULL) && (varsNumber > fwFieldsCount)) {
        API_PRINT(
            "requested variables' number with empty input list is bigger than the actual variables list [%u of %u]. Hence the number is shrinked.\n",
            varsNumber,
            fwFieldsCount);
        varsNumber = fwFieldsCount;
    }
    if ((valuesBuffer != NULL) && (varsNumber != 0u)) {
        for (uint16_t ind = 0u; (ind < varsNumber) && (res == SPI_DRV_FUNC_RES_OK); ind++) {
            if (varsList != NULL) {
                varName = varsList[ind];
            } else {
                varName = fwFields[ind].fldName;
            }
            if (fldsList != NULL) {
                fldName = fldsList[ind];
            } else {
                fldName = NULL;
            }
            res |= spiDriver_GetByName(varName, &valueBuffer, fldName);
            valuesBuffer[ind] = valueBuffer;
        }
    } else {
        fprintf(stderr, "Read variables is not possible since the output values buffer is NULL\n");
    }
    return res;
}


FuncResult_e spiDriver_WriteVariables(uint32_t* valuesBuffer,
                                      SpiDriver_FldName_t** varsList,
                                      SpiDriver_FldName_t** fldsList,
                                      uint16_t varsNumber)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    SpiDriver_FldName_t* varName;
    SpiDriver_FldName_t* fldName;
    uint32_t valueBuffer;

    if ((varsList == NULL) && (varsNumber > fwFieldsCount)) {
        API_PRINT(
            "writing variables' number with empty input list is bigger than the actual variables list [%u of %u]. Hence the number is shrinked.\n",
            varsNumber,
            fwFieldsCount);
        varsNumber = fwFieldsCount;
    }
    if ((valuesBuffer != NULL) && (varsNumber != 0u)) {
        for (uint16_t ind = 0u; (ind < varsNumber) && (res == SPI_DRV_FUNC_RES_OK); ind++) {
            if (varsList != NULL) {
                varName = varsList[ind];
            } else {
                varName = fwFields[ind].fldName;
            }
            valueBuffer = valuesBuffer[ind];
            if (fldsList != NULL) {
                fldName = fldsList[ind];
            } else {
                fldName = NULL;
            }
            res = spiDriver_SetByName(varName, valueBuffer, fldName);
        }
    } else {
        fprintf(stderr, "Writing variables is not possible since the values buffer is NULL\n");
    }
    return res;
}


FuncResult_e spiDriver_SetByName(const SpiDriver_FldName_t* const varName,
                                 uint32_t value,
                                 const SpiDriver_FldName_t* const bitFieldName)
{
    FwFieldInfo_t* var;
    FuncResult_e res;
    var = GetFwVariableByName(varName);
    if (var != NULL) {
        uint32_t cur_value;
        res = spiCom_Read(var->offset, var->wordSize, (uint16_t*)&cur_value);
        if (res == SPI_DRV_FUNC_RES_OK) {
            uint32_t new_value = 0ul;
            if ((bitFieldName != NULL) && (bitFieldName[0] != '\0')) {
                FwFieldInfo_t* bvar;
                bvar = GetFwBitFieldByName(var, bitFieldName);
                if (bvar == NULL) {
                    res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
                } else {
                    new_value = spiDriver_SetBitByVar(bvar, cur_value, value);
                }
            } else {
                new_value = spiDriver_SetByteByVar(var, cur_value, value);
            }
            if (res == SPI_DRV_FUNC_RES_OK) {
                res = spiCom_Write(var->offset, var->wordSize, (uint16_t*)&new_value, false);
            }
        }
    } else {
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
    }
    return res;
}


FuncResult_e spiDriver_GetByName(const SpiDriver_FldName_t* const varName,
                                 uint32_t* const value,
                                 const SpiDriver_FldName_t* const bitFieldName)
{
    FwFieldInfo_t* var;
    FuncResult_e res;
    var = GetFwVariableByName(varName);
    if (var != NULL) {
        uint32_t cur_value = 0ul;
        res = spiCom_Read(var->offset, var->wordSize, (uint16_t*)&cur_value);
        if (res == SPI_DRV_FUNC_RES_OK) {
            if ((bitFieldName != NULL) && (bitFieldName[0] != '\0')) {
                FwFieldInfo_t* bvar;
                bvar = GetFwBitFieldByName(var, bitFieldName);
                if (bvar == NULL) {
                    res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
                } else {
                    *value = spiDriver_GetBitByVar(bvar, cur_value);
                }
            } else {
                *value = (uint32_t)spiDriver_GetByteByVar(var, cur_value);
            }
        }
    } else {
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
    }
    return res;
}


uint16_t strncopyStripped(const char* const lineString, uint16_t maxSize, const SpiDriver_FldName_t* dest)
{
    const char* line = lineString;
    char* int_dest = (char*)dest;
    while ((*line != '\0') && (strchr(delimiters, *line) != NULL)) {
        line++;
    }
    while ((*line != '\0') && (strchr(delimiters, *line) == NULL) && (maxSize != 0u)) {
        *int_dest++ = *line++;
        maxSize--;
    }
    *int_dest = '\0';
    return (uint16_t)(line - lineString);
}

bool readValue(const char* const valLine, uint32_t* const resValue)
{
    bool res = false;
    char* delPos;
    if (strncmp("0x", valLine, 2u) == 0) {
        *resValue = strtoul(valLine, &delPos, 16);
        res = true;
    } else {
        /* check whether we deal with value at all */
        if ((valLine[0] >= '0') && (valLine[0] <= '9')) {
            *resValue = strtoul(valLine, &delPos, 10);
            res = true;
        }
    }
    return res;
}


/** Gets the data from the text line
 * Uses the format:
 *      "<cmd> <IC> <name> <delimiter> [<fieldname><delimiter>] <value>".
 * The function strips the spaces and tabs for var_name on both sides,
 * considering them as 'delimiter' field. It also discards empty lines, and lines with "#" sign at the end of line.
 * The function also supports bit-fields assignment.
 * @param[in]   lineString      Input (source) string
 * @param       varName         The buffer being filled with variables' name. Cuts the name with number of MAX_FLD_NAME symbols
 * @param       fldName         The buffer being filled with variables' field name (if it exists in the line). Cuts the name with number of MAX_FLD_NAME symbols
 * @param       resValue        Buffer for a value to return (format starting from "0x" is used for HEX values);
 * @retval      SPI_DRV_FUNC_RES_OK successfull data parse
 * @retval      SPI_DRV_FUNC_RES_FAIL_INPUT_CFG the data cannot be assigned (but it shouldn't be treated as an error, just skip the line)
 * @retval      SPI_DRV_FUNC_RES_FAIL_INPUT_DATA something wrong is with the data
 */
static FuncResult_e spiDriverWriteVarFromString(const char* const lineString,
                                                SpiDriver_FldName_t* varName,
                                                SpiDriver_FldName_t* fldName,
                                                uint32_t* const resValue)
{
    char valBuf[256];
    char* pvalLine;
    uint16_t lenBuf;
    char* delPos;
    uint16_t lenStr;
    bool doExit = false;
    uint16_t fmt_ind = 0u;
    FuncResult_e res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;

    // Clean-up delimiters
    pvalLine = (char*)lineString;
    while ((*pvalLine != '\0') && (strchr(delimiters, *pvalLine) != NULL)) {
        pvalLine++;
    }
    // Strip comments:
    delPos = strchr(pvalLine, SPI_DRV_TEXT_DEFAULT_COMMENTS);
    if (delPos != NULL) {
        lenStr = ((long)delPos - (long)pvalLine) / sizeof(char);
    } else {
        lenStr = strlen(pvalLine);
    }
    if (lenStr > 3) {
        while (!doExit) {
            lenBuf = strncopyStripped(pvalLine, lenStr - 1, valBuf); /* Get the new field in a sequence */
            switch (fmt_ind) {
                case 0: // Variable name
                    API_PRINT("Varname: %s, ", valBuf);
                    strcpy(varName, valBuf);
                    break;

                case 1: // Field-name / value
                    if (readValue(valBuf, resValue)) {
                        API_PRINT("Value: %u\n", *resValue);
                        *fldName = '\0'; // Erase the bit-field name
                        res = SPI_DRV_FUNC_RES_OK;
                        doExit = true;
                    } else {
                        /* Try to treat the value field as a bitfield and go further */
                        strcpy(fldName, valBuf);
                        API_PRINT("FldName: %s, ", valBuf);
                    }
                    break;

                case 2: // Value-only (when there's field-name)
                    if (readValue(valBuf, resValue)) {
                        API_PRINT("Value: %u\n", *resValue);
                        res = SPI_DRV_FUNC_RES_OK;
                        doExit = true;
                    } else {
                        doExit = true;
                        strcpy(fldName, valBuf);
                        API_PRINT("Error in value\n");
                    }
                    break;
            }
            lenStr -= lenBuf;
            pvalLine += lenBuf;
            fmt_ind++;
        }
    } else {
        // Skipping the line
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
    }
    return res;
}



FuncResult_e spiDriver_WriteVariablesFromFile(const char* const varsFilename)
{
    uint32_t tmpValue;
    FILE* fp;
    char line[256];
    SpiDriver_FldName_t varName[MAX_FLD_NAME];
    SpiDriver_FldName_t fldName[MAX_FLD_NAME];
    uint16_t cur_line_num = 0u;
    FuncResult_e res;
    uint32_t* valsBuffer = NULL;
    SpiDriver_FldName_t** namesBuffer = NULL;
    SpiDriver_FldName_t** fldsBuffer = NULL;
    if (varsFilename != NULL) {
        fp = fopen(varsFilename, "r");
        if (fp != NULL) {
            res = SPI_DRV_FUNC_RES_OK;
            while ((fgets(line, sizeof(line), fp) != NULL) && (res == SPI_DRV_FUNC_RES_OK)) {
                res = spiDriverWriteVarFromString(line, varName, fldName, &tmpValue);
                if (res == SPI_DRV_FUNC_RES_OK) {
                    valsBuffer = realloc(valsBuffer, (cur_line_num + 1) * sizeof(uint32_t));
                    valsBuffer[cur_line_num] = tmpValue;
                    namesBuffer = realloc(namesBuffer, (cur_line_num + 1) * sizeof(SpiDriver_FldName_t*));
                    namesBuffer[cur_line_num] = malloc((strlen(varName) + 1) * sizeof(SpiDriver_FldName_t));
                    strcpy(namesBuffer[cur_line_num], varName);
                    fldsBuffer = realloc(fldsBuffer, (cur_line_num + 1) * sizeof(SpiDriver_FldName_t*));
                    if (fldName[0] != '\0' ) {
                        fldsBuffer[cur_line_num] = malloc((strlen(fldName) + 1) * sizeof(SpiDriver_FldName_t));
                        strcpy(fldsBuffer[cur_line_num], fldName);
                    } else {
                        fldsBuffer[cur_line_num] = NULL;
                    }
                    cur_line_num++;
                } else if (res == SPI_DRV_FUNC_RES_FAIL_INPUT_CFG) {
                    /* No worries. Just skip the line */
                    res = SPI_DRV_FUNC_RES_OK;
                } else {
                    fprintf(stderr, "Variables file [%s] read error, line %u\n", varsFilename, cur_line_num + 1);
                    res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
                    break;
                }
            }
            fclose(fp);
            if (res == SPI_DRV_FUNC_RES_OK) {
                res = spiDriver_WriteVariables(valsBuffer, namesBuffer, fldsBuffer, cur_line_num);
            }
            spiDriver_FreeVariableNamesArray(namesBuffer, fldsBuffer, cur_line_num);
            free(valsBuffer);
        } else {
            res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
            fprintf(stderr, "Error: Cannot open file [%s] for reading\n", varsFilename);
        }
    } else {
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
        fprintf(stderr, "Error: Read variables from file : filename is not assigned\n");
    };
    return res;
}


FuncResult_e spiDriver_ReadVariablesIntoFile(const char* const varsFilename,
                                             char delimiter,
                                             SpiDriver_FldName_t** varsList,
                                             SpiDriver_FldName_t** fldsList)
{
    FILE* fp;
    char line[256];
    SpiDriver_FldName_t* varName;
    SpiDriver_FldName_t* fldName;
    FuncResult_e res;
    uint32_t* valsBuffer = NULL;
    uint16_t valsCount = 0u;
    if (varsFilename != NULL) {
        /* get variables number to read from IC */
        if (varsList != NULL) {
            while ((varsList[valsCount] != NULL) && (varsList[valsCount][0u] != '\0')) {
                valsCount++;
            }
        } else {
            valsCount = fwFieldsCount;
        }
        valsBuffer = malloc(valsCount * sizeof(uint32_t));
        res = spiDriver_ReadVariables(valsBuffer, varsList, fldsList, valsCount);
        if (res == SPI_DRV_FUNC_RES_OK) {
            fp = fopen(varsFilename, "w");
            if (fp != NULL) {
                for (uint16_t varInd = 0u; varInd < valsCount; varInd++) {
                    if (varsList != NULL) {
                        varName = varsList[varInd];
                    } else {
                        varName = fwFields[varInd].fldName;
                    }
                    if (fldsList != NULL) {
                        fldName = fldsList[varInd];
                    } else {
                        fldName = NULL;
                    }
                    if (fldName == NULL) {
                        sprintf(line, "%s%c 0x%08X\n", varName, delimiter, valsBuffer[varInd]);
                    } else {
                        sprintf(line, "%s%c %s%c 0x%08X\n", varName, delimiter, fldName, delimiter,
                                valsBuffer[varInd]);
                    }
                    fputs(line, fp);
                }
                fclose(fp);
            } else {
                res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
                fprintf(stderr, "Error: Cannot open file [%s] for writing\n", varsFilename);
            }
        }
        free(valsBuffer);
    } else {
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
        fprintf(stderr, "Error: Write variables into file : filename is not assigned\n");
    }
    return res;
}


FuncResult_e spiDriver_ReadVariableNamesFromFile(const char* const varsFilename,
                                                 SpiDriver_FldName_t*** varsListOut,
                                                 SpiDriver_FldName_t*** fldsListOut,
                                                 uint16_t* varsCount)
{
    FILE* fp;
    char line[256];
    SpiDriver_FldName_t varName[MAX_FLD_NAME];
    SpiDriver_FldName_t fldName[MAX_FLD_NAME];
    FuncResult_e res;
    uint32_t tmpValue;
    *varsCount = 0u;
    SpiDriver_FldName_t** varsList = *varsListOut;
    SpiDriver_FldName_t** fldsList = *fldsListOut;
    if (varsFilename != NULL) {
        fp = fopen(varsFilename, "r");
        if (fp != NULL) {
            res = SPI_DRV_FUNC_RES_OK;
            while ((fgets(line, sizeof(line), fp) != NULL) && (res == SPI_DRV_FUNC_RES_OK)) {
                res = spiDriverWriteVarFromString(line, varName, fldName, &tmpValue);
                (void)tmpValue; /* not used, allocated because the common parser requires it */
                if (res == SPI_DRV_FUNC_RES_OK) {
                    varsList = realloc(varsList, ((*varsCount) + 1) * sizeof(SpiDriver_FldName_t*));
                    varsList[*varsCount] = malloc((strlen(varName) + 1) * sizeof(SpiDriver_FldName_t));
                    strcpy(varsList[*varsCount], varName);
                    fldsList = realloc(fldsList, ((*varsCount) + 1) * sizeof(SpiDriver_FldName_t*));
                    if (fldName[0] != '\0') {
                        fldsList[*varsCount] = malloc((strlen(fldName) + 1) * sizeof(SpiDriver_FldName_t));
                        strcpy(fldsList[*varsCount], fldName);
                    } else {
                        fldsList[*varsCount] = NULL;
                    }
                    (*varsCount)++;
                } else if (res == SPI_DRV_FUNC_RES_FAIL_INPUT_CFG) {
                    /* No worries. Just skip the line */
                    res = SPI_DRV_FUNC_RES_OK;
                } else {
                    API_PRINT("Variables file read error, line %u", *varsCount + 1);
                    break;
                }
            }
            fclose(fp);
            /* add a NULL at the end, marking end-of-sequence */
            varsList = realloc(varsList, (*varsCount + 1) * sizeof(SpiDriver_FldName_t*));
            varsList[*varsCount] = NULL;
            fldsList = realloc(fldsList, (*varsCount + 1) * sizeof(SpiDriver_FldName_t*));
            fldsList[*varsCount] = NULL;
            (*varsCount)++;
        } else {
            res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
            fprintf(stderr, "Error: Cannot open file [%s] for reading\n", varsFilename);
        }
        *varsListOut = varsList;
        *fldsListOut = fldsList;
    } else {
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
        fprintf(stderr, "Error: Read variables from file : filename is not assigned\n");
    };
    return res;
}


FuncResult_e spiDriver_SetupMultiICs(const char** icNames, const uint16_t icNumber)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
    uint16_t num;
    if (icNames != NULL) {
        if (icNumber > MAX_IC_ID_NUMBER) {
            fprintf(stderr,
                    "ICs number cannot be more than %u [%u is requested]. So, the ICs list is reduced\n",
                    MAX_IC_ID_NUMBER,
                    icNumber);
            num = MAX_IC_ID_NUMBER;
        } else {
            num = icNumber;
            res = SPI_DRV_FUNC_RES_OK;
        }
        memcpy(icIntNames, icNames, num * sizeof(icNames[0u]));
        icIntNamesNumber = num;
    } else {
        fprintf(stderr,
                "Error: IC IDs list shouldn't be empty, it should contain an array of IDs even all of them are empty or NULL\nError: IC list is not set\n");
    }
    return res;
}


static spiDriverCommand_e getScriptCmdId(const char* const commandStr)
{
    uint16_t res = 0xFFFFu;
    for (uint16_t i = 0u; i < SUPPORTED_SCRIPT_COMMANDS; i++) {
        if (strcmp(commandStr, spiDriverScriptCommandStrings[i]) == 0) {
            res = i;
            break;
        }
    }
    return res;
}


uint16_t getIcId(const char* const icIdStr)
{
    uint16_t res = 0xFFFFu;
    if (icIdStr[0] == '*') {
        res = IC_ID_BROADCAST;
    } else {
        for (uint16_t i = 0u; i < icIntNamesNumber; i++) {
            if (strcmp(icIdStr, icIntNames[i]) == 0) {
                res = i;
                break;
            }
        }
    }
    return res;
}


/** Parses the rest of line when it's not known */
__attribute__((weak)) FuncResult_e spiDrvTableDefaultCase(const char* const lineString)
{
    API_PRINT("\nCommand is not supported: %s \n ", lineString);
    (void)lineString; /* Assure we are "using" an input variable */
    return SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
}

/** Decides what to do if the variable to write is not found */
__attribute__((weak)) FuncResult_e spiDrvSetDefaultCase(const uint16_t icItId,
                                                        SpiDriver_FldName_t* varName,
                                                        const uint32_t value)
{
    API_PRINT("Variable is not found: %s \n ", varName);
    (void)icItId; /* Assure we use it */
    (void)varName;
    (void)value;
    return SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
}

/** Decides what to do if the variable to write is not found */
__attribute__((weak)) FuncResult_e spiDrvGetDefaultCase(const uint16_t icItId,
                                                        SpiDriver_FldName_t* varName,
                                                        uint32_t* value)
{
    API_PRINT("Variable is not found: %s \n ", varName);
    (void)icItId; /* Assure we use it */
    (void)varName;
    (void)value;
    return SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;
}

/** Executes the command from the text line
 * Uses the format:
 *      "<cmd> <IC> <name> <delimiter> [<fieldname><delimiter>] <value>".
 * The function strips the spaces and tabs for var_name on both sides,
 * considering them as 'delimiter' field. It also discards empty lines, and lines with "#" sign at the end of line.
 * The function also supports bit-fields assignment.
 * @param[in]   lineString      Input (source) string
 * @retval      SPI_DRV_FUNC_RES_OK successfull data parse
 * @retval      SPI_DRV_FUNC_RES_FAIL_INPUT_CFG the data cannot be assigned (but it shouldn't be treated as an error, just skip the line)
 * @retval      SPI_DRV_FUNC_RES_FAIL_INPUT_DATA something wrong is with the data
 */
static FuncResult_e spiDriverRunCmdFromString(const char* const lineString)
{
    char valBuf[256];
    char* pvalLine;
    uint32_t resValue;
    uint16_t lenBuf;
    spiDriverCommand_e cmd_id = SPI_DRV_CMD_NONE;
    uint16_t icId = 0u;
    uint16_t icItId;
    SpiDriver_FldName_t varName[MAX_FLD_NAME];
    SpiDriver_FldName_t fldName[MAX_FLD_NAME];

    char* delPos;
    uint16_t lenStr;
    bool doExit = false;
    uint16_t fmt_ind = 0u;
    FuncResult_e res = SPI_DRV_FUNC_RES_FAIL_INPUT_DATA;

    // Clean-up delimiters
    pvalLine = (char*)lineString;
    while ((*pvalLine != '\0') && (strchr(delimiters, *pvalLine) != NULL)) {
        pvalLine++;
    }
    // Strip comments:
    delPos = strchr(pvalLine, SPI_DRV_TEXT_DEFAULT_COMMENTS);
    if (delPos != NULL) {
        lenStr = ((long)delPos - (long)pvalLine) / sizeof(char);
    } else {
        lenStr = strlen(pvalLine);
    }
    if (lenStr > 3) {
        while (!doExit) {
            lenBuf = strncopyStripped(pvalLine, lenStr - 1, valBuf); /* Get the new field in a sequence */
            if (lenBuf > 0) {
                switch (fmt_ind) {
                    case 0: // Command
                        cmd_id = getScriptCmdId(valBuf);
                        API_PRINT("cmd:%d, ", cmd_id);
                        switch (cmd_id) {
                            case SPI_DRV_CMD_WRITE:              /**< Writes the variable, just continue. Parse further arguments */
                            case SPI_DRV_CMD_READ:               /**< Reads the variable to stdout. Parse further arguments */
                                break;

                            case SPI_DRV_CMD_NONE:               /**< No operation */
                                API_PRINT("No operation\n");
                                res = SPI_DRV_FUNC_RES_OK;
                                doExit = true;
                                break;

                            case SPI_DRV_CMD_SLEEP:              /**< Runs the sleep command */
                                lenBuf = strncopyStripped(pvalLine + lenBuf, lenStr - lenBuf - 1, valBuf);
                                if (readValue(valBuf, &resValue)) {
                                    API_PRINT("Pause for %u secs\n", resValue);
                                    sleep(resValue);
                                }
                                res = SPI_DRV_FUNC_RES_OK;
                                doExit = true;
                                break;

                            case SPI_DRV_CMD_IMPORT:              /**< Imports another script file */
                                lenBuf = strncopyStripped(pvalLine + lenBuf, lenStr - lenBuf - 1, valBuf);
                                API_PRINT("Importing the script file :%s\n", valBuf);
                                res = spiDriver_RunScript(valBuf);
                                doExit = true;
                                break;

                            default:
                                res = spiDrvTableDefaultCase(pvalLine);
                                doExit = true; // Command is not recognized or wrong
                        }
                        break;

                    case 1: // IC ID
                        icId = getIcId(valBuf);
                        API_PRINT("IC:%s(ind:%d)", valBuf, icId);
                        break;

                    case 2: // Variable name
                        API_PRINT("Varname: %s, ", valBuf);
                        strcpy(varName, valBuf);
                        break;

                    case 3: // Field-name / value
                        icItId = icId;
                        if (cmd_id == SPI_DRV_CMD_WRITE) {
                            if (readValue(valBuf, &resValue)) {
                                API_PRINT("Value: %u\n", resValue);
                                *fldName = '\0'; // Erase the bit-field name
                                res = spiDriver_SetMultiByName(icItId, varName, resValue, fldName);
                                if (res == SPI_DRV_FUNC_RES_OK) {
                                    API_PRINT("%s: write %s <== %04x\n", icIntNames[icItId], varName, resValue);
                                } else if (res == SPI_DRV_FUNC_RES_FAIL_INPUT_DATA) {
                                    res = spiDrvSetDefaultCase(icItId, varName, resValue);
                                } else {
                                    fprintf(stderr, "Error writing %s\n", varName);
                                }
                                doExit = true;
                            } else {
                                /* Try to treat the value field as a bitfield and go further */
                                strcpy(fldName, valBuf);
                                API_PRINT("FldName: %s, ", fldName);
                            }
                        } else if (cmd_id == SPI_DRV_CMD_READ) {
                            if (icItId < icIntNamesNumber) {
                                if (lenBuf > 0u) {
                                    strcpy(fldName, valBuf);
                                    API_PRINT("FldName: %s, ", fldName);
                                    res = spiCom_SetDev(icItId);
                                    res |= spiDriver_GetByName(varName, &resValue, fldName);
                                    if (res == SPI_DRV_FUNC_RES_OK) {
                                        printf("%s %s %s %04x\n", icIntNames[icItId], varName, fldName, resValue);
                                    } else if (res == SPI_DRV_FUNC_RES_FAIL_INPUT_DATA) {
                                        res = spiDrvGetDefaultCase(icItId, varName, &resValue);
                                        if (res == SPI_DRV_FUNC_RES_OK) {
                                            printf("%s %s %04x\n", icIntNames[icItId], varName, resValue);
                                        } else {
                                            printf("Error reading %s\n", varName);
                                        }
                                    } else {
                                        fprintf(stderr, "Error reading %s %s\n", varName, fldName);
                                    }
                                } else {
                                    res = spiCom_SetDev(icItId);
                                    res |= spiDriver_GetByName(varName, &resValue, NULL);
                                    if (res == SPI_DRV_FUNC_RES_OK) {
                                        printf("%s %s %04x\n", icIntNames[icItId], varName, resValue);
                                    } else {
                                        fprintf(stderr, "Error reading %s\n", varName);
                                    }
                                }
                            } else {
                                fprintf(stderr, "Error: IC ID is not defined or not able to be read\n");
                            }
                            doExit = true;
                        }
                        break;

                    case 4: // Value-only (when there's field-name)
                        icItId = icId;
                        if (readValue(valBuf, &resValue)) {
                            API_PRINT("Value: %u\n", resValue);
                            res = spiDriver_SetMultiByName(icId, varName, resValue, fldName);
                            if (res == SPI_DRV_FUNC_RES_OK) {
                                API_PRINT("%s: write %s %s <== %04x\n", icIntNames[icItId], varName, fldName, resValue);
                            } else {
                                fprintf(stderr, "Error writing %s %s\n", varName, fldName);
                            }
                            doExit = true;
                        } else {
                            doExit = true;
                            strcpy(fldName, valBuf);
                            fprintf(stderr, "Error in value\n");
                        }
                        break;
                }
                lenStr -= lenBuf;
                pvalLine += lenBuf;
                fmt_ind++;
            } else {
                fprintf(stderr, "Error: Not enough fields in command: in line field:\n%s\n", lineString);
            }
        }
    } else {
        // Skipping the line
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
    }
    return res;
}


FuncResult_e spiDriver_RunScript(const char* const scriptFilename)
{
    FILE* fp;
    char line[256];
    uint16_t cur_line_num = 0u;
    FuncResult_e res;
    if (scriptFilename != NULL) {
        fp = fopen(scriptFilename, "r");
        if (fp != NULL) {
            res = SPI_DRV_FUNC_RES_OK;
            API_PRINT("Run script from file %s\n", scriptFilename);
            while ((fgets(line, sizeof(line), fp) != NULL) && (res == SPI_DRV_FUNC_RES_OK)) {
                res = spiDriverRunCmdFromString(line);
                if (res == SPI_DRV_FUNC_RES_OK) { /* Skip positive command execution */
                    cur_line_num++;
                } else if (res == SPI_DRV_FUNC_RES_FAIL_INPUT_CFG) {
                    /* No worries. Just skip the line */
                    res = SPI_DRV_FUNC_RES_OK;
                } else {
                    fprintf(stderr, "Variables file [%s] read error, line %u\n", scriptFilename, cur_line_num + 1);
                    break;
                }
            }
            fclose(fp);
        } else {
            res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
            fprintf(stderr, "Error: Cannot open file [%s] for reading\n", scriptFilename);
        }
    } else {
        res = SPI_DRV_FUNC_RES_FAIL_INPUT_CFG;
        fprintf(stderr, "Error: run script from file : filename is not assigned\n");
    };
    return res;
}


FuncResult_e spiDriver_GoStandBy(void)
{
    return spiCom_SensorStandby();
}


void spiDriver_FreeVariableNamesArray(SpiDriver_FldName_t** varsList,
                                      SpiDriver_FldName_t** fldsList,
                                      const uint16_t varsCount)
{
    for (uint16_t valInd = 0u; valInd < varsCount; valInd++) {
        free(varsList[valInd]);
        free(fldsList[valInd]);
    }
    free(varsList);
    free(fldsList);
}

