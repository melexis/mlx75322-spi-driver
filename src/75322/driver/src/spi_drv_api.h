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
 * @ingroup spi_driver
 * @defgroup spi_api SPI protocol API
 *
 * @details
 *
 * This component provides the common functions set, which combines set of read-write variables' calls and the API
 *     for getting/setting IC variables, with using the **spi_data** and **spi_com** functions. This component is
 *     intended to be used from the application.
 *
 * @defgroup spi_api_init SPI driver Initialization
 * @defgroup spi_drv_variables Writing and reading data variables group
 * @defgroup spi_drv_tables Writing and reading data tables group
 * @defgroup spi_com SPI protocol communication layer
 * @defgroup spi_drv_misc_cmds Miscellaneous commands
 * @defgroup spi_data SPI driver Database component
 *
 */

#ifndef SPI_DRV_API_H
#define SPI_DRV_API_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "spi_drv_common_types.h"
#include "spi_drv_data.h"
#include "spi_drv_com.h"
#include "spi_drv_sync_mode.h"

#ifndef API_DEBUG
#define API_DEBUG 0
#endif /* TRACE_DEBUG */

#if (API_DEBUG == 1)
#define API_PRINT printf
#else
#define API_PRINT(...)
#endif

/**
 * @ingroup spi_api
 * @addtogroup spi_drv_tables
 *
 * @details
 *
 * To handle the grouped data reading and setting, this group provides a set of functions
 * for reading and writing external files of variables' values.
 *
 * @{
 */

extern uint16_t icIntNamesNumber;

/** Default text-files (like variable's list/values) delimiter. This set should not have > 15 characters */
#define SPI_DRV_TEXT_DEFAULT_DELIMITERS " \t\n\r"
/** Default text-files (like variable's list/values) character which discards end of string after it */
#define SPI_DRV_TEXT_DEFAULT_COMMENTS '#'
/** Default output files format delimiter */
#define SPI_DRV_TEXT_DEFAULT_DELIMITER ','
/** The maximum number of ICs handled by the driver */
#define MAX_IC_ID_NUMBER 16
/** The IC's id used to handle the command as a broadcast message */
#define IC_ID_BROADCAST MAX_IC_ID_NUMBER

/** @}*/

/**
 * @ingroup spi_api
 * @addtogroup spi_api_init
 *
 * @details
 *
 * During the configuration phase, the driver initiates the HW communication layer
 * and reads the external variables' configuration, allowing to use them as a
 * database of variables and some fields within this variables if applicable. The
 * initialization should always be passed successfully before any function called from
 * SPI driver API.
 *
 * Additionally, an initialization procedure might need to upload the patch into the IC.
 * This function is also implemented in initialization phase.
 * @{
 */

/** SPI driver input configuration structure */
typedef struct {
    SpiComConfig_t* spiComCfg;      /**< Pointer to communication-level configuration */
    char* fwFileName;   /**< The filename of variables' set configuration */
    char* patchFileName;/**< The filename of patch. Can be omitted by "" or NULL pointer */
    char* scriptFileName;/**< The filename of script for init. Can be omitted by "" or NULL pointer */
    char* configFileName;/**< The filename of configuration. Can be omitted by "" or NULL pointer */
} spiDriver_InputConfiguration_t;


#define SUPPORTED_SCRIPT_COMMANDS 5


/** An Array of string commands reresentation to match with script's data */
extern const char* spiDriverScriptCommandStrings[SUPPORTED_SCRIPT_COMMANDS];

/** Specifies scripts commands available.
 * The similar list of text array should match this enumeration. Refer to ::spiDriverScriptCommandStrings for details */
typedef enum {
    SPI_DRV_CMD_NONE = 0,           /**< No operation */
    SPI_DRV_CMD_WRITE,              /**< Writes the variable */
    SPI_DRV_CMD_READ,               /**< Reads the variable to stdout */
    SPI_DRV_CMD_SLEEP,              /**< Runs the sleep command */
    SPI_DRV_CMD_IMPORT              /**< Imports another script file */
} spiDriverCommand_e;


/** Inits the driver with an input data and runs the initialization on all driver's layers
 * This function also runs the patch applying after initialization and sends the configuration set from a file.
 *
 * @param[in]   spiDriver_InputCfg a pointer for driver's configuration
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input variable name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL               Low-level communication operation had failed
 */
