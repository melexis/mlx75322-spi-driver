/**
 * @file
 * @brief SPI driver for MLX75322 Continuous mode support
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
 * @ingroup spi_cont_lib
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include "cont_mode_lib.h"
#include "spi_drv_common_types.h"
#include "spi_drv_sync_mode.h"
#include "trig_data.h"
#include "spi_drv_hal_udp.h"

/* API-level functions used by continuous mode which shouldn't be shared as driver's API */

extern FuncResult_e spiDriver_InitContinuousModeInt(void);
extern FuncResult_e spiDriver_StopContinuousModeInt(const uint16_t icIdx);
extern void spiDriver_UpdateCurrentData(const spiDriver_ChipData_t* const spiDriver_chipDataTmp,
                                        const uint16_t spiDriver_chipDataSizeTmp);
extern FuncResult_e spiDriver_StartContinuousMode(const spiDriver_LayerConfig_t* const layerConfigurations,
                                                  const uint16_t layerConfigCount,
                                                  const uint16_t* const layerOrder,
                                                  const uint16_t layerCount);


static pthread_t ContModeThreadID;
static ContModeCmd_e contModeThread[MAX_IC_ID_NUMBER] = { CONT_MODE_NOT_INITED };
uint16_t contModePendingSteps[MAX_IC_ID_NUMBER] = { 0u };
volatile int msqid = 0;
ContModeCfg_t contModeCfg;

extern volatile SpiDriver_State_t spiDriver_currentState;

#if (CONT_MODE_DEBUG == 1)

#define CONT_MODE_NAMES_BUF 11

const char contModeNames[][CONT_MODE_NAMES_BUF] = {
    "IDLE",
    "WORK",
    "LAST SCENE",
    "STOP",
    "ERROR",
    "EXIT",
    "NOT INITED"
};

char* spiDriver_GetContinuousModeName(const ContModeCmd_e mode)
{
    static char outbuf[CONT_MODE_NAMES_BUF];
    strcpy(outbuf, contModeNames[mode]);
    return outbuf;
}

#endif /* CONT_MODE_DEBUG */

/** Helper function which flushes the current message queue from previously left messages */
static void FlushMessageQueue(volatile int msqid, const long msgid)
{
    contModeInterface_t rbuf;
    uint16_t counter = 0;
    while (msgrcv(msqid, &rbuf, contModeInterface_size, msgid, IPC_NOWAIT) >= 0) {
        counter++;
    }
    CONT_PRINT("\nCont thread: %u messages were flushed before run\n", counter);
}

/** Checks if at least one IC in the configuration needs to be processed */
static bool ContModeWork(void)
{
    bool res = false;
    for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
        if (contModeThread[ic] == CONT_MODE_WORK) {
            res = true;
            break;
        }
    }
    return res;
}

