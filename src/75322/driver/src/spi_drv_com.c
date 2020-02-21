/**
 * @file
 * @brief API for low-level SPI driver interaction
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
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "spi_drv_common_types.h"
#include "spi_drv_com.h"
#include "spi_drv_com_tools.h"
#include "spi_drv_tools.h"
#include "spi_drv_hal_spidev.h"
#include "spi_drv_hal_gpio.h"

/** @{*/

/* ---------------- Variables ---------------- */

DiagDetailsPkt_t diagDetails[2];
uint16_t pktWords [MAX_PKT_WORDS];
uint16_t payload [MAX_PKT_PAYLOAD_WORDS];


/** Used for CRC calculations on each SPI packet transferred (MISO, MOSI) */
const uint16_t crcTable [] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD,
    0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C,
    0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF,
    0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE,
    0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969,
    0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58,
    0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B,
    0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A,
    0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025,
    0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214,
    0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447,
    0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676,
    0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1,
    0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0,
    0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83,
    0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2,
    0x0ED1, 0x1EF0
};

/* ---------------- External Functions ---------------- */

FuncResult_e spiCom_Init(const SpiComConfig_t* const comCfg)
{
    FuncResult_e res;

#if (COM_DEBUG_DETAIL_0 == 1) || \
    (COM_DEBUG_DETAIL_1 == 1) || \
    (COM_DEBUG_DETAIL_2 == 1)
    comDebugFile = fopen("com_debug.org", "w");
    if (comDebugFile == NULL) {
        printf("main.c: Error opening com_debug.org as output file\n");
        exit(1);
    }
#endif

    COM_DEBUG_PRINT(comDebugFile, "** %s\n", __FUNCTION__);

    if (comCfg == NULL) {
        spiDriver_PinInit(NULL);
        res = spiDriver_SpiOpenPort(NULL);
    } else {
        spiDriver_PinInit(comCfg->pinCfg);
        res = spiDriver_SpiOpenPort(comCfg->spiCfg);
    }

    return res;
}



FuncResult_e spiCom_ResetASIC(void)
{
    COM_DEBUG_PRINT(comDebugFile, "** %s: ---- RESET ----\n", __FUNCTION__);
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;

    spiDriver_PinResetAsic();

    return res;
}



FuncResult_e spiCom_SetDev(uint16_t devId)
{
    FuncResult_e res;
    res = spiDriver_SpiSetDev(devId);
    res |= spiDriver_PinSetDev(devId);
    return res;
}



FuncResult_e spiCom_SetSel(uint16_t fourBits)
{
    COM_DEBUG_PRINT(comDebugFile, "** %s: fourBits = 0x%01X\n", __FUNCTION__, fourBits);
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;

    spiDriver_PinSetSel(fourBits);

    return res;
}



ComStat_e spiCom_WaitForReady(void)
{
    ComStat_e res;
    res = spiDriver_PinWaitForReady();

    if (res == CS_TIMEOUT) {
        COM_DEBUG_PRINT(comDebugFile, "** %s: ERROR: spiDriver_PinWaitForReady() returned CS_TIMEOUT\n", __FUNCTION__);
        printf("%s: ERROR: spiDriver_PinWaitForReady() returned CS_TIMEOUT\n", __FUNCTION__);
    }

    return res;
}