spiDriver_Status_t spiDriver_Initialize(const spiDriver_InputConfiguration_t* const spiDriver_InputCfg);

/** @} */

/**
 * @ingroup spi_api
 * @addtogroup spi_drv_variables
 *
 * @details
 *
 * This group of functions allows to read and write the IC's variables and their fields if they exist.
 * Variable's set is defined during the initialization while the rest is done through the driver's communication layer.
 *
 * @{
 */

/** Sets the variable by variable name via its offset
 * @param[in]   varName     variable's name
 * @param[in]   value       value to set
 * @param[in]   bitFieldName specifies the bit-fieldname, if such option is avaialable for the
 *      variable with varName. Can be omitted by setting to an empty string or NULL
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input variable name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL               Low-level communication operation had failed
 */
FuncResult_e spiDriver_SetByName(const SpiDriver_FldName_t* const varName,
                                 uint32_t value,
                                 const SpiDriver_FldName_t* const bitFieldName);


/** Gets the variable by variable name via its offset
 * @param[in]   varName     variable's name
 * @param[in]   value       32-bit value's buffer to read and store the data.
 *                          @warning this pointer is not checked for NULL in favor of code efficiency.
 * @param[in]   bitFieldName specifies the bit-fieldname, if such option is avaialable for the
 *      variable with varName. Can be omitted by setting to an empty string or NULL
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input variable name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL               Low-level communication operation had failed
 */
FuncResult_e spiDriver_GetByName(const SpiDriver_FldName_t* const varName,
                                 uint32_t* const value,
                                 const SpiDriver_FldName_t* const bitFieldName);

/** @} */

/**
 * @ingroup spi_api
 * @addtogroup spi_api_init
 * @{
 */

/** Loads the patch into the chip
 * @param[in]   patchFileName     Intel-HEX file containing patch
 * @return      loading patch result
 */
spiDriver_Status_t spiDriver_LoadPatch(const char* const patchFileName);

/** @}*/

/**
 * @ingroup spi_api
 * @addtogroup spi_drv_variables
 * @{
 */

/** Sets the delimiters' list for variable tables
 * The input data is buffered and shrinked to 15 characters
 * @param[in]   tableDelimiters     Characters' string containing a list of delimiters between values.
 *                                  Any char from this list will be recognized as "space".
 *                                  Use ::SPI_DRV_TEXT_DEFAULT_DELIMITERS as the default value
 */
void spiDriver_SetDelimiter(const char* const tableDelimiters);

/** Reads variables from the IC
 * @param[in]   valuesBuffer        input variables value buffer for values read
 * @param[in]   varsList            array of strings with variable names to read. If NULL - variables from the internal database are used
 * @param[in]   fldsList            array of strings with variable field names to read. If NULL - the fields are not used.
 * @param[in]   varsNumber          number of variables to read
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input variable name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL               Low-level communication operation had failed
 */
FuncResult_e spiDriver_ReadVariables(uint32_t* valuesBuffer,
                                     SpiDriver_FldName_t** varsList,
                                     SpiDriver_FldName_t** fldsList,
                                     uint16_t varsNumber);

/** Write variables into the IC
 * @param[in]   valuesBuffer        the variables values buffer to write
 * @param[in]   varsList            array of strings with variable names to write. If NULL - variables from the internal database are used.
 * @param[in]   fldsList            array of strings with variable field names to write. If NULL - the fields are not used.
 * @param[in]   varsNumber          number of variables to write
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input variable name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL               Low-level communication operation had failed
 */
FuncResult_e spiDriver_WriteVariables(uint32_t* valuesBuffer,
                                      SpiDriver_FldName_t** varsList,
                                      SpiDriver_FldName_t** fldsList,
                                      uint16_t varsNumber);

/** Sets up the ICs names used by tables' and scripts functions
 * @param[in]   icNames     the list of IC names. Each IC name will be assigned to the ID of it's position
 * @param[in]   icNumber    the Names number. The max is ::MAX_IC_ID_NUMBER
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input parameter is not acceptable
 */
FuncResult_e spiDriver_SetupMultiICs(const char** icNames, const uint16_t icNumber);


/** @} */

/**
 * @ingroup spi_api
 * @addtogroup spi_drv_tables
 * @{
 */