/* Continuous mode thread function */
void* contModeExecute(void* temp)
{
    const contModeInterface_t msg_request_data = {.cmd = CONT_MODE_WORK, .mtype = CONT_MODE_REQUEST_DATA};
    const int msgflg = IPC_CREAT | 0666;
#if (CONT_MODE_DEBUG == 1)
    unsigned long index = 0ul;
#endif /* CONT_MODE_DEBUG */
    bool looping = true;
    bool lastRequest[MAX_IC_ID_NUMBER] = { false };
    (void)temp;
    /* get the message queue id for the key with value CONT_MODE_MSQ_KEY */
    CONT_PRINT("\nmsgget: Calling msgget(%#x,\%#o)\n", CONT_MODE_MSQ_KEY, msgflg);
    if ((msqid = msgget(CONT_MODE_MSQ_KEY, msgflg)) < 0) {
        perror("msgget");
        exit(1);
    } else {
        CONT_PRINT("msgget: msgget succeeded: msqid = %d\n", msqid);
        FlushMessageQueue(msqid, 0l);
    }
    for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
        contModePendingSteps[ic] = CONT_MODE_MAX_PENDING;
    }
    while (looping) {
        contModeInterface_t ctrl_buf;
        ssize_t cmd_res;
        if (!(ContModeWork())) {
            CONT_PRINT("\nCont thread: Wait for command\n");
            cmd_res = msgrcv(msqid, &ctrl_buf, contModeInterface_size, CONT_MODE_CTRL, 0);
        } else {
            cmd_res = msgrcv(msqid, &ctrl_buf, contModeInterface_size, CONT_MODE_CTRL, IPC_NOWAIT);
        }

        if (cmd_res < 0) {
            /* No commands */
            if (ContModeWork()) {
                contModeInterface_t rbuf;
                if (msgrcv(msqid, &rbuf, contModeInterface_size, CONT_MODE_DATA_READY, 0) >= 0) { /* Wait for data ready signal */
                    spiDriver_ChipData_t* chipData = (spiDriver_ChipData_t*)spiDriver_chipData;
                    spiDriver_ChipData_t* chipDataIt = chipData;
                    bool newRequest = false;
                    ContModeCbRet_t cbRes;
                    /* Update the pending flags when they're used */
                    for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
                        if (contModePendingSteps[ic] < CONT_MODE_MAX_PENDING) {
                            if (contModePendingSteps[ic] > 0u) {
                                #if (CONT_MODE_DEBUG == 1)
                                CONT_PRINT("Trigger: %u tasks left for IC%u\n",
                                           contModePendingSteps[ic],
                                           spiDriver_currentState.params[ic].icIndex);
                                #endif /* CONT_MODE_DEBUG */
                                contModePendingSteps[ic]--;
                            }
                        }
                    }
                    for (uint16_t dataInd = 0u; dataInd < spiDriver_chipDataSize; dataInd++) {
                        uint16_t icIdx = spiDriver_GetIcIndexById(chipDataIt->chip_id);
                        if (icIdx >= MAX_IC_ID_NUMBER) {
                            CONT_PRINT("Data acquisition error. IC ID [%u] returned was not found\n",
                                       chipDataIt->chip_id);
                        } else {
                            if (rbuf.cmd == CONT_MODE_WORK) {
                                CONT_PRINT("Cont thread: Data from IC%u's ready\n", chipDataIt->chip_id);

                                if ((contModeThread[icIdx] != CONT_MODE_IDLE) && (contModePendingSteps[icIdx] != 0u)) { /* Just skip IDLE devices */
                                    if (!lastRequest[icIdx]) { /* All's ok - we request new data */
                                        if (!newRequest) {
                                            CONT_PRINT("Cont thread: New Data request\n");
                                            newRequest = true;
                                        }
                                    } else {
                                        if (contModeCfg.useAsyncSequence) { /* Trigger IC stop only when it's ready to do that (I.e. latest layer have just been gathered) */
                                            if (spiDriver_currentState.params[icIdx].contState ==
                                                CONT_MODE_STATE_FINISHED) {                                                   /* Call STOP when the scene's finished */
                                                if (spiDriver_StopContinuousModeInt(icIdx) == SPI_DRV_FUNC_RES_OK) {
                                                    CONT_PRINT("Cont thread: Stop IC%u\n",
                                                               spiDriver_currentState.params[icIdx].icIndex);
                                                    contModePendingSteps[icIdx] =
                                                        spiDriver_currentState.params[icIdx].sceneLayersAmount;
                                                    if (contModePendingSteps[icIdx] < 2u) {
                                                        contModePendingSteps[icIdx] = 2;
                                                    }
                                                    CONT_PRINT("Cont thread: IC%u has (%u pending steps)\n",
                                                               chipDataIt->chip_id, contModePendingSteps[icIdx]);
                                                    lastRequest[icIdx] = false;
                                                } else {
                                                    CONT_PRINT("Cont thread: Failed to stop IC%u\n",
                                                               spiDriver_currentState.params[icIdx].icIndex);
                                                }
                                            } else {
                                                CONT_PRINT(
                                                    "Cont thread: Cannot stop IC%u, it processes its layer %u from %u\n",
                                                    chipDataIt->chip_id,
                                                    spiDriver_currentState.params[icIdx].sceneCurrentLayer + 1,
                                                    spiDriver_currentState.params[icIdx].sceneLayersAmount);
                                            }
                                            if (!newRequest) {
                                                CONT_PRINT(
                                                    "Cont thread: Gather pending messages... New Data request\n");
                                                newRequest = true;
                                            }
                                        } else { /* In the syncronized configuration mode we can stop all the IC's at once */
                                            for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
                                                if (spiDriver_StopContinuousModeInt(ic) == SPI_DRV_FUNC_RES_OK) {
                                                    CONT_PRINT("Cont thread: Stop IC%u\n",
                                                               spiDriver_currentState.params[ic].icIndex);
                                                    contModePendingSteps[ic] = 1u;
                                                    if (spiDriver_currentState.params[icIdx].sceneLayersAmount == 1u) {
                                                        contModePendingSteps[ic] = 2;
                                                    }
                                                    CONT_PRINT("Cont thread: IC%u has (%u pending steps)\n",
                                                               chipDataIt->chip_id, contModePendingSteps[ic]);
                                                    lastRequest[ic] = false;
                                                    spiDriver_currentState.continuousMode = false; /* TODO: this may be required to be separate for each IC */
                                                }
                                            }
                                            if (spiDriver_currentState.params[icIdx].sceneLayersAmount == 1u) {
                                                contModePendingSteps[icIdx] = 1u;
                                            }
                                            if (!newRequest) {
                                                CONT_PRINT(
                                                    "Cont thread: Gather pending messages... New Data request\n");
                                                newRequest = true;
                                            }
                                            break; /* we do not need to iterate the ICs since they all do the same [syncronized config] */
                                        }
                                    }
                                }
                            }
                        }
                        chipDataIt++;
                    }
                    if (newRequest) {
                        msgsnd(msqid, &msg_request_data, contModeInterface_size, IPC_CREAT);
                    }
                    CONT_PRINT("Cont thread: Run callback\n");
                    spiDriver_UdpCallback(chipData);
                    if (contModeCfg.callback != NULL) {
                        cbRes = contModeCfg.callback(chipData);
                    } else {
                        cbRes = CB_RET_OK;
                    }
                    CONT_PRINT("Cont thread: CB result:%u\n", cbRes);
                    for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
                        if (contModePendingSteps[ic] == 0u) {
                            const contModeInterface_t out_stop_msg =
                            {.cmd = CONT_MODE_IDLE, .mtype = CONT_MODE_FEEDBACK};
                            CONT_PRINT("Cont thread: No data left for IC%u\n",
                                       spiDriver_currentState.params[ic].icIndex);
                            spiDriver_currentState.params[ic].contState = CONT_MODE_STATE_IDLE;
                            contModeThread[ic] = CONT_MODE_IDLE;
                            msgsnd(msqid, &out_stop_msg, contModeInterface_size, IPC_CREAT);
                            contModePendingSteps[ic] = CONT_MODE_MAX_PENDING;
                        }
                    }

                    (void)cbRes;
                    /* TODO: react on what the callback wants */
                }
            } else {
                for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
                    contModeThread[ic] = CONT_MODE_IDLE;
                }
            }
        } else {
            /* Some command has arrived */
            switch (ctrl_buf.cmd) {
                case CONT_MODE_IDLE: /* Command IDLE. Do nothing */
                    CONT_PRINT("\nCont thread: Thread is idle\n");
                    break;
                case CONT_MODE_WORK:  /* Command WORK. Do initial data request */
                    CONT_PRINT("\nCont thread: Run polling data. Send request %lu\n", index++);
                    msgsnd(msqid, &msg_request_data, contModeInterface_size, IPC_CREAT);
                    break;
                case CONT_MODE_STOP: /* Command STOP. Wait for last response and go to IDLE */
                    CONT_PRINT("\nCont thread: Stop polling data. Waiting for last message\n");
                    ctrl_buf.cmd = CONT_MODE_WORK; /* We should still be in WORK mode */
                    for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
                        lastRequest[ic] = true;
                    }
                    break;
                case CONT_MODE_EXIT: /* Saying goodbye, and exiting the thread */
                    CONT_PRINT("\nCont thread: Goodbye\n");
                    looping = false;
                    break;
                case CONT_MODE_ERROR:
                case CONT_MODE_NOT_INITED:
                default: /* Do nothing, default case */
                    CONT_PRINT("\nCont thread: Not Inited or Error\n");
                    looping = false;
                    break;
            }
            for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
                contModeThread[ic] = ctrl_buf.cmd;
            }
        }
    }
    CONT_PRINT("\nCont thread: Finished\n");
    for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
        contModeThread[ic] = CONT_MODE_NOT_INITED;
    }
    return NULL;
}