FuncResult_e spiCom_Read(const uint16_t offset, const uint16_t wordSize, uint16_t* readWords)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t totalSize;
    uint16_t numXactFull;
    uint16_t numXactPartial;
    uint16_t sizeXactPartial;
    uint16_t offsetXact;
    uint16_t sizeXact;
    uint16_t* rptr;
    uint16_t xactNum;
    uint16_t* misoWords = NULL;
    uint8_t* pktBytes;

    COM_DEBUG_PRINT(comDebugFile,
                    "** %s:  ---- READ ----  devId = %0d, offset = 0x%04X, wordSize = %0d\n",
                    __FUNCTION__,
                    spiDriver_SpiGetDev(),
                    offset,
                    wordSize
                    );

    clearDiagDetails();

    totalSize = wordSize;
    numXactFull = totalSize / MAX_RW_SIZE;         /* Whole number division (drop the remainder).  */
    sizeXactPartial = totalSize % MAX_RW_SIZE;     /* The remainder.                               */

    if (sizeXactPartial > 0) {                      /* If total_size is a not exact multiple of MAX */
        numXactPartial = 1;
    }                                                 /* Then one last xact is a partial              */
    else {                                            /* Else                                         */
        numXactPartial = 0;
    }                                                 /* ll xacts are full (MAX size)                 */

    offsetXact = offset;                             /* Start with original offset from client.      */
    rptr = readWords;                                /* Start with base of read_words from client    */


    for ( xactNum = 1; xactNum < (numXactFull + numXactPartial + 1); xactNum++ ) {
        if ( (xactNum > numXactFull) && (numXactPartial == 1) ) {
            sizeXact = sizeXactPartial;
        } else {
            sizeXact = MAX_RW_SIZE;
        }

        /* - - - - - - - - Packet 1 - - - - - - - - */

        payload[0] = offsetXact;
        payload[1] = 0;
        makeSpiPacket(READ, sizeXact, 2, payload); /* Make Packet 1, size is for Packet 2 */

        pktBytes = (uint8_t*)pktWords;          /* Words become bytes for transmission */
        ReverseBytes16(pktBytes, 8);

        diagDetails[0].comStat |= spiCom_WaitForReady();                     /* WAIT_FOR_READY */
        diagDetails[0].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8);  /* Send Packet 1 on MOSI, capture MISO */

        if (diagDetails[0].halStat >= 0) {
            ReverseBytes16(pktBytes, 8);
            misoWords = (uint16_t*)pktBytes;
            res |= validatePkt(0, STATUS_SHORT, 2, misoWords);
        } else {
            res |= SPI_DRV_FUNC_RES_FAIL_COMM;
        }

        /* - - - - - - - - Packet 2 - - - - - - - -                                          */
        if (sizeXact < 3) {                          /* Make Packet 2 a STATUS_SHORT        */

            payload [0] = 0;
            payload [1] = 0;
            makeSpiPacket(STATUS_SHORT, 0, 2, payload);

            pktBytes = (uint8_t*)pktWords;           /* Words become bytes for transmission */
            ReverseBytes16(pktBytes, 8);

            diagDetails[1].comStat |= spiCom_WaitForReady();                                              /* WAIT_FOR_READY                            */
            diagDetails[1].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8);   /* Send Packet 2 on MOSI, capture MISO bytes */

            if (diagDetails[1].halStat >= 0) {
                ReverseBytes16(pktBytes, 8);
                misoWords = (uint16_t*)pktBytes;
                res |= validatePkt(1, READ_DATA_RESP_SHORT, sizeXact, misoWords);
            } else {
                res |= SPI_DRV_FUNC_RES_FAIL_COMM;
            }
        } else {                                        /* Make Packet 2 = STATUS_LONG         */
            memset(payload, 0, sizeof(payload));
            makeSpiPacket(STATUS_LONG, 0, sizeXact, payload);

            pktBytes = (uint8_t*)pktWords;
            ReverseBytes16(pktBytes, (sizeXact + 2) * 2);

            diagDetails[1].comStat |= spiCom_WaitForReady();                                      /* WAIT_FOR_READY                            */
            diagDetails[1].halStat = spiDriver_SpiWriteAndRead(pktBytes, (sizeXact * 2) + 4);    /* Send Packet 2 on MOSI, capture MISO bytes */

            if (diagDetails[1].halStat >= 0) {
                ReverseBytes16(pktBytes, (sizeXact + 2) * 2);
                misoWords = (uint16_t*)pktBytes;
                res |= validatePkt(1, READ_DATA_RESP_LONG, sizeXact, misoWords);
            } else {
                res |= SPI_DRV_FUNC_RES_FAIL_COMM;
            }
        }

        memcpy(rptr, &misoWords[1], (sizeXact * sizeof(uint16_t)) );   /* copy payload portion to client buffer*/
        rptr += sizeXact;

        offsetXact += MAX_RW_SIZE;

    }  /* for xactNum++ */

    return res;
}



