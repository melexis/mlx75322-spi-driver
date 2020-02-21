/**
 * @file
 * @brief SPI driver for MLX75322 Continuous mode support
 * @internal
 *
 * @copyright (C) 2020 Melexis N.V.
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
 * @ingroup spi_udp
 *
 */


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "spi_drv_trace.h"
#include "udp_callback.h"
#include "spi_drv_hal_udp.h"

static int _sockfd;
static uint16_t udp_dest_port = DEST_PORT;


static bool calculateDataLength(spiDriver_ChipData_t chipData, uint16_t* length)
{
    bool ret = false;

    if (length != NULL) {
        ret = true;
        switch(chipData.dataFormat) {
            case CHIP_DATA_FAST:
                *length = sizeof(chipData.data->echo.format_fast);
                break;
            case CHIP_DATA_9P:
                *length = sizeof(chipData.data->echo.format_9p);
                break;
            case CHIP_DATA_SHORT:
                *length = sizeof(chipData.data->echo.format_short);
                break;
            case CHIP_DATA_DETAIL:
                *length = sizeof(chipData.data->echo.format_detail_1);
                break;
            case CHIP_DATA_TRACE:
                *length = chipData.samples * N_CHANNELS * 2;
                break;
            default:
                CONT_PRINT("chipData format not recognized\n");
                ret = false;
                break;
        }
    }
    return ret;
}

static int appendToBuffer(char* buffer, void* data, uint16_t length, uint16_t* pos)
{
    memcpy(buffer + *pos, (char*)data, length);
    *pos += length;

    return 0;
}

static int appendLayerToBuffer(char* buffer, spiDriver_ChipData_t chipData, uint16_t length, uint16_t* pos)
{
    appendToBuffer(buffer, &length, sizeof(uint16_t), pos);
    appendToBuffer(buffer, chipData.data, length, pos);
    appendToBuffer(buffer, &chipData.samples, sizeof(uint16_t), pos);
    appendToBuffer(buffer, chipData.metaData, sizeof(Metadata_t), pos);
    appendToBuffer(buffer, &chipData.dataFormat, sizeof(ChipDataFormat_e), pos);
    appendToBuffer(buffer, &chipData.status, sizeof(FuncResult_e), pos);

    return 0;
}

static void spiDriver_InitUdpSocket(const uint16_t dest_port)
{
    // Creating socket file descriptor
    if ( (_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    } else {
        udp_dest_port = dest_port;
    }
}

/* 0  2             DL    DL+2
 * +--+--+--+ .. +--+--+--+--+--+ .. +--+--+--+--+--+--+--+--+ ..
 * |DL|     Data    |  S  |   metadata     |  F  |  S  | DL
 * +--+--+--+ .. +--+--+--+--+--+ .. +--+--+--+--+--+--+--+--+ ..
 * Layer 0                                             Layer 1
 * --------------------------------------------------------------
 *  DL: length of data buffer
 *  Data: data buffer
 *  S : number of samples
 *  F : format
 *  S : status
 *  M : metadata structure
 */
ContModeCbRet_t spiDriver_UdpCallback(spiDriver_ChipData_t* chipData)
{
    bool size_ok = false;
    ContModeCbRet_t res = CB_RET_OK;
    ssize_t udp_res = 1;
    struct sockaddr_in cliaddr;
    uint16_t data_length = 0;
    char buffer[650000];
    uint16_t buffer_pos = 0;

    /* Filling server information */
    cliaddr.sin_family = AF_INET;    /* IPv4 */
    cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    cliaddr.sin_port = htons(udp_dest_port);

    /* when echo: append all layers to one buffer to send through UDP */
    if (chipData[0].dataFormat != CHIP_DATA_TRACE) {
        for (int i = 0; i < spiDriver_chipDataSize; i++) {
            size_ok = calculateDataLength(chipData[i], &data_length);
            if (size_ok != true) {
                res = CB_RET_EXIT;
                break;
            }
            appendLayerToBuffer(buffer, chipData[i], data_length, &buffer_pos);
        }
        if (res == CB_RET_OK) {
            udp_res = sendto(_sockfd,
                             (const char*)buffer,
                             buffer_pos,
                             MSG_CONFIRM,
                             (const struct sockaddr*) &cliaddr,
                             sizeof(struct sockaddr));
        }
    } else { /* when trace: send every layer separately through UDP */
        for (int i = 0; i < spiDriver_chipDataSize; i += 2) {
            size_ok = calculateDataLength(chipData[i], &data_length);
            if (size_ok != true) {
                res = CB_RET_EXIT;
                break;
            }
            appendLayerToBuffer(buffer, chipData[i], data_length, &buffer_pos);
            udp_res = sendto(_sockfd,
                             (const char*)buffer,
                             buffer_pos,
                             MSG_CONFIRM,
                             (const struct sockaddr*) &cliaddr,
                             sizeof(struct sockaddr));
            buffer_pos = 0;
        }
    }

    if (udp_res != 0) {
        res = CB_RET_EXIT;
    }
    return res;
}

void spiDriver_InitUdpCallback(uint16_t dest_port)
{
    if (dest_port == 0u) {
        dest_port = DEST_PORT;
    }
    spiDriver_InitUdpSocket(dest_port);
}