void spiDriver_InitContinuousMode(const ContModeCfg_t* const cfg)
{
    contModeCfg = *cfg;
    uint32_t msqid_counter = 0x10000;

    if (contModeThread[0u] == CONT_MODE_NOT_INITED) {
        (void)pthread_create(&ContModeThreadID, NULL, &contModeExecute, NULL);
        while((msqid == 0) && (msqid_counter-- != 0ul)) {
            ;
        }
        if (msqid_counter != 0ul) {
            CONT_PRINT("\nCont thread: Created\n");
            /* get the message queue id for the key with value CONT_MODE_MSQ_KEY */
            CONT_PRINT("\nmsgget: Calling msgget(%#x,\%#o)\n", CONT_MODE_MSQ_KEY, 0666);
            if ((msqid = msgget(CONT_MODE_MSQ_KEY, 0666)) < 0) {
                perror("msgget");
                exit(1);
            } else {
                CONT_PRINT("msgget: msgget succeeded: msqid = %d\n", msqid);
            }
            for (uint16_t ic = 0u; ic < syncModeCfg.icCount; ic++) {
                contModeThread[ic] = CONT_MODE_IDLE;
            }
            (void)spiDriver_InitTrigData();
            spiDriver_InitUdpCallback(DEST_PORT);
            spiDriver_InitContinuousModeInt();
        } else {
            CONT_PRINT("\n Error creating message queue\n");
        }
    }
}