FuncResult_e spiCom_Write(const uint16_t offset,const uint16_t wordSize,uint16_t* writeWords,const bool patch)
{

    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t totalSize;
    uint16_t numXactFull;
    uint16_t numXactPartial;
    uint16_t sizeXactPartial;
    uint16_t sizeXact;
    uint16_t sizeBytesXact;
    uint16_t offsetXact;
    uint16_t* ptrWriteWords;
    uint16_t xactNum;
    uint16_t pkt1Type;
    uint16_t* misoWords = NULL;
    uint8_t* pktBytes;

#if (COM_DEBUG_DETAIL_0 == 1)
    if ( (wordSize == 1) && (patch == false) ) {
        COM_DEBUG_PRINT(comDebugFile,
                        "** %s: ---- WRITE ---- devId = %0d, offset = 0x%04X,    value = 0x%04X\n",
                        __FUNCTION__,
                        spiDriver_SpiGetDev(),
                        offset,
                        *writeWords);
    } else {
        COM_DEBUG_PRINT(comDebugFile,
                        "** %s: ---- WRITE ---- devId = %0d, offset = 0x%04X, wordSize = %0d, patch = %0d\n",
                        __FUNCTION__,
                        spiDriver_SpiGetDev(),
                        offset,
                        wordSize,
                        patch);
    }
#endif

    clearDiagDetails();

    totalSize = wordSize;

    if ( patch == 0 ) {
        pkt1Type = WRITE;
    } else {
        pkt1Type = WRITE_PATCH;
    }

    numXactFull = totalSize / MAX_RW_SIZE;     /* Whole number division (drop the remainder).  */
    sizeXactPartial = totalSize % MAX_RW_SIZE; /* The remainder.                               */

    if (sizeXactPartial > 0) {                  /* If total_size is a not exact multiple of MAX */
        numXactPartial = 1;
    }                                             /* Then one last xact is a partial      */
    else {                                        /* Else                                    */
        numXactPartial = 0;
    }                                             /* all xacts are full (MAX size)        */

    offsetXact = offset;                         /* Start with original offset from client. */
    ptrWriteWords = writeWords;                /* Start from base of payload from client. */

    for ( xactNum = 1; xactNum < (numXactFull + numXactPartial + 1); xactNum++ ) {
        if ( (xactNum > numXactFull) && (numXactPartial == 1) ) {
            sizeXact = sizeXactPartial;
        } else {
            sizeXact = MAX_RW_SIZE;
        }

        sizeBytesXact = sizeXact * 2;

        if (sizeXact == 1) {                    /* Then single packet transaction, 2-word payload is [offset, value] */
            payload[0] = offsetXact;            /* Make Packet 1, a WRITE with payload = [offset, value] */
            payload[1] = writeWords[0];

            makeSpiPacket(pkt1Type, 1, 2, payload); /* sizeField = 1 ; sizePayload = 2 */

            pktBytes = (uint8_t*)pktWords;          /* Words become bytes for transmission */
            ReverseBytes16(pktBytes, 8);

            diagDetails[0].comStat |= spiCom_WaitForReady();                                            /* WAIT_FOR_READY                      */
            diagDetails[0].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8); /* Send Packet 1 on MOSI, capture MISO */
        } else {
            payload[0] = offsetXact; /* Make Packet 1, a WRITE with offset, long payload will follow in Packet 2 */
            payload[1] = 0;

            makeSpiPacket(pkt1Type, sizeXact, 2, payload);               /* size_field = offsetXact (for Packet 2) ; sizePayload = 2 (for this Packet 1) */

            pktBytes = (uint8_t*)pktWords;          /* Words become bytes for transmission */
            ReverseBytes16(pktBytes, 8);


            diagDetails[0].comStat |= spiCom_WaitForReady();    /* WAIT_FOR_READY */
            diagDetails[0].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8); /* Send Packet 1 on MOSI, capture MISO bytes */
            /* Convert misoBytes to words and validate */
            if (diagDetails[0].halStat >= 0) {
                ReverseBytes16(pktBytes, 8);
                misoWords = (uint16_t*)pktBytes;
                res |= validatePkt(0, STATUS_SHORT, 2, misoWords);
            } else {
                res |= SPI_DRV_FUNC_RES_FAIL_COMM;
            }

            /* -------- Make Packet 2 -------- */
            makeSpiPacket(WRITE_DATA_LONG, sizeXact, sizeXact, ptrWriteWords);

            pktBytes = (uint8_t*)pktWords;
            ReverseBytes16(pktBytes, (sizeXact + 2) * sizeof(uint16_t) );

            diagDetails[1].comStat |= spiCom_WaitForReady();                                                              /* WAIT_FOR_READY */
            diagDetails[1].halStat = spiDriver_SpiWriteAndRead(pktBytes, sizeBytesXact + 4); /* Send Packet 2 on MOSI, capture MISO bytes */
            /* Convert miso bytes to words and validate */
            if (diagDetails[1].halStat >= 0) {
                ReverseBytes16(pktBytes, sizeBytesXact + 4);
                misoWords = (uint16_t*)pktBytes;
                res |= validatePkt(1, STATUS_LONG, sizeXact, misoWords);
            } else {
                res |= SPI_DRV_FUNC_RES_FAIL_COMM;
            }
        }

        offsetXact += MAX_RW_SIZE;
        ptrWriteWords += sizeXact;
    }

    return res;
}



