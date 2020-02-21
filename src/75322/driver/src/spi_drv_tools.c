/**
 * @file
 * @brief Miscellanious tools and helpers for common purpose
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

#include "spi_drv_tools.h"

uint32_t ReverseBits32(uint32_t bitsData)
{
    bitsData = (((bitsData & 0xAAAAAAAAul) >> 1) | ((bitsData & 0x55555555ul) << 1));
    bitsData = (((bitsData & 0xCCCCCCCCul) >> 2) | ((bitsData & 0x33333333ul) << 2));
    bitsData = (((bitsData & 0xF0F0F0F0ul) >> 4) | ((bitsData & 0x0F0F0F0Ful) << 4));
    bitsData = (((bitsData & 0xFF00FF00ul) >> 8) | ((bitsData & 0x00FF00FFul) << 8));
    return((bitsData >> 16) | (bitsData << 16));
}

uint16_t ReverseBits16(uint16_t bitsData)
{
    bitsData = (((bitsData & 0xAAAAu) >> 1) | ((bitsData & 0x5555u) << 1));
    bitsData = (((bitsData & 0xCCCCu) >> 2) | ((bitsData & 0x3333u) << 2));
    bitsData = (((bitsData & 0xF0F0u) >> 4) | ((bitsData & 0x0F0Fu) << 4));
    return((bitsData >> 8) | (bitsData << 8));
}

uint8_t ReverseBits8(uint8_t bitsData)
{
    bitsData = (((bitsData & 0xAAu) >> 1) | ((bitsData & 0x55u) << 1));
    bitsData = (((bitsData & 0xCCu) >> 2) | ((bitsData & 0x33u) << 2));
    bitsData = (((bitsData & 0xF0u) >> 4) | ((bitsData & 0x0Fu) << 4));
    return bitsData;
}

void ReverseBytes16(uint8_t* buffer, uint16_t size)
{
    size &= ~(uint16_t)1u;
    uint8_t tmpBuffer;
    for (uint16_t ind = 0; ind < size; ind += 2) {
        tmpBuffer = buffer[ind];
        buffer[ind] = buffer[ind + 1];
        buffer[ind + 1] = tmpBuffer;
    }
}

