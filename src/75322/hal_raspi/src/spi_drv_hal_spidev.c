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
 * @defgroup spi_hal_api_spi_raspi Raspberry PI's SPI HAL library
 * @ingroup spi_hal_api_spi_abs
 *
 * @details
 */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "spi_drv_hal_spidev.h"

static uint16_t devIdSpiDevGlobal = 0;

static SpiConfig_t spiCfg = {0};

const SpiConfig_t defaultSpiCfg = {
    .initDevId = 0,
    .mode = SPI_MODE_1,
    .bitsPerWord = 8,
    .speed = 12500000ul,
};

int spiCs0GlobalFd;
int spiCs1GlobalFd;

FuncResult_e spiDriver_SpiOpenPort(const SpiConfig_t* const spiCfgInput)
{
    uint16_t statusValue = 0;
    int ioctlRes = -1;
    int* spiCs0Fd;
    int* spiCs1Fd;
    unsigned long tmpVal;

    /* ----- SET SPI MODE ----- */
    /* SPI_MODE_0: CPOL = 0, CPHA = 0, Clock idle low, data sampled on rising edge, data change on falling edge */
    /* SPI_MODE_1: CPOL = 0, CPHA = 1, Clock idle low, data sampled on falling edge, data change on rising edge */
    /* SPI_MODE_2: CPOL = 1, CPHA = 0, Clock idle high, data sampled on falling edge, data change on rising edge */
    /* SPI_MODE_3: CPOL = 1, CPHA = 1, Clock idle high, data sampled on rising, edge data change on falling edge */

    if (spiCfgInput == NULL) {
        spiCfg = defaultSpiCfg;
    } else {
        spiCfg = *spiCfgInput;
    }

    devIdSpiDevGlobal = spiCfg.initDevId;

    /* -------- */
    spiCs0Fd = &spiCs0GlobalFd;
    spiCs1Fd = &spiCs1GlobalFd;

    *spiCs0Fd = open("/dev/spidev0.0", O_RDWR);
    *spiCs1Fd = open("/dev/spidev0.1", O_RDWR);

    if (*spiCs0Fd < 0) {
        statusValue |= 0x0001;
        perror("Error - Could not open SPI device 0");
    }

    if (*spiCs1Fd < 0) {
        statusValue |= 0x0002;
        perror("Error - Could not open SPI device 1");
    }

    /* -------- */
    tmpVal = (unsigned long)spiCfg.mode;
    ioctlRes = ioctl(*spiCs0Fd, SPI_IOC_WR_MODE, &tmpVal);
    if(ioctlRes < 0) {
        statusValue |= 0x0004;
        perror("Could not set SPIMode (WR) on SPI device 0 ...ioctl fail");
    }

    tmpVal = (unsigned long)spiCfg.mode;
    ioctlRes = ioctl(*spiCs1Fd, SPI_IOC_WR_MODE, &tmpVal);
    if(ioctlRes < 0) {
        statusValue |= 0x0008;
        perror("Could not set SPIMode (WR) on SPI device 1 ...ioctl fail");
    }

    /* -------- */
    ioctlRes = ioctl(*spiCs0Fd, SPI_IOC_RD_MODE, &tmpVal);
    if(ioctlRes < 0) {
        statusValue |= 0x0010;
        perror("Could not set SPIMode (RD) on SPI device 0 ...ioctl fail");
    }

    ioctlRes = ioctl(*spiCs1Fd, SPI_IOC_RD_MODE, &tmpVal);
    if(ioctlRes < 0) {
        statusValue |= 0x0020;
        perror("Could not set SPIMode (RD) on SPI device 1 ...ioctl fail");
    }

    /* -------- */
    tmpVal = (unsigned long)spiCfg.bitsPerWord;
    ioctlRes = ioctl(*spiCs0Fd, SPI_IOC_WR_BITS_PER_WORD, &tmpVal);
    if(ioctlRes < 0) {
        statusValue |= 0x0040;
        perror("Could not set SPI bitsPerWord (WR) on SPI device 0 ...ioctl fail");
    }

    tmpVal = (unsigned long)spiCfg.bitsPerWord;
    ioctlRes = ioctl(*spiCs1Fd, SPI_IOC_WR_BITS_PER_WORD, &tmpVal);
    if(ioctlRes < 0) {
        statusValue |= 0x0080;
        perror("Could not set SPI bitsPerWord (WR) on SPI device 1 ...ioctl fail");
    }

    /* -------- */
    ioctlRes = ioctl(*spiCs0Fd, SPI_IOC_RD_BITS_PER_WORD, &tmpVal);
    if(ioctlRes < 0) {
        statusValue |= 0x0100;
        perror("Could not set SPI bitsPerWord(RD) on SPI device 0 ...ioctl fail");
    }

    ioctlRes = ioctl(*spiCs1Fd, SPI_IOC_RD_BITS_PER_WORD, &tmpVal);
    if(ioctlRes < 0) {
        statusValue |= 0x0200;
        perror("Could not set SPI bitsPerWord(RD) on SPI device 1 ...ioctl fail");
    }

    /* -------- */
    tmpVal = (unsigned long)spiCfg.speed;
    ioctlRes = ioctl(*spiCs0Fd, SPI_IOC_WR_MAX_SPEED_HZ, &tmpVal);
    if(ioctlRes < 0) {
        statusValue |= 0x0400;
        perror("Could not set SPI speed (WR) on SPI device 0 ...ioctl fail");
    }

    tmpVal = (unsigned long)spiCfg.speed;
    ioctlRes = ioctl(*spiCs1Fd, SPI_IOC_WR_MAX_SPEED_HZ, &tmpVal);
    if(ioctlRes < 0) {
        statusValue |= 0x0800;
        perror("Could not set SPI speed (WR) on SPI device 1 ...ioctl fail");
    }

    /* -------- */
    ioctlRes = ioctl(*spiCs0Fd, SPI_IOC_RD_MAX_SPEED_HZ, &tmpVal);
    if(ioctlRes < 0) {
        statusValue |= 0x1000;
        perror("Could not set SPI speed (RD) on SPI device 0 ...ioctl fail");
    }
    ioctlRes = ioctl(*spiCs1Fd, SPI_IOC_RD_MAX_SPEED_HZ, &tmpVal);
    if(ioctlRes < 0) {
        statusValue |= 0x2000;
        perror("Could not set SPI speed (RD) on SPI device 1 ...ioctl fail");
    }

    /* -------- */
    if (statusValue == 0) {
        return SPI_DRV_FUNC_RES_OK;
    } else {
        return SPI_DRV_FUNC_RES_FAIL_COMM;
    }
}