FuncResult_e spiCom_WritePatch(uint32_t offset, uint32_t size, uint8_t* dataBuf)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t* patchWords = NULL;
    uint16_t wordSize = size >> 1;     /* Check for and only allow even size */

    COM_DEBUG_PRINT(comDebugFile, "** %s: devId = %0d\n", __FUNCTION__, spiDriver_SpiGetDev());

    patchWords = (uint16_t*)dataBuf;          /* Bytes become words for packet construction */
    ReverseBytes16((uint8_t*)patchWords, (wordSize * 2) );

    for ( uint16_t ww = 0; ww < wordSize; ww++ ) {
    }

    spiCom_Write(offset, wordSize, patchWords, 1); /* patch=1 */

    return res;
}



FuncResult_e spiCom_ApplyPatch(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t* misoWords;
    uint8_t* pktBytes;

    clearDiagDetails();

    COM_DEBUG_PRINT(comDebugFile, "** %s: devId = %0d\n", __FUNCTION__, spiDriver_SpiGetDev());

    payload[0] = APPLY_PATCH;
    payload[1] = 0;
    makeSpiPacket(FUNCTION, 0, 2, payload);                        /* Make Packet 1 */

    pktBytes = (uint8_t*)pktWords;          /* Words become bytes for transmission */
    ReverseBytes16(pktBytes, 8);

    diagDetails[0].comStat |= spiCom_WaitForReady();                                            /* WAIT_FOR_READY */
    diagDetails[0].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8); /* Send Packet on MOSI, capture MISO */

    if (diagDetails[0].halStat >= 0) {
        ReverseBytes16(pktBytes, 8);
        misoWords = (uint16_t*)pktBytes;
        res = validatePkt(0, STATUS_SHORT, 2, misoWords);
    } else {
        res = SPI_DRV_FUNC_RES_FAIL_COMM;
    }

    return(res);                                         /* (Overall Error Status Flag) */

}