/** Writes the variables from the file into the IC
 * @param[in]   varsFilename        Input filename to read the data from and write it into the variables. The file has per-line delimited text format.
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input file name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_DATA    input file's content is wrong and/or variable mentioned was not found
 * @retval  SPI_DRV_FUNC_RES_FAIL               Low-level communication operation had failed
 */
FuncResult_e spiDriver_WriteVariablesFromFile(const char* const varsFilename);

/** Reads the variable names from the file and provides their list
 * @param[in]   varsFilename        Input filename to read the data from and write it into the variables. The file has per-line delimited text format.
 * @param[out]  varsListOut         array of strings with variable names to write.
 *                                  If NULL - variables from the internal databasea are used.
 *                                  The latest name will be "" or NULL pointer (to a string).
 * @param[out]  fldsListOut         array of strings with variable field names to write.
 *                                  If NULL - variable fields are not used.
 *                                  The latest name will be "" or NULL pointer (to a string).
 * @param[out]  varsCount           The number of variables read from the file.
 *
 * @note    This functions allocates memory for holding the string variables and their names. Thus, this memory should be erased after used by
 *          function spiDriver_FreeNamesList();
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input file name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_DATA    input file's content is wrong and/or variable mentioned was not found
 * @retval  SPI_DRV_FUNC_RES_FAIL               Low-level communication operation had failed
 */
FuncResult_e spiDriver_ReadVariableNamesFromFile(const char* const varsFilename,
                                                 SpiDriver_FldName_t*** varsListOut,
                                                 SpiDriver_FldName_t*** fldsListOut,
                                                 uint16_t* varsCount);

/** Frees-out allocated array of strings (names)
 * @param[out]  varsList            array of strings with variable names to be freed.
 * @param[out]  fldsList            array of strings with field names to be freed.
 * @param[in]  varsCount            The number of variables in a list.
 *
 */
void spiDriver_FreeVariableNamesArray(SpiDriver_FldName_t** varsList,
                                      SpiDriver_FldName_t** fldsList,
                                      const uint16_t varsCount);

/** Reads the variables from the IC a writes them into the external file
 * @param[in]   varsFilename        Input filename to read the data from and write it into the variables. The file has per-line delimited text format.
 * @param[in]   delimiter           Parameter's delimiter in the text file used. 'SPI_DRV_TEXT_DEFAULT_DELIMITER' can be used
 * @param[in]   varsList            array of strings with variable names to write.
 *                                  If NULL - variables from the internal databasea are used.
 *                                  The latest name should be "" or NULL pointer (to a string).
 * @param[in]   fldsList            array of strings with variable field names to write.
 *                                  If NULL - no fields will be used. The items should be NULL is no field name should be used.
 *                                  The number of items in an array should exact the number of items in 'varsList'
 *                                  The latest name should be "" or NULL pointer (to a string).
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_CFG     input file name is not found
 * @retval  SPI_DRV_FUNC_RES_FAIL_INPUT_DATA    input file's content is wrong and/or variable mentioned was not found
 * @retval  SPI_DRV_FUNC_RES_FAIL               Low-level communication operation had failed
 */
FuncResult_e spiDriver_ReadVariablesIntoFile(const char* const varsFilename,
                                             char delimiter,
                                             SpiDriver_FldName_t** varsList,
                                             SpiDriver_FldName_t** fldsList);


/** Parses commands from a file
 * This function accepts the filename of commands script, parses it and sends commands to ICs, according their names
 * Each line has the following format accepted:
 *      ["<command> <ic_id> <variable> [<field>] <value>" ] # comment
 * Refer to ::spiDriverCommand_e for commands explanation
 * "ic_id" is the IC's id representation from the list of ID pre-configured be function ::spiDriver_SetupMultiICs(). "ic_id" field
 * can have a broadcast name "*". It allows to send command for all ICs in a list sequentially at once.
 *
 */
FuncResult_e spiDriver_RunScript(const char* const scriptFilename);

/** @}*/

/**
 * @ingroup spi_api
 * @addtogroup spi_drv_misc_cmds
 *
 * @{
 */

/** Sends the StandBy command to IC
 * @retval  SPI_DRV_FUNC_RES_OK                 operation is successful
 * @retval  SPI_DRV_FUNC_RES_FAIL_COMM          wrong communication
 */
FuncResult_e spiDriver_GoStandBy(void);

/** @}*/


#ifdef __cplusplus
}
#endif

#endif /* SPI_DRV_API_H */

