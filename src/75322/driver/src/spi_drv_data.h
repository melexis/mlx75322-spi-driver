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
 * @addtogroup spi_data
 * @ingroup spi_api
 *
 * @details SPI driver database, to work with. Provides a set of functions allowing to work with an IC's
 *     configuration with variable names. Allows to read/write the configurations from/to files and upload the patch
 *     into the IC. These functions are mainly used from higher levels of SPI driver. So, its usage should have some
 *     special case and an additional care about it.
 *
 */

#ifndef SPI_DRV_DATA_H
#define SPI_DRV_DATA_H

/** @{*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

/** MAX_FLD_NAME specifies the buffer's size for holding the field's name in a structure */
#define MAX_FLD_NAME 64
/** MAX_BLCK_NAME specifies the buffer's size for holding the HW block's name in a structure */
#define MAX_BLCK_NAME 16
/** MAX_RESET_NAME specifies the buffer's size for holding the HW field's [reset] info in a structure */
#define MAX_RESET_NAME 8
/** MAX_DESC_NAME specifies the buffer's size for holding the HW field's description info in a structure */
#define MAX_DESC_NAME 128

/** Specifies the structure of FW's field information */
typedef struct FwFieldInfo_s {
    uint32_t fldNameHash;           /**< full_hash for Field's name */
    char fldName[MAX_FLD_NAME];     /**< Field's name */
    uint16_t fldAddr;               /**< Field's address */
    bool bitField;                  /**< determines whether the field is a bit-field (flag). Its bitSize=1 when "true" */
    uint8_t bitOffset;              /**< Data's bit offset within the port */
    uint8_t bitSize;                /**< Data's bitwise width */
    uint8_t byteSize;               /**< Data's bytes size */
    bool isSigned;                  /**< Data is used as a signed value */
    uint8_t wordSize;               /**< Data's word size */
    uint16_t offset;                /**< Offset */
    uint8_t bitFieldCount;          /**< Number of nested bit-fields */
    struct FwFieldInfo_s* bitFields;       /**< bit-fields included into the field */
} FwFieldInfo_t;

extern FwFieldInfo_t* fwFields;
extern uint16_t fwFieldsCount;

/** Allocates an instance of fwFields[] array and reads the FW fields from a file specified by "f_name"
 * @param[in]   f_name      Database's file name
 * @return      result of an operation
 */
FuncResult_e ReadFwJson(const char* const f_name);

/** Returns the FW variable by its name
 * @param[in]   var_name        Variable's name
 * @return      a pointer to a FW variable's structure. Returns NULL if variable was not found
 */
FwFieldInfo_t* GetFwVariableByName(const SpiDriver_FldName_t* const var_name);

/** Returns the FW variable by its offset
 * @param[in]   offset         Variable's offset
 * @return      a pointer to a FW variable's structure. Returns NULL if variable was not found
 */
FwFieldInfo_t* GetFwVariableByOffset(const uint16_t offset);

/** Returns the FW variable bit-field withing the variable
 * @param[in]   fwField         Pointer to a field's description
 * @param[in]   field_name      Bit-field name
 * @return      a pointer to a FW variable's bit-field structure. Returns NULL if bit-field was not found
 */
FwFieldInfo_t* GetFwBitFieldByName(const FwFieldInfo_t* const fwField, const SpiDriver_FldName_t* const field_name);

/** Gets the bitField's boolean value, with MSB data direction expected (and real LSB placement)
 * The function detects the size of data (8, 16, 32 bits wide)
 * @param[in]   value           The initial value (expected 32bit wide but support any kind <=32bit width)
 * @param[in]   bitOffset       Bit offset, MSB (0[MSB],1,2,3, ... ,14,15[LSB]). Actual bit is shown in "[]"
 * @param[in]   bitSize         Bitfield width in bits
 * @param[in]   byteSize        Field's width in bytes
 * @return      masked input value
 */
uint32_t spiDriver_GetBit(const uint32_t value, const uint8_t bitOffset, const uint8_t bitSize, const uint8_t byteSize);

/** Gets the bitField's boolean value, with MSB data direction expected (and real LSB placement)
 * The function detects the size of data (8, 16, 32 bits wide)
 * @param[in]   value           The initial value (expected 32bit wide but support any kind <=32bit width)
 * @param[in]   fwField         Pointer to a field's description
 * @return      masked input value
 */
uint32_t spiDriver_GetBitByVar(const FwFieldInfo_t* fwField, const uint32_t value);

/** Sets the bitField's value, with MSB data direction expected (and real LSB placement)
 * @param[in]   value           MSB value
 * @param[in]   newValue        New Field's value in MSB format
 * @param[in]   bitOffset       Bit offset, MSB (0[MSB],1,2,3, ... ,14,15[LSB]). Actual bit is shown in "[]"
 * @param[in]   bitSize         Bitfield width in bits
 * @param[in]   byteSize        Field's width in bytes
 * @return      updated value
 */
uint16_t spiDriver_SetBit(const uint32_t value,
                          const uint32_t newValue,
                          const uint8_t bitOffset,
                          const uint8_t bitSize,
                          const uint8_t byteSize);

/** Sets the bitField's value, with MSB data direction expected (and real LSB placement)
 * @param[in]   value           MSB value
 * @param[in]   newValue        New Field's value in MSB format
 * @param[in]   fwField         Pointer to a field's description
 * @return      updated value
 */
uint32_t spiDriver_SetBitByVar(const FwFieldInfo_t* fwField, const uint32_t value, const uint32_t newValue);

/** Gets the byte-oriented data from the variables base, by the field's instance pointer
 * @param[in]   fwField         FW database field's structure
 * @param[in]   bufValue        The data to use as a byte-oriented
 * @return      result of an operation
 */
uint32_t spiDriver_GetByteByVar(const FwFieldInfo_t* fwField, const uint32_t bufValue);

/** Sets the byte-oriented data to the variables base, by the field's instance pointer
 * @param[in]   fwField         FW database field's structure
 * @param[in]   bufValue        The current data content in a word-oriented instance
 * @param[in]   fieldValue      The field's byte-value to write
 * @return      result of an operation
 */
uint32_t spiDriver_SetByteByVar(const FwFieldInfo_t* fwField, const uint32_t bufValue, const uint32_t fieldValue);

/** Gets the byte-oriented data from the variables name, by its name
 *
 * @note        This function checks the field's presence and throws an exception if the field does not exist
 *
 * @param[in]   varName         Field's name
 * @param[in]   bufValue        The data to use as a byte-oriented
 * @return      result of an operation
 */
uint16_t spiDriver_GetByteByName(const SpiDriver_FldName_t* const varName, const uint16_t bufValue);

/** Sets the byte-oriented data to the variables base, by its name
 *
 * @note        This function checks the field's presence and throws an exception if the field does not exist
 *
 * @param[in]   varName         Field's name
 * @param[in]   bufValue        The current data content in a word-oriented instance
 * @param[in]   fieldValue      The field's byte-value to write
 * @return      result of an operation
 */
uint16_t spiDriver_SetByteByName(const SpiDriver_FldName_t* const varName,
                                 const uint16_t bufValue,
                                 const uint16_t fieldValue);

/** Return the word-oriented data address
 * @param[in]  address         Input address
 * @return      word-aligned address
 */
uint16_t spiDriver_CalcAddress(const uint16_t address);

#ifdef __cplusplus
}
#endif

/** @}*/

#endif /* SPI_DRV_DATA_H */

