/**
 * @file
 * @brief GPIO HAL to support MLX75322 on Raspberry Pi platform, using the WiringPi package included on Raspbian
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
 * @defgroup spi_hal_api_gpio_raspi Raspberry PI's GPIO HAL library
 * @ingroup spi_hal_api_gpio_abs
 *
 * @details
 */

#include <stdint.h>
#include <stdio.h>
#include <wiringPi.h>
#include "spi_drv_common_types.h"
#include "spi_drv_hal_gpio.h"

/* uint32_t, so max value (2**32) - 1. Example value: 200000 (x 5us  = 1 second) */
#define READY_POLL_LIMIT_CNT 200000
#define READY_POLL_INTERVAL_US 5

#define RESET_ASSERTION_WIDTH_MS 1
#define RESET_RECOVERY_TIME_MS 100

static uint16_t devIdPinGlobal = 0;

/** RaspberryPi GPIO pinout default configuration */
const GpioConfig_t raspiDefaultGpioCfg = {
    .initDevId = 0,
    .ready0Pin = {
        .port = 0,              /**< not used */
        .pin = 25,              /**< Raspi's GPIO pin */
        .mode = INPUT,          /**< Signal's direction mode INPUT/OUTPUT */
        .miscFlags = PUD_UP     /**< Used for Pulls control initialization */
    },
    .ready1Pin = {
        .port = 0,              /**< not used */
        .pin = 17,              /**< Raspi's GPIO pin */
        .mode = INPUT,          /**< Signal's direction mode INPUT/OUTPUT */
        .miscFlags = PUD_UP     /**< Used for Pulls control initialization */
    },
    .resetPin = {
        .port = 0,              /**< not used */
        .pin = 4,               /**< Raspi's GPIO pin */
        .mode = OUTPUT,         /**< Signal's direction mode INPUT/OUTPUT */
        .miscFlags = PUD_UP     /**< Used for Pulls control initialization */
    },
    .sel1Pin = {
        .port = 0,              /**< not used */
        .pin = 5,               /**< Raspi's GPIO pin */
        .mode = OUTPUT,         /**< Signal's direction mode INPUT/OUTPUT */
        .miscFlags = PUD_DOWN   /**< Used for Pulls control initialization */
    },
    .sel2Pin = {
        .port = 0,              /**< not used */
        .pin = 12,              /**< Raspi's GPIO pin */
        .mode = OUTPUT,         /**< Signal's direction mode INPUT/OUTPUT */
        .miscFlags = PUD_DOWN   /**< Used for Pulls control initialization */
    },
    .sel3Pin = {
        .port = 0,              /**< not used */
        .pin = 6,               /**< Raspi's GPIO pin */
        .mode = OUTPUT,         /**< Signal's direction mode INPUT/OUTPUT */
        .miscFlags = PUD_DOWN   /**< Used for Pulls control initialization */
    },
    .sel4Pin = {
        .port = 0,              /**< not used */
        .pin = 13,              /**< Raspi's GPIO pin */
        .mode = OUTPUT,         /**< Signal's direction mode INPUT/OUTPUT */
        .miscFlags = PUD_DOWN   /**< Used for Pulls control initialization */
    },
};

static GpioConfig_t pinCfg;

FuncResult_e spiDriver_PinInit(const GpioConfig_t* const raspiGpioCfg)
{
    if (raspiGpioCfg != NULL) {
        pinCfg = *raspiGpioCfg;
    } else {
        pinCfg = raspiDefaultGpioCfg;
    }

    wiringPiSetupGpio();
    devIdPinGlobal = pinCfg.initDevId;

    pinMode(pinCfg.ready0Pin.pin, pinCfg.ready0Pin.mode);
    pullUpDnControl(pinCfg.ready0Pin.pin, pinCfg.ready0Pin.miscFlags);

    pinMode(pinCfg.ready1Pin.pin, pinCfg.ready1Pin.mode);
    pullUpDnControl(pinCfg.ready1Pin.pin, pinCfg.ready1Pin.miscFlags);

    pinMode(pinCfg.resetPin.pin, pinCfg.resetPin.mode);
    pullUpDnControl(pinCfg.resetPin.pin, pinCfg.resetPin.miscFlags);
    digitalWrite(pinCfg.resetPin.pin, 1);

    pinMode(pinCfg.sel1Pin.pin, pinCfg.sel1Pin.mode);
    pullUpDnControl(pinCfg.sel1Pin.pin, pinCfg.sel1Pin.miscFlags);
    digitalWrite(pinCfg.sel1Pin.pin, 0);

    pinMode(pinCfg.sel2Pin.pin, pinCfg.sel2Pin.mode);
    pullUpDnControl(pinCfg.sel2Pin.pin, pinCfg.sel2Pin.miscFlags);
    digitalWrite(pinCfg.sel2Pin.pin, 0);

    pinMode(pinCfg.sel3Pin.pin, pinCfg.sel3Pin.mode);
    pullUpDnControl(pinCfg.sel3Pin.pin, pinCfg.sel3Pin.miscFlags);
    digitalWrite(pinCfg.sel3Pin.pin, 0);

    pinMode(pinCfg.sel4Pin.pin, pinCfg.sel4Pin.mode);
    pullUpDnControl(pinCfg.sel4Pin.pin, pinCfg.sel4Pin.miscFlags);
    digitalWrite(pinCfg.sel4Pin.pin, 0);

    delay(100);
    return SPI_DRV_FUNC_RES_OK;
}