FuncResult_e spiDriver_SpiSetDev(uint16_t devId)
{
#if (SYNC_TEST_FLOW != 1)
    devIdSpiDevGlobal = devId;
#endif /* SYNC_TEST_FLOW */
    return SPI_DRV_FUNC_RES_OK;
}



uint16_t spiDriver_SpiGetDev(void)
{
    return devIdSpiDevGlobal;
}



FuncResult_e spiDriver_SpiWriteAndRead(unsigned char* data, int length)
{
    struct spi_ioc_transfer spiIOC;
    int* spiCsFd;
    int ioctlRes = -1;
    uint16_t statusValue = 0;

    if (devIdSpiDevGlobal == 0) {
        spiCsFd = &spiCs0GlobalFd;
    } else {
        spiCsFd = &spiCs1GlobalFd;
    }

    memset(&spiIOC, 0, sizeof (spiIOC));
    spiIOC.tx_buf = (unsigned long)(data);            /* transmit from and receive to a single buffer simultaneously */
    spiIOC.rx_buf = (unsigned long)(data);
    spiIOC.len = length;
    spiIOC.delay_usecs = 0;
    spiIOC.speed_hz = 0;
    spiIOC.bits_per_word = spiCfg.bitsPerWord;
    spiIOC.cs_change = 0;

    ioctlRes = ioctl(*spiCsFd, SPI_IOC_MESSAGE(1), &spiIOC);

    if(ioctlRes < 0) {
        statusValue |= 0x0001;
        perror("Error - Problem transmitting spi data..ioctl");
    }

    if (statusValue == 0) {
        return SPI_DRV_FUNC_RES_OK;
    } else {
        return SPI_DRV_FUNC_RES_FAIL_COMM;
    }
}



FuncResult_e spiDriver_SpiClosePort(void)
{
    int* spiCs0Fd;
    int* spiCs1Fd;
    int res;
    uint16_t statusValue = 0;

    spiCs1Fd = &spiCs1GlobalFd;
    spiCs0Fd = &spiCs0GlobalFd;

    res = close(*spiCs0Fd);
    if(res < 0) {
        statusValue |= 0x0001;
        perror("Error - Could not close SPI device 0");
    }

    res = close(*spiCs1Fd);
    if(res < 0) {
        statusValue |= 0x0002;
        perror("Error - Could not close SPI device 1");
    }

    if (statusValue == 0) {
        return SPI_DRV_FUNC_RES_OK;
    } else {
        return SPI_DRV_FUNC_RES_FAIL_COMM;
    }
}