FuncResult_e spiCom_SensorStart(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t* misoWords;
    uint8_t* pktBytes;

    COM_DEBUG_PRINT(comDebugFile, "** %s: devId = %0d\n", __FUNCTION__, spiDriver_SpiGetDev());
    clearDiagDetails();

    payload[0] = SENSOR_START;
    payload[1] = 0;
    makeSpiPacket(FUNCTION, 0, 2, payload);                        /* Make Packet 1 */

    pktBytes = (uint8_t*)pktWords;          /* Words become bytes for transmission */
    ReverseBytes16(pktBytes, 8);

    diagDetails[0].comStat |= spiCom_WaitForReady();                                            /* WAIT_FOR_READY */
    diagDetails[0].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8); /* Send Packet on MOSI, capture MISO */

    if (diagDetails[0].halStat >= 0) {
        ReverseBytes16(pktBytes, 8);
        misoWords = (uint16_t*)pktBytes;
        res |= validatePkt(0, STATUS_SHORT, 2, misoWords);
    } else {
        res |= SPI_DRV_FUNC_RES_FAIL_COMM;
    }


    payload[0] = 0;
    payload[1] = 0;
    makeSpiPacket(STATUS_SHORT, 0, 2, payload);                    /* Make Packet 2 */

    pktBytes = (uint8_t*)pktWords;          /* Words become bytes for transmission */
    ReverseBytes16(pktBytes, 8);

    diagDetails[1].comStat |= spiCom_WaitForReady();                                            /* WAIT_FOR_READY */
    diagDetails[1].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8); /* Send Packet on MOSI, capture MISO */

    if (diagDetails[1].halStat >= 0) {
        ReverseBytes16(pktBytes, 8);
        misoWords = (uint16_t*)pktBytes;
        res |= validatePkt(1, STATUS_SHORT, 2, misoWords);
    } else {
        res |= SPI_DRV_FUNC_RES_FAIL_COMM;
    }

    return res;
}



FuncResult_e spiCom_AcquSync(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t* misoWords;
    uint8_t* pktBytes;

    COM_DEBUG_PRINT(comDebugFile, "** %s: devId = %0d\n", __FUNCTION__, spiDriver_SpiGetDev());
    clearDiagDetails();

    payload[0] = ACQU_SYNC;
    payload[1] = 0;
    makeSpiPacket(FUNCTION, 0, 2, payload);                        /* Make Packet 1 */

    pktBytes = (uint8_t*)pktWords;          /* Words become bytes for transmission */
    ReverseBytes16(pktBytes, 8);

    diagDetails[0].comStat |= spiCom_WaitForReady();                                            /* WAIT_FOR_READY */
    diagDetails[0].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8); /* Send Packet on MOSI, capture MISO */

    if (diagDetails[0].halStat >= 0) {
        ReverseBytes16(pktBytes, 8);
        misoWords = (uint16_t*)pktBytes;
        res |= validatePkt(0, STATUS_SHORT, 2, misoWords);
    } else {
        res |= SPI_DRV_FUNC_RES_FAIL_COMM;
    }

    return res;
}



FuncResult_e spiCom_Sync(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t* misoWords;
    uint8_t* pktBytes;

    COM_DEBUG_PRINT(comDebugFile, "** %s:     devId = %0d\n", __FUNCTION__, spiDriver_SpiGetDev());
    clearDiagDetails();

    payload[0] = 0;
    payload[1] = 0;
    makeSpiPacket(SYNC, 0, 2, payload);                            /* Make Packet 1 */

    pktBytes = (uint8_t*)pktWords;          /* Words become bytes for transmission */
    ReverseBytes16(pktBytes, 8);

    diagDetails[0].comStat |= spiCom_WaitForReady();                                            /* WAIT_FOR_READY */
    diagDetails[0].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8); /* Send Packet on MOSI, capture MISO */

    if (diagDetails[0].halStat >= 0) {
        ReverseBytes16(pktBytes, 8);
        misoWords = (uint16_t*)pktBytes;
        res |= validatePkt(0, SYNC, 0, misoWords);
    } else {
        res |= SPI_DRV_FUNC_RES_FAIL_COMM;
    }

    return res;
}