FuncResult_e spiDriver_PinResetAsic(void)
{
    digitalWrite(pinCfg.resetPin.pin, LOW);
    delay(RESET_ASSERTION_WIDTH_MS);
    digitalWrite(pinCfg.resetPin.pin, HIGH);
    delay(RESET_RECOVERY_TIME_MS);
    return SPI_DRV_FUNC_RES_OK;
}



FuncResult_e spiDriver_PinSetDev(uint16_t devId)
{
#if (SYNC_TEST_FLOW != 1)
    devIdPinGlobal = devId;
#endif /* SYNC_TEST_FLOW */
    return SPI_DRV_FUNC_RES_OK;
}



uint16_t spiDriver_PinGetSel(void)
{
    return (uint16_t) ( (digitalRead(pinCfg.sel4Pin.pin) << 3)
                        | (digitalRead(pinCfg.sel3Pin.pin) << 2)
                        | (digitalRead(pinCfg.sel2Pin.pin) << 1)
                        | digitalRead(pinCfg.sel1Pin.pin)
                        );
}


/** On the EVK, these pins are used to enable just one of the multiple
 * illumination sources at a time. On the EVK, four light source enable signals
 * are supported. The enable pins do not activate the light source, but rather
 * they simply enable (or select) a light source, which will be dynamically
 * triggered during sensor acquisition of a layer.
 *
 * On the EVK, for each bit of the input parameter, 0 = disable, 1 = enable.
 */
FuncResult_e spiDriver_PinSetSel(uint16_t fourBits)
{
    digitalWrite(pinCfg.sel1Pin.pin, (fourBits & 0x0001) ? HIGH : LOW);
    digitalWrite(pinCfg.sel2Pin.pin, ( (fourBits & 0x0002) >> 1) ? HIGH : LOW);
    digitalWrite(pinCfg.sel3Pin.pin, ( (fourBits & 0x0004) >> 2) ? HIGH : LOW);
    digitalWrite(pinCfg.sel4Pin.pin, ( (fourBits & 0x0008) >> 3) ? HIGH : LOW);
    return SPI_DRV_FUNC_RES_OK;
}



/** The EVK example implementation using a simple polling method on the READY
 * pin, with two compile time constants to control the poll timing.
 *
 * READY_POLL_LIMIT_CNT sets an upper limit on the polling time (a "timeout"
 * value).
 *
 * READY_POLL_INTERVAL_US sets a time, in micro-seconds, to wait between
 * polls. Thus tradeoff can be made between added assertion detection latency
 * and consumption of host CPU time resource.
 */
ComStat_e spiDriver_PinWaitForReady(void)
{
    int pinValue;
    uint32_t ii;
    uint16_t pinId;

    if (devIdPinGlobal == 1) {
        pinId = pinCfg.ready1Pin.pin;
    } else {
        pinId = pinCfg.ready0Pin.pin;
    }

    pinValue = digitalRead(pinId);

    for (ii = 0; ii < READY_POLL_LIMIT_CNT; ii++) {
        if ( pinValue != HIGH ) {
            delayMicroseconds(READY_POLL_INTERVAL_US);
            pinValue = digitalRead(pinId);
        } else {
            break;
        }
    }

    if (ii < READY_POLL_LIMIT_CNT) {
        return CS_SUCCESS;
    } else {
        return CS_TIMEOUT;
    }
}

