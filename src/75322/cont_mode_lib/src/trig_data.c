/**
 * @file
 * @brief Continuous Data Measuring Task
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
 * @ingroup spi_cont_get_data
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
#include "trig_data.h"
#include "spi_drv_trace.h"
#include "spi_drv_sync_mode.h"

static pthread_t trigDataThreadID;
static ContModeCmd_e trigDataMode = CONT_MODE_NOT_INITED;

extern FuncResult_e spiDriver_getSingleScene(volatile SpiDriver_Params_t* params,
                                             spiDriver_ChipData_t** spiDriver_chipDataTmp,
                                             uint16_t* spiDriver_chipDataSizeTmp);
extern FuncResult_e spiDriver_getSingleSyncStep(spiDriver_ChipData_t** spiDriver_chipDataTmp,
                                                uint16_t* spiDriver_chipDataSizeTmp);
extern void spiDriver_UpdateCurrentData(const spiDriver_ChipData_t* const spiDriver_chipDataTmp,
                                        const uint16_t spiDriver_chipDataSizeTmp);

extern ContModeCfg_t contModeCfg;

/* Continuous mode thread function */
void* trigDataExecute(void* temp)
{
#if (CONT_MODE_DEBUG == 1)
    unsigned long index = 0ul;
#endif /* CONT_MODE_DEBUG */
    bool looping = true;
    contModeInterface_t rbuf;
    const contModeInterface_t out_msg = {.cmd = CONT_MODE_WORK, .mtype = CONT_MODE_DATA_READY};
    const int msgflg = 0666;
    (void)temp;
    /* get the message queue id for the key with value CONT_MODE_MSQ_KEY */
    CONT_PRINT("\nmsgget: Calling msgget(%#x,\%#o)\n", CONT_MODE_MSQ_KEY, msgflg);
    if ((msqid = msgget(CONT_MODE_MSQ_KEY, msgflg)) < 0) {
        perror("msgget");
        exit(1);
    } else {
        CONT_PRINT("msgget: msgget succeeded: msqid = %d\n", msqid);
    }
    while ( looping ) {
        CONT_PRINT("Trigger: Waiting for trigger data request\n");
        if (msgrcv(msqid, &rbuf, contModeInterface_size, CONT_MODE_REQUEST_DATA, 0) < 0) {
            CONT_PRINT("Trigger: msgrcv err:%x\n", errno);
            looping = false;
        } else {
            spiDriver_ChipData_t* spiDriver_chipDataTmp = NULL;
            uint16_t spiDriver_chipDataSizeTmp = 0;

            if ((rbuf.cmd == CONT_MODE_WORK) && (trigDataMode != CONT_MODE_NOT_INITED)) {

                CONT_PRINT("Trigger: WORK request to read data from ICs\n");
                if (contModeCfg.useAsyncSequence) {
                    spiDriver_getSingleSyncStep(&spiDriver_chipDataTmp, &spiDriver_chipDataSizeTmp);
                } else {
                    spiDriver_getSingleScene(NULL, &spiDriver_chipDataTmp, &spiDriver_chipDataSizeTmp);
                }
                spiDriver_UpdateCurrentData(spiDriver_chipDataTmp, spiDriver_chipDataSizeTmp);

                /* Signal about the new data's ready */
                CONT_PRINT("Trigger: Send data ready message %lu\n", index++);
                msgsnd(msqid, &out_msg, contModeInterface_size, IPC_CREAT);
            } else if (rbuf.cmd == CONT_MODE_EXIT) {
                CONT_PRINT("Trigger: exit signal received\n");
                looping = false;
            } else {
                CONT_PRINT("Trigger: PACKET???\n");
                /* Just continue if STOP */
            }
        }
    }
    CONT_PRINT("\nTrigger: Polling thread is Finished\n");
    trigDataMode = CONT_MODE_NOT_INITED;
    return NULL;
}

void spiDriver_InitTrigData(void)
{
    uint32_t msqid_counter = 0x10000;
    if (trigDataMode == CONT_MODE_NOT_INITED) {
        (void)pthread_create(&trigDataThreadID, NULL, &trigDataExecute, NULL);
        while((msqid == 0) && (msqid_counter-- != 0ul)) {
            ;
        }
        if (msqid_counter != 0ul) {
            CONT_PRINT("\nTrigger thread created\n");
            /* get the message queue id for the key with value CONT_MODE_MSQ_KEY */
            CONT_PRINT("\nmsgget: Calling msgget(%#x,\%#o)\n", CONT_MODE_MSQ_KEY, 0666);
            if ((msqid = msgget(CONT_MODE_MSQ_KEY, 0666)) < 0) {
                perror("msgget");
                exit(1);
            } else {
                CONT_PRINT("msgget: msgget succeeded: msqid = %d\n", msqid);
            }
            trigDataMode = CONT_MODE_IDLE;
        }
    }
}

void spiDriver_RunTrigData(void)
{
    contModeInterface_t msg;
    msg.cmd = CONT_MODE_WORK;
    msg.mtype = CONT_MODE_REQUEST_DATA;
    (void)msgsnd(msqid, &msg, contModeInterface_size, IPC_CREAT);
}

void spiDriver_StopTrigData(void)
{
    contModeInterface_t msg;
    msg.cmd = CONT_MODE_STOP;
    msg.mtype = CONT_MODE_REQUEST_DATA;
    (void)msgsnd(msqid, &msg, contModeInterface_size, IPC_CREAT);
}

void spiDriver_ExitTrigData(void)
{
    contModeInterface_t msg;
    msg.cmd = CONT_MODE_EXIT;
    msg.mtype = CONT_MODE_REQUEST_DATA;
    (void)msgsnd(msqid, &msg, contModeInterface_size, IPC_CREAT);
}