FuncResult_e spiCom_GetRaw(uint16_t layersAndSamples, uint16_t* trace, uint16_t* rawMetaData)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t wordSize;
    uint16_t cc;
    uint16_t* dptr = trace;
    uint16_t* mptr = rawMetaData;
    uint16_t* misoWords;
    uint8_t* pktBytes;

    COM_DEBUG_PRINT(comDebugFile,
                    "** %s:   devId = %0d, wordSize = %0d\n",
                    __FUNCTION__,
                    spiDriver_SpiGetDev(),
                    layersAndSamples);

    clearDiagDetails();

    wordSize = layersAndSamples;
    dptr = trace;
    mptr = rawMetaData;

    for (cc = 0; cc < 16; cc++ ) {                 /*  always loop over 16 channels (a "frame") */
        payload[0] = GET_RAW;
        payload[1] = 0;
        makeSpiPacket(FUNCTION, wordSize, 2, payload); /*  Make Packet 1, wordSize is for Packet 2, payload[1] is always 0 */

        pktBytes = (uint8_t*)pktWords;          /* Words become bytes for transmission */
        ReverseBytes16(pktBytes, 8);

        diagDetails[0].comStat |= spiCom_WaitForReady();                                            /*  WAIT_FOR_READY */
        diagDetails[0].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8); /*  Send Packet 1 on MOSI, capture MISO */

        if (diagDetails[0].halStat >= 0) {                         /* Convert misoBytes to words and validate */
            ReverseBytes16(pktBytes, 8);
            misoWords = (uint16_t*)pktBytes;
            res |= validatePkt(0, STATUS_SHORT, 2, misoWords);
        } else {
            res |= SPI_DRV_FUNC_RES_FAIL_COMM;
        }

        memset(payload, 0, sizeof(payload));
        makeSpiPacket(STATUS_LONG, 0, wordSize, payload);

        pktBytes = (uint8_t*)pktWords;
        ReverseBytes16(pktBytes, (wordSize + 2) * 2);

        diagDetails[0].comStat |= spiCom_WaitForReady();                                                             /*  WAIT_FOR_READY */
        diagDetails[0].halStat = spiDriver_SpiWriteAndRead(pktBytes, (wordSize * 2) + 4); /*  Send Packet 2 on MOSI, capture MISO bytes */

        if (diagDetails[0].halStat >= 0) {
            ReverseBytes16(pktBytes, (wordSize + 2) * 2);
            misoWords = (uint16_t*)pktBytes;
            res |= validatePkt(1, RAW_DATA_RESP, wordSize, misoWords);
        } else {
            res |= SPI_DRV_FUNC_RES_FAIL_COMM;
        }

        /* copy payload portion of pktBytes to output buffers */
        /* 8 words of metadata come first */
        memcpy(mptr, &pktBytes[2], 16);
        mptr += 8;
        /* then remainder is echo structure data */
        memcpy(dptr, &pktBytes[18], (wordSize - 8) * sizeof(uint16_t));
        dptr += wordSize - 8;
    }

    return res;
}



