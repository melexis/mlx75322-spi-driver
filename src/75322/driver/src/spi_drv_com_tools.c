/**
 * @file
 * @brief Internal functions for low-level SPI Comm driver operations
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
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "spi_drv_common_types.h"
#include "spi_drv_com_tools.h"
#include "spi_drv_tools.h"
#include "spi_drv_hal_spidev.h"

/* ---------------- Variables ---------------- */

extern DiagDetailsPkt_t diagDetails[2];         /* Global Variable, can be accessed by high level driver */
extern const uint16_t crcTable [];
extern uint16_t pktWords [MAX_PKT_WORDS];
extern uint16_t payload [MAX_PKT_PAYLOAD_WORDS];

/* ---------------- Functions ---------------- */

void clearDiagDetails(void)
{
    diagDetails[0].halStat = 0;
    diagDetails[0].comStat = 0;
    diagDetails[0].devStat = 0;
    diagDetails[1].halStat = 0;
    diagDetails[1].comStat = 0;
    diagDetails[1].devStat = 0;
}



uint16_t calcCrc(uint16_t wordSizeOfCrcEnvelope, uint16_t* wordBuf)
{
    uint16_t crc = 0x0000;
    uint8_t byteHi;
    uint8_t byteLo;
    uint8_t idx;

    for ( uint16_t ww = 0; ww < wordSizeOfCrcEnvelope; ww++) {
        byteHi = (uint8_t)( wordBuf[ww] >> 8         );
        byteLo = (uint8_t)( wordBuf[ww] & 0x00FF     );

        idx = ( (uint8_t)( crc >> 8) ^ byteHi         ) & 0xFF;
        crc = (          ( crc << 8) ^ crcTable[idx] ) & 0xFFFF;
        idx = ( (uint8_t)( crc >> 8) ^ byteLo         ) & 0xFF;
        crc = (          ( crc << 8) ^ crcTable[idx] ) & 0xFFFF;
    }
    return crc;
}



#if (COM_DEBUG_DETAIL_1 == 1) || \
    (COM_DEBUG_DETAIL_2 == 1)
void printPtype(uint16_t ptype)
{
    switch (ptype) {
        case 0:  fprintf(comDebugFile, "%-20s", "READ");
            break;
        case 1:  fprintf(comDebugFile, "%-20s", "WRITE");
            break;
        case 2:  fprintf(comDebugFile, "%-20s", "FUNCTION");
            break;
        case 4:  fprintf(comDebugFile, "%-20s", "STATUS_SHORT");
            break;
        case 5:  fprintf(comDebugFile, "%-20s", "STATUS_LONG");
            break;
        case 6:  fprintf(comDebugFile, "%-20s", "READ_DATA_RESP_SHORT");
            break;
        case 7:  fprintf(comDebugFile, "%-20s", "READ_DATA_RESP_LONG");
            break;
        case 8:  fprintf(comDebugFile, "%-20s", "WRITE_DATA_LONG");
            break;
        case 9:  fprintf(comDebugFile, "%-20s", "ENTER_KEY");
            break;
        case 10: fprintf(comDebugFile, "%-20s", "READ_DIRECT");
            break;
        case 11: fprintf(comDebugFile, "%-20s", "WRITE_DIRECT");
            break;
        case 12: fprintf(comDebugFile, "%-20s", "ECHO_DATA_RESP");
            break;
        case 13: fprintf(comDebugFile, "%-20s", "RAW_DATA_RESP");
            break;
        case 14: fprintf(comDebugFile, "%-20s", "SYNC");
            break;
        case 15: fprintf(comDebugFile, "%-20s", "WRITE_PATCH");
            break;
    }
}
#endif



FuncResult_e makeSpiPacket(uint16_t ptype,  uint16_t sizeField,  uint16_t sizePayload, uint16_t* payload)
{
    uint16_t crc;

    if ( (ptype == STATUS_SHORT) || (ptype == STATUS_LONG) ) { /* STATUS type packets have size field set to 0, */
        pktWords[0] = ptype << 12;                            /* even though physical size is not 0.           */
    } else {
        pktWords[0] = (ptype << 12) | sizeField;
    }

    memcpy(&pktWords[1], payload, sizePayload * sizeof(uint16_t));   /* copy payload to pktWords */

    crc = calcCrc(1 + sizePayload, pktWords);               /* CRC calculated over 1 header word + size of payload words */
    pktWords[sizePayload + 1] = crc;

#if (COM_DEBUG_DETAIL_1 == 1) || \
    (COM_DEBUG_DETAIL_2 == 1)
    fprintf(comDebugFile, "MOSI: <");
    printPtype(ptype);
    fprintf(comDebugFile, "> <%3d> (%3d) <0x%04X_%04X> <0x%04X>\n",
            sizeField, sizePayload, payload[0], payload[1], crc);
#endif

#if (COM_DEBUG_DETAIL_2 == 1)
    for (uint16_t ww = 0; ww < (sizePayload + 2); ww++ ) {            /* For debug: loop over the whole packet, print each word */
        fprintf(comDebugFile, "pktWords{%d} = 0x%04X\n", ww, pktWords[ww]);
    }
    fprintf(comDebugFile, "\n");
#endif

    return SPI_DRV_FUNC_RES_OK;
}