bool spiDriver_RunContinuousMode(void)
{
    bool res = true;
    if (contModeThread[0u] == CONT_MODE_IDLE) { /* TODO: check all thread states */
        contModeInterface_t msg;
        msg.cmd = CONT_MODE_WORK;
        msg.mtype = CONT_MODE_CTRL;

        if ((contModeCfg.layerConfigurations == NULL) || (contModeCfg.layerConfigCount == 0u)) {
            /* Read the layers' config since the config was not defined from configuration */
            CONT_PRINT("Read layer's configuration(s)\n");
            res =
                (spiDriver_ReadSceneConfig(&contModeCfg.layerConfigurations,
                                           &contModeCfg.layerConfigCount) == SPI_DRV_FUNC_RES_OK);
            /* Init the data flow process */
            spiDriver_StartContinuousMode(NULL, 0u, NULL, 0u); /* Make initialization transparent to the layers' parameters as they were just read above */
        } else {
            CONT_PRINT("Use drv-defined layer's configuration(s)\n");
            /* Init the data flow process */
            spiDriver_StartContinuousMode(contModeCfg.layerConfigurations,
                                          contModeCfg.layerConfigCount,
                                          contModeCfg.layerOrder,
                                          contModeCfg.layerCount);
        }
        CONT_PRINT("SEND MESSAGE RUN\n");
        (void)msgsnd(msqid, &msg, contModeInterface_size, IPC_CREAT);
    } else {
        CONT_PRINT("Can't start continuous mode, it's in %s mode\n",
                   spiDriver_GetContinuousModeName(contModeThread[0u]));
        res = false;
    }
    return res;

}

bool spiDriver_StopContinuousMode(void)
{
    bool res = true;
    contModeInterface_t rbuf;
    contModeInterface_t msg;
    msg.cmd = CONT_MODE_STOP;
    msg.mtype = CONT_MODE_CTRL;
    FlushMessageQueue(msqid, CONT_MODE_FEEDBACK);
    CONT_PRINT("SEND MESSAGE STOP\n");
    (void)msgsnd(msqid, &msg, contModeInterface_size, IPC_CREAT);
    // wait for threads to stop
    for (uint16_t ic = 0u; (ic < syncModeCfg.icCount) && res; ic++) {
        if (msgrcv(msqid, &rbuf, contModeInterface_size, CONT_MODE_FEEDBACK, 0) < 0) {
            CONT_PRINT("Trigger: msgrcv err:%x\n", errno);
            res = false;
        }
    }
    if ((res) && (rbuf.cmd == CONT_MODE_IDLE)) {
        CONT_PRINT("Continuous mode is stopped\n");
        res = true;
    } else {
        CONT_PRINT("Continuous mode is not stopped\n");
        res = false;
    }
    return res;
}

bool spiDriver_ExitContinuousMode(void)
{
    bool res;
    if (contModeThread[0u] == CONT_MODE_IDLE) { /* TODO: check all IC configs */
        contModeInterface_t msg;
        msg.cmd = CONT_MODE_EXIT;
        msg.mtype = CONT_MODE_CTRL;
        CONT_PRINT("SEND MESSAGE EXIT\n");
        (void)msgsnd(msqid, &msg, contModeInterface_size, IPC_CREAT);
        spiDriver_ExitTrigData();
        res = true;
    } else {
        CONT_PRINT("Can't exit the continuous mode, it's in %s mode\n",
                   spiDriver_GetContinuousModeName(contModeThread[0u]));
        res = false;
    }
    return res;
}


void spiDriver_UpdateCurrentData(const spiDriver_ChipData_t* const spiDriver_chipDataTmp,
                                 const uint16_t spiDriver_chipDataSizeTmp)
{
    spiDriver_ChipData_t* spiDriver_chipDataOld = (spiDriver_ChipData_t*)spiDriver_chipData;
    uint16_t spiDriver_chipDataSizeOld = spiDriver_chipDataSize;
    spiDriver_chipData = (volatile spiDriver_ChipData_t*)spiDriver_chipDataTmp;
    spiDriver_chipDataSize = (volatile uint16_t)spiDriver_chipDataSizeTmp;
    spiDriver_CleanChipData(spiDriver_chipDataOld, &spiDriver_chipDataSizeOld);
}