FuncResult_e spiCom_GetEcho(uint16_t echoByte, uint16_t* EchoesData, uint16_t* echoMetaData)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t wordSize;
    uint16_t* dptr = EchoesData;
    uint16_t* mptr = echoMetaData;
    uint16_t* misoWords;
    uint8_t* pktBytes;

    COM_DEBUG_PRINT(comDebugFile,
                    "** %s:  devId = %0d, wordSize = %0d\n",
                    __FUNCTION__,
                    spiDriver_SpiGetDev(),
                    echoByte);

    clearDiagDetails();

    wordSize = echoByte;

    payload[0] = GET_ECHO;
    payload[1] = 0;
    makeSpiPacket(FUNCTION, wordSize, 2, payload); /* Make Packet 1, wordSize is for Packet 2, payload[1] is always 0 */

    pktBytes = (uint8_t*)pktWords;          /* Words become bytes for transmission */
    ReverseBytes16(pktBytes, 8);

    diagDetails[0].comStat |= spiCom_WaitForReady();                                            /* WAIT_FOR_READY */
    diagDetails[0].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8); /*  Send Packet 1 on MOSI, capture MISO */

    if (diagDetails[0].halStat >= 0) {    /* Convert misoBytes to words and validate */
        ReverseBytes16(pktBytes, 8);
        misoWords = (uint16_t*)pktBytes;
        res |= validatePkt(0, STATUS_SHORT, 2, misoWords);
    } else {
        res |= SPI_DRV_FUNC_RES_FAIL_COMM;
    }

    memset(payload, 0, sizeof(payload));
    makeSpiPacket(STATUS_LONG, 0, wordSize, payload);

    pktBytes = (uint8_t*)pktWords;
    ReverseBytes16(pktBytes, (wordSize + 2) * 2);

    diagDetails[1].comStat |= spiCom_WaitForReady();                                                             /*  WAIT_FOR_READY */
    diagDetails[1].halStat = spiDriver_SpiWriteAndRead(pktBytes, (wordSize + 2) * 2); /*  Send Packet 2 on MOSI, capture MISO bytes */

    if (diagDetails[1].halStat >= 0) {    /* Convert misoBytes to words and validate */
        ReverseBytes16(pktBytes, (wordSize + 2) * 2);
        misoWords = (uint16_t*)pktBytes;
        res |= validatePkt(1, ECHO_DATA_RESP, wordSize, misoWords);
    } else {
        res |= SPI_DRV_FUNC_RES_FAIL_COMM;
    }

    /* copy payload portion of pktBytes to output buffers */
    /* 8 words of metadata come first */
    memcpy(mptr, &pktBytes[2], 16);
    mptr += 8;
    /* then remainder is echo structure data */
    memcpy(dptr, &pktBytes[18], (wordSize - 8) * sizeof(uint16_t));
    dptr += wordSize - 8;

    return res;
}



FuncResult_e spiCom_SensorStop(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t* misoWords;
    uint8_t* pktBytes;

    COM_DEBUG_PRINT(comDebugFile, "** %s: devId = %0d\n", __FUNCTION__, spiDriver_SpiGetDev());
    clearDiagDetails();

    payload[0] = SENSOR_STOP;
    payload[1] = 0;
    makeSpiPacket(FUNCTION, 0, 2, payload);                        /* Make Packet 1 */

    pktBytes = (uint8_t*)pktWords;          /* Words become bytes for transmission */
    ReverseBytes16(pktBytes, 8);

    diagDetails[0].comStat |= spiCom_WaitForReady();                                            /* WAIT_FOR_READY */
    diagDetails[0].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8); /* Send Packet on MOSI, capture MISO */

    if (diagDetails[0].halStat >= 0) {
        ReverseBytes16(pktBytes, 8);
        misoWords = (uint16_t*)pktBytes;
        res |= validatePkt(0, STATUS_SHORT, 2, misoWords);
    } else {
        res |= SPI_DRV_FUNC_RES_FAIL_COMM;
    }

    return res;
}



FuncResult_e spiCom_SensorStandby(void)
{
    FuncResult_e res = SPI_DRV_FUNC_RES_OK;
    uint16_t* misoWords;
    uint8_t* pktBytes;

    COM_DEBUG_PRINT(comDebugFile, "** %s: devId = %0d\n", __FUNCTION__, spiDriver_SpiGetDev());
    clearDiagDetails();

    payload[0] = SENSOR_STANDBY;
    payload[1] = 0;
    makeSpiPacket(FUNCTION, 0, 2, payload);                        /* Make Packet 1 */

    pktBytes = (uint8_t*)pktWords;          /* Words become bytes for transmission */
    ReverseBytes16(pktBytes, 8);

    diagDetails[0].comStat |= spiCom_WaitForReady();                                            /* WAIT_FOR_READY */
    diagDetails[0].halStat = spiDriver_SpiWriteAndRead(pktBytes, 8); /* Send Packet on MOSI, capture MISO */

    if (diagDetails[0].halStat >= 0) {
        ReverseBytes16(pktBytes, 8);
        misoWords = (uint16_t*)pktBytes;
        res |= validatePkt(0, STATUS_SHORT, 2, misoWords);
    } else {
        res |= SPI_DRV_FUNC_RES_FAIL_COMM;
    }

    return res;
}

/** @}*/