FuncResult_e validatePkt(uint8_t pktNum, uint16_t expectedPtype, uint16_t xactSize, uint16_t* validateBuf)
{

    uint16_t statusValue = 0;
    FuncResult_e res;
    uint16_t valBufPtype;
    uint16_t valBufSizeField;
    uint16_t valBufCrc;
    uint16_t calculatedCrc;
    uint16_t wordSizeOfCrcEnvelope;
    uint16_t expectedSizeField;
    uint16_t expectedLengthOfBuf;

    /* Extract fields from the packet */
    valBufPtype = validateBuf[0] >> 12;                     /* first 4 bits of the packet */
    valBufSizeField = validateBuf[0] & 0x0FFF;             /* next 12 bits of the packet */

    if ( (expectedPtype == STATUS_SHORT) || (expectedPtype == STATUS_LONG) ) {
        expectedSizeField = 0;
        diagDetails[pktNum].devStat = ( ((uint32_t)validateBuf[1] << 16) | (uint32_t)validateBuf[2] );
    } else {
        expectedSizeField = xactSize;
    }

    if (    ( expectedPtype == READ)
            || ( expectedPtype == WRITE)
            || ( expectedPtype == FUNCTION)
            || ( expectedPtype == STATUS_SHORT)
            || ( expectedPtype == READ_DATA_RESP_SHORT)
            || ( expectedPtype == 9)
            || ( expectedPtype == 10)
            || ( expectedPtype == 11)
            || ( expectedPtype == SYNC)
            || ( expectedPtype == WRITE_PATCH)
            ) {
        expectedLengthOfBuf = 4;
    } else {
        expectedLengthOfBuf = xactSize + 2;
    }

    valBufCrc = validateBuf[expectedLengthOfBuf - 1];        /* last word of the packet */
    /* Calc CRC based on received packet header and payload */
    wordSizeOfCrcEnvelope = expectedLengthOfBuf - 1;    /* add 1 to include header, but leave out CRC */
    calculatedCrc = calcCrc(wordSizeOfCrcEnvelope, validateBuf);

#if (COM_DEBUG_DETAIL_1 == 1) || \
    (COM_DEBUG_DETAIL_2 == 1)
    fprintf(comDebugFile, "MISO: <");
    printPtype(valBufPtype);
    fprintf(comDebugFile, "> <%3d> (%3d) <0x%04X_%04X> <0x%04X>\n",
            valBufSizeField, xactSize, validateBuf[1], validateBuf[2], valBufCrc);
#endif


    if (valBufCrc != calculatedCrc) {
        statusValue |= 0x0001;
        printf("%s: ERROR: valBufCrc = 0x%04X, caclulcatedCrc = 0x%04X\n",
               __FUNCTION__,
               valBufCrc,
               calculatedCrc);
        diagDetails[pktNum].comStat = diagDetails[pktNum].comStat | CS_CRC;
    }

    if (valBufPtype != expectedPtype) {
        statusValue |= 0x0002;
        printf("%s: ERROR: valBufPtype = %0d, expectedPtype = %0d\n",
               __FUNCTION__,
               valBufPtype,
               expectedPtype);
        diagDetails[pktNum].comStat = diagDetails[pktNum].comStat | CS_TYPE;
    }

    if (valBufSizeField != expectedSizeField) {
        statusValue |= 0x0004;
        printf("%s: ERROR: valBufSizeField = %0d, expectedSizeField = %0d\n",
               __FUNCTION__,
               valBufSizeField,
               expectedSizeField);
        diagDetails[pktNum].comStat = diagDetails[pktNum].comStat | CS_SIZE;
    }

    if ( (diagDetails[pktNum].halStat < 0) || (diagDetails[pktNum].comStat != CS_SUCCESS) ||
         ( (diagDetails[pktNum].devStat & (~DEV_STAT_IGNORE_MASK) ) != 0 ) ) {
        statusValue |= 0x0008;
        printf("%s: ERROR(1of3): diagDetails{%0d}.halStat = 0x%08X\n", __FUNCTION__, pktNum,
               diagDetails[pktNum].halStat);
        printf("%s: ERROR(2of3): diagDetails{%0d}.comStat = 0x%08X\n", __FUNCTION__, pktNum,
               diagDetails[pktNum].comStat);
        printf("%s: ERROR(3of3): diagDetails{%0d}.devStat = 0x%08X\n", __FUNCTION__, pktNum,
               diagDetails[pktNum].devStat);
        COM_DEBUG_PRINT(comDebugFile,
                        "ERROR(1of3): diagDetails{%0d}.halStat = 0x%08X\n",
                        pktNum,
                        diagDetails[pktNum].halStat);
        COM_DEBUG_PRINT(comDebugFile,
                        "ERROR(2of3): diagDetails{%0d}.comStat = 0x%08X\n",
                        pktNum,
                        diagDetails[pktNum].comStat);
        COM_DEBUG_PRINT(comDebugFile,
                        "ERROR(3of3): diagDetails{%0d}.devStat = 0x%08X\n",
                        pktNum,
                        diagDetails[pktNum].devStat);
    }

    if (statusValue == 0) {
        res = SPI_DRV_FUNC_RES_OK;
    } else {
        res = SPI_DRV_FUNC_RES_FAIL_COMM;
    }

#if (COM_DEBUG_DETAIL_2 == 1)
    for ( uint16_t ww = 0; ww < (xactSize + 2); ww++ ) {            /* For debug: loop over the whole packet, print each word */
        fprintf(comDebugFile, "validateBuf{%d} = 0x%04X\n", ww, validateBuf[ww]);
    }
    fprintf(comDebugFile, "\n");
#endif

    return res;
}

