/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/*
 * By default disable both asserts and log for this module.
 * This must be done before DebugP.h is included.
 */
#ifndef DebugP_ASSERT_ENABLED
#define DebugP_ASSERT_ENABLED 0
#endif
#ifndef DebugP_LOG_ENABLED
#define DebugP_LOG_ENABLED 0
#endif
#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC32XX.h>

#include <ti/drivers/i2c/I2CCC32XX.h>

#include <ti/devices/cc32xx/inc/hw_types.h>
#include <ti/devices/cc32xx/inc/hw_memmap.h>
#include <ti/devices/cc32xx/inc/hw_ocp_shared.h>
#include <ti/devices/cc32xx/driverlib/rom.h>
#include <ti/devices/cc32xx/driverlib/rom_map.h>
#include <ti/devices/cc32xx/driverlib/i2c.h>
#include <ti/devices/cc32xx/driverlib/prcm.h>
#include <ti/devices/cc32xx/inc/hw_i2c.h>
#include <ti/devices/cc32xx/driverlib/pin.h>

/* Pad configuration defines */
#define PAD_CONFIG_BASE (OCP_SHARED_BASE + OCP_SHARED_O_GPIO_PAD_CONFIG_0)
#define PAD_DEFAULT_STATE 0xC60 /* pad reset, plus set GPIO mode to free I2C */

#define ERROR_INTERRUPTS (I2C_MASTER_INT_NACK | \
        I2C_MASTER_INT_ARB_LOST)

#define DESIRED_INTERRUPTS (I2C_MASTER_INT_STOP | \
        ERROR_INTERRUPTS)

/* Prototypes */
static void I2CCC32XX_blockingCallback(I2C_Handle handle, I2C_Transaction *msg,
                                       bool transferStatus);
void         I2CCC32XX_cancel(I2C_Handle handle);
void         I2CCC32XX_reset();
void         I2CCC32XX_close(I2C_Handle handle);
int_fast16_t I2CCC32XX_control(I2C_Handle handle, uint_fast16_t cmd, void *arg);
void         I2CCC32XX_init(I2C_Handle handle);
I2C_Handle   I2CCC32XX_open(I2C_Handle handle, I2C_Params *params);
int_fast16_t I2CCC32XX_transfer(I2C_Handle handle,
                                I2C_Transaction *transaction, uint32_t timeout);

static void initHw(I2C_Handle handle);
static int postNotify(unsigned int eventType, uintptr_t eventArg,
                      uintptr_t clientArg);

static void primeReadBurst(I2CCC32XX_Object *object,
                           I2CCC32XX_HWAttrsV1 const *hwAttrs);
static void primeWriteBurst(I2CCC32XX_Object *object,
                            I2CCC32XX_HWAttrsV1 const *hwAttrs);
static void I2CCC32XX_primeTransfer(I2CCC32XX_Object          *object,
                                    I2CCC32XX_HWAttrsV1 const *hwAttrs,
                                    I2C_Transaction           *transaction);

/* I2C function table for I2CCC32XX implementation */
const I2C_FxnTable I2CCC32XX_fxnTable = {
    I2CCC32XX_cancel,
    I2CCC32XX_close,
    I2CCC32XX_control,
    I2CCC32XX_init,
    I2CCC32XX_open,
    I2CCC32XX_transfer
};

static const uint32_t bitRate[] = {
    false,  /*  I2C_100kHz = 0 */
    true    /*  I2C_400kHz = 1 */
};

/*
 *  ======== I2CC32XX_cancel ========
 */
void I2CCC32XX_cancel(I2C_Handle handle)
{
    I2CCC32XX_Object          *object = handle->object;
    I2CCC32XX_HWAttrsV1 const *hwAttrs = handle->hwAttrs;
    uintptr_t key;

    /* just return if no transfer is in progress */
    if (!object->headPtr) {
        return;
    }

    /* disable interrupts, send STOP to complete any transfer */
    key = HwiP_disable();
    MAP_I2CMasterIntDisableEx(hwAttrs->baseAddr, DESIRED_INTERRUPTS);
    MAP_I2CMasterControl(hwAttrs->baseAddr,
                         I2C_MASTER_CMD_FIFO_BURST_SEND_ERROR_STOP);

    /*
     * Sometimes after calling I2C_cancel, the clock line can be stuck
     * low even if we disable and re-enable the I2C peripheral.
     * Additionally we found that the data of a cancelled transaction
     * can remain stuck in the TX FIFO and get sent out in the next
     * transaction, even if we flush the TX FIFO. To avoid this
     * unwanted behavior we should hard reset the I2C peripheral
     */
    I2CCC32XX_reset();

    /* call the transfer callback for the current transfer, indicate failure */
    object->transferCallbackFxn(handle, object->currentTransaction, false);

    /* also dequeue and call the transfer callbacks for any queued transfers */
    while (object->headPtr != object->tailPtr) {
        object->headPtr = object->headPtr->nextPtr;
        object->transferCallbackFxn(handle, object->headPtr, false);
    }

    /* clean up object */
    object->currentTransaction = NULL;
    object->headPtr = NULL;
    object->tailPtr = NULL;
    object->mode = I2CCC32XX_IDLE_MODE;

    /* Re-initialize the hardware */
    initHw(handle);

    HwiP_restore(key);
}

/*
 *  ======== I2CCC32XX_close ========
 */
void I2CCC32XX_close(I2C_Handle handle)
{
    I2CCC32XX_Object          *object = handle->object;
    I2CCC32XX_HWAttrsV1 const *hwAttrs = handle->hwAttrs;
    uint32_t                  padRegister;

    /* Check to see if a I2C transaction is in progress */
    DebugP_assert(object->headPtr == NULL);

    /* Mask I2C interrupts */
    MAP_I2CMasterIntDisableEx(hwAttrs->baseAddr, DESIRED_INTERRUPTS);

    /* Disable the I2C Master */
    MAP_I2CMasterDisable(hwAttrs->baseAddr);

    /* Disable I2C module clocks */
    Power_releaseDependency(PowerCC32XX_PERIPH_I2CA0);

    Power_unregisterNotify(&(object->notifyObj));

    /* Restore pin pads to their reset states */
    padRegister = (PinToPadGet((hwAttrs->clkPin) & 0xff)<<2) + PAD_CONFIG_BASE;
    HWREG(padRegister) = PAD_DEFAULT_STATE;
    padRegister = (PinToPadGet((hwAttrs->dataPin) & 0xff)<<2) + PAD_CONFIG_BASE;
    HWREG(padRegister) = PAD_DEFAULT_STATE;

    if (object->hwiHandle) {
        HwiP_delete(object->hwiHandle);
        object->hwiHandle = NULL;
    }
    if (object->mutex) {
        SemaphoreP_delete(object->mutex);
        object->mutex = NULL;
    }
    /* Destruct the Semaphore */
    if (object->transferComplete) {
        SemaphoreP_delete(object->transferComplete);
        object->transferComplete = NULL;
    }

    object->isOpen = false;

    DebugP_log1("I2C: Object closed 0x%x", hwAttrs->baseAddr);

    return;
}

/*
 *  ======== I2CCC32XX_completeTransfer ========
 */
static void I2CCC32XX_completeTransfer(I2C_Handle handle)
{
    /* Get the pointer to the object */
    I2CCC32XX_Object *object = handle->object;

    DebugP_log1("I2C:(%p) ISR Transfer Complete",
                ((I2CCC32XX_HWAttrsV1 const *)(handle->hwAttrs))->baseAddr);

    /* See if we need to process any other transactions */
    if (object->headPtr == object->tailPtr) {

        /* No other transactions need to occur */
        object->headPtr = NULL;
        object->tailPtr = NULL;

        /*
         * Allow callback to run. If in CALLBACK mode, the application
         * may initiate a transfer in the callback which will call
         * primeTransfer().
         */
        object->transferCallbackFxn(handle, object->currentTransaction,
                                    (object->mode == I2CCC32XX_IDLE_MODE));

        /* release constraint since transaction is done */
        Power_releaseConstraint(PowerCC32XX_DISALLOW_LPDS);

        DebugP_log1("I2C:(%p) ISR No other I2C transaction in queue",
                    ((I2CCC32XX_HWAttrsV1 const *)(handle->hwAttrs))->baseAddr);
    }
    else {
        /* Another transfer needs to take place */
        object->headPtr = object->headPtr->nextPtr;

        /*
         * Allow application callback to run. The application may
         * initiate a transfer in the callback which will add an
         * additional transfer to the queue.
         */
        object->transferCallbackFxn(handle, object->currentTransaction,
                                    (object->mode == I2CCC32XX_IDLE_MODE));

        DebugP_log2("I2C:(%p) ISR Priming next I2C transaction (%p) from queue",
                    ((I2CCC32XX_HWAttrsV1 const *)(handle->hwAttrs))->baseAddr,
                    (uintptr_t)object->headPtr);

        I2CCC32XX_primeTransfer(object,
                                (I2CCC32XX_HWAttrsV1 const *)handle->hwAttrs,
                                object->headPtr);
    }
}

/*
 *  ======== I2CCC32XX_control ========
 *  @pre    Function assumes that the handle is not NULL
 */
int_fast16_t I2CCC32XX_control(I2C_Handle handle, uint_fast16_t cmd, void *arg)
{
    /* No implementation yet */
    return (I2C_STATUS_UNDEFINEDCMD);
}

/*
 *  ======== I2CCC32XX_hwiFxn ========
 *  Hwi interrupt handler to service the I2C peripheral
 *
 *  The handler is a generic handler for a I2C object.
 */
static void I2CCC32XX_hwiFxn(uintptr_t arg)
{
    /* Get the pointer to the object and hwAttrs */
    I2CCC32XX_Object          *object = ((I2C_Handle)arg)->object;
    I2CCC32XX_HWAttrsV1 const *hwAttrs = ((I2C_Handle)arg)->hwAttrs;
    uint32_t                   intStatus;
    size_t                     originalWriteCount;

    /* Get interrupt status */
    intStatus = MAP_I2CMasterIntStatusEx(hwAttrs->baseAddr, true);

    /* Clear interrupt source to avoid additional interrupts */
    MAP_I2CMasterIntClearEx(hwAttrs->baseAddr, intStatus);

    /* Sometimes we get an 'ghost' interrupt with no status */
    if(intStatus == 0x00000000) {
        /*
         * TODO figure out why we get this ghost interrupt after
         * refilling/draining the TX/RX FIFOs
         */
        return;
    }

    /*
     * Don't enter this if statement if this interrupt was caused
     * by an ERROR_INTERRUPT. But if object->mode == ERROR, then
     * enter the if statement and complete the transfer.
     */
    if (!(intStatus & ERROR_INTERRUPTS) || (object->mode == I2CCC32XX_ERROR)) {
        switch (object->mode) {
        /*
         * ERROR case is OK because if an Error is detected, a STOP bit was
         * sent by the previous hardware interrupt--which in turn called this
         * interrupt. Now we will post the transferComplete semaphore to
         * unblock the I2C_transfer function
         */
        case I2CCC32XX_ERROR:
        case I2CCC32XX_TIMEOUT:
        case I2CCC32XX_IDLE_MODE:
            I2CCC32XX_completeTransfer((I2C_Handle) arg);
            break;
        case I2CCC32XX_WRITE_MODE:
            if (object->burstWriteCountIdx) {
                /* The TX FIFO should be empty if we reach here. */
                if(!(MAP_I2CFIFOStatus(hwAttrs->baseAddr)
                        & I2C_FIFOSTATUS_TXFE)) {
                    /* TX FIFO was not empty when it should have been! */
                    object->mode = I2CCC32XX_ERROR;
                }

                /*
                 * Fill the TX FIFO until it's full or
                 * until we have no data left to give it
                 */
                while(object->burstWriteCountIdx) {
                    if(!MAP_I2CFIFODataPutNonBlocking(hwAttrs->baseAddr,
                                                      *(object->writeBufIdx))){

                        /* The FIFO is full, break out of while loop */
                        break;
                    }
                    object->writeBufIdx++;
                    object->totalWriteCountIdx--;
                    object->burstWriteCountIdx--;
                }

                /* Clear TX FIFO empty interrupt */
                MAP_I2CMasterIntClearEx(hwAttrs->baseAddr,
                                        I2C_MASTER_INT_TX_FIFO_EMPTY);
            }
            else if(!object->burstWriteCountIdx && object->totalWriteCountIdx) {
                /*
                 * If we reached here, we have finished writing the write burst.
                 * But we still have more data to write for the overall
                 * transaction! Start another burst.
                 */
                originalWriteCount = object->totalWriteCountIdx;
                primeWriteBurst(object, hwAttrs);
                if(object->totalReadCountIdx || (originalWriteCount > 255)) {
                    /*
                     * Perform N FIFO-serviced TRANSMITs. Master remains in
                     * TRANSMIT state. If we have data to read, we will change
                     * to RECEIVE state in the hwiFxn. N is defined as what we
                     * passed into I2CMasterBurstLengthSet().
                     * See TRM decoding of I2CMCS register.
                     */
                    MAP_I2CMasterControl(hwAttrs->baseAddr,
                                         I2C_MASTER_CMD_FIFO_BURST_SEND_CONT);
                }
                else {
                    /*
                     * We are only writing. Perform N FIFO-serviced TRANSMITs
                     * and send a STOP condition. Master goes to idle state.
                     * N is defined as what we passed into I2CMasterBurstLengthSet().
                     * See TRM decoding of I2CMCS register.
                     */
                    MAP_I2CMasterControl(hwAttrs->baseAddr,
                                         I2C_MASTER_CMD_FIFO_BURST_SEND_FINISH);
                }
            }
            else if(!object->totalWriteCountIdx && !object->totalReadCountIdx) {
                /*
                 * If we reached here, we have finished writing and
                 * and we have no data to read. So we can complete
                 * the transfer.
                 */
                object->mode = I2CCC32XX_IDLE_MODE;

                /* Disable the TX FIFO EMPTY interrupt */
                MAP_I2CMasterIntDisableEx(hwAttrs->baseAddr,
                                          I2C_MASTER_INT_TX_FIFO_EMPTY);
                /*
                 * The STOP bit should usually occur in another interrupt, but
                 * it's possible it occurred in this one (TX FIFO EMPTY).
                 * If the STOP interrupt is already detected
                 * complete the transfer now. If not, then completeTransfer
                 * will be called in the next interrupt since
                 * object->mode == IDLE_MODE
                 */
                if((intStatus & I2C_MASTER_INT_STOP)) {
                    I2CCC32XX_completeTransfer((I2C_Handle) arg);
                }
            }
            else {
                /*
                 * If we reached here, we have finished writing and
                 * now we need to read some data.
                 */
                object->mode = I2CCC32XX_READ_MODE;

                /* Disable TX FIFO FULL interrupt */
                MAP_I2CMasterIntDisableEx(hwAttrs->baseAddr,
                                      I2C_MASTER_INT_TX_FIFO_EMPTY);

                primeReadBurst(object, hwAttrs);

                /*
                 * If we are reading more than 255 bytes, don't send a STOP
                 * condition. The STOP condition will be sent in another burst
                 * started in the hwiFxn.
                 */
                if(object->totalReadCountIdx > 255) {
                    /*
                     * Send START condition followed by N FIFO-serviced
                     * RECEIVE operations. Master goes to master receive state.
                     * See TRM decoding of I2CMCS register.
                     */
                    MAP_I2CMasterControl(hwAttrs->baseAddr,
                                         I2C_MASTER_CMD_FIFO_BURST_RECEIVE_START);
                }
                else {
                    /*
                     * Send START condition followed by N FIFO-serviced
                     * RECEIVE operations with a negative ACK on the last
                     * RECEIVE and STOP condition (master remains in idle state)
                     * See TRM decoding of I2CMCS register.
                     */
                    MAP_I2CMasterControl(hwAttrs->baseAddr,
                                         I2C_MASTER_CMD_FIFO_SINGLE_RECEIVE);
                }
            }
            break;
        case I2CCC32XX_READ_MODE:
            /* If we have more data to read, empty the RX FIFO */
            if(object->burstReadCountIdx){
                /*
                 * Read from the RX FIFO until it's empty or until we
                 * have no data left to read
                 */
                while(object->burstReadCountIdx) {
                    if(!MAP_I2CFIFODataGetNonBlocking(hwAttrs->baseAddr,
                                                      object->readBufIdx)){
                        break;
                    }
                    else {
                        object->readBufIdx++;
                        object->totalReadCountIdx--;
                        object->burstReadCountIdx--;
                    }
                }

                /* If we still have data left to read, set up the interrupts */
                if(object->burstReadCountIdx)
                {
                    /*
                     * If we still have between 1 and 8 bytes to
                     * read, set the RX FIFO REQ interrupt to trigger when we
                     * finish burstReadCountIdx bytes.
                     */
                    if(object->burstReadCountIdx < 8) {
                        MAP_I2CRxFIFOConfigSet(hwAttrs->baseAddr,
                                               I2C_FIFO_CFG_RX_MASTER |
                                               (object->burstReadCountIdx
                                                       << I2C_FIFOCTL_RXTRIG_S));
                    }
                    /*
                     * Else we are reading more than 8 bytes. Set the RX FIFO
                     * REQ interrupt to trigger when the RX FIFO is full
                     */
                    else {
                        MAP_I2CRxFIFOConfigSet(hwAttrs->baseAddr,
                                               I2C_FIFO_CFG_RX_MASTER |
                                               I2C_FIFO_CFG_RX_TRIG_8);
                    }
                }

                MAP_I2CMasterIntClearEx(hwAttrs->baseAddr,
                                        I2C_MASTER_INT_RX_FIFO_REQ);
            }
            /*
             * It is important that this case is not an 'else' because
             * the above block can modify object->totalReadCountIdx
             */
            if(!object->burstReadCountIdx && object->totalReadCountIdx) {
                /*
                 * If we reached here, we have finished read the read burst.
                 * But we still have more data to read for the overall
                 * transaction! Start another read burst.
                 */
                primeReadBurst(object, hwAttrs);

                /*
                 * If we are reading more than 255 bytes, don't send a STOP
                 * condition. The STOP condition will be sent in another burst
                 * started in the hwiFxn.
                 */
                if(object->totalReadCountIdx > 255) {
                    /*
                     * Perform N FIFO-serviced RECEIVE operations.
                     * Master remains master receive state.
                     * See TRM decoding of I2CMCS register.
                     */
                    MAP_I2CMasterControl(hwAttrs->baseAddr,
                                         I2C_MASTER_CMD_FIFO_BURST_RECEIVE_CONT);
                }
                else {
                    /*
                     * Perform N FIFO-serviced RECEIVE operations with a
                     * negative ACK on the last RECEIVE and send STOP condition.
                     * Master goes to idle state.
                     * See TRM decoding of I2CMCS register.
                     */
                    MAP_I2CMasterControl(hwAttrs->baseAddr,
                                         I2C_MASTER_CMD_FIFO_BURST_RECEIVE_FINISH);
                }
            }
            /*
             * It is important that this case is not an 'else' because
             * the above block can modify object->totalReadCountIdx
             */
            if(!object->burstReadCountIdx && !object->totalReadCountIdx) {
                /* Read mode --> Idle mode */
                object->mode = I2CCC32XX_IDLE_MODE;

                MAP_I2CMasterIntDisableEx(hwAttrs->baseAddr,
                                          I2C_MASTER_INT_RX_FIFO_REQ);

                /*
                 * The STOP interrupt probably occurred in the interrupt we are
                 * in, but it's also possible that it's about to occur in a
                 * future one. If the STOP interrupt is already detected
                 * complete the transfer now.
                 */
                if((intStatus & I2C_MASTER_INT_STOP)) {
                    I2CCC32XX_completeTransfer((I2C_Handle) arg);
                }
            }

            break;
        }
    }
    else {
        object->mode = I2CCC32XX_ERROR;

        MAP_I2CMasterIntDisableEx(hwAttrs->baseAddr,
                                  I2C_MASTER_INT_TX_FIFO_EMPTY |
                                  I2C_MASTER_INT_RX_FIFO_REQ);

        if((intStatus & I2C_MASTER_INT_STOP)) {
            I2CCC32XX_completeTransfer((I2C_Handle) arg);
        }
    }
    return;
}

/*
 *  ======== I2CCC32XX_init ========
 */
void I2CCC32XX_init(I2C_Handle handle)
{
    /*
     * Relying on ELF to set
     * ((I2CCC32XX_Object *)(handle->object))->isOpen = false
     */
}

/*
 *  ======== I2CCC32XX_open ========
 */
I2C_Handle I2CCC32XX_open(I2C_Handle handle, I2C_Params *params)
{
    uintptr_t                  key;
    I2CCC32XX_Object          *object = handle->object;
    I2CCC32XX_HWAttrsV1 const *hwAttrs = handle->hwAttrs;
    SemaphoreP_Params          semParams;
    HwiP_Params                hwiParams;
    uint16_t                   pin;
    uint16_t                   mode;

    /* Check for valid bit rate */
    if (params->bitRate > I2C_400kHz) {
        return (NULL);
    }

    /* Determine if the device index was already opened */
    key = HwiP_disable();
    if(object->isOpen == true){
        HwiP_restore(key);
        return (NULL);
    }

    /* Mark the handle as being used */
    object->isOpen = true;
    HwiP_restore(key);

    /* Save parameters */
    object->transferMode = params->transferMode;
    object->transferCallbackFxn = params->transferCallbackFxn;
    object->bitRate = params->bitRate;

    /* Enable the I2C module clocks */
    Power_setDependency(PowerCC32XX_PERIPH_I2CA0);

    /* In case of app restart: disable I2C module, clear interrupt at NVIC */
    MAP_I2CMasterDisable(hwAttrs->baseAddr);
    HwiP_clearInterrupt(hwAttrs->intNum);

    pin = hwAttrs->clkPin & 0xff;
    mode = (hwAttrs->clkPin >> 8) & 0xff;
    MAP_PinTypeI2C((unsigned long)pin, (unsigned long)mode);

    pin = hwAttrs->dataPin & 0xff;
    mode = (hwAttrs->dataPin >> 8) & 0xff;
    MAP_PinTypeI2C((unsigned long)pin, (unsigned long)mode);

    Power_registerNotify(&(object->notifyObj), PowerCC32XX_AWAKE_LPDS,
                         postNotify, (uint32_t)handle);

    HwiP_Params_init(&hwiParams);
    hwiParams.arg = (uintptr_t)handle;
    hwiParams.priority = hwAttrs->intPriority;
    object->hwiHandle = HwiP_create(hwAttrs->intNum,
                                    I2CCC32XX_hwiFxn,
                                    &hwiParams);
    if (object->hwiHandle == NULL) {
        I2CCC32XX_close(handle);
        return (NULL);
    }

    /*
     * Create threadsafe handles for this I2C peripheral
     * Semaphore to provide exclusive access to the I2C peripheral
     */
    SemaphoreP_Params_init(&semParams);
    semParams.mode = SemaphoreP_Mode_BINARY;
    object->mutex = SemaphoreP_create(1, &semParams);
    if (object->mutex == NULL) {
        I2CCC32XX_close(handle);
        return (NULL);
    }

    /*
     * Store a callback function that posts the transfer complete
     * semaphore for synchronous mode
     */
    if (object->transferMode == I2C_MODE_BLOCKING) {
        /*
         * Semaphore to cause the waiting task to block for the I2C
         * to finish
         */
        object->transferComplete = SemaphoreP_create(0, &semParams);
        if (object->transferComplete == NULL) {
            I2CCC32XX_close(handle);
            return (NULL);
        }

        /* Store internal callback function */
        object->transferCallbackFxn = I2CCC32XX_blockingCallback;
    }
    else {
        /* Check to see if a callback function was defined for async mode */
        DebugP_assert(object->transferCallbackFxn != NULL);
    }

    /* Specify the idle state for this I2C peripheral */
    object->mode = I2CCC32XX_IDLE_MODE;

    /* Clear the head pointer */
    object->headPtr = NULL;
    object->tailPtr = NULL;

    DebugP_log1("I2C: Object created 0x%x", hwAttrs->baseAddr);

    /* Set the I2C configuration */
    initHw(handle);

    /* Return the address of the handle */
    return (handle);
}

/*
 *  ======== primeReadBurst =======
 */
static void primeReadBurst(I2CCC32XX_Object *object,
                           I2CCC32XX_HWAttrsV1 const *hwAttrs)
{
    /* Update the I2C mode */
    object->mode = I2CCC32XX_READ_MODE;

    /* Disable TX FIFO EMTPY interrupt and RX FIFO REQ interrupt*/
    MAP_I2CMasterIntDisableEx(hwAttrs->baseAddr,
                              I2C_MASTER_INT_RX_FIFO_REQ);

    /* Flush the RX FIFO and wait for it to flush */
    MAP_I2CRxFIFOFlush(hwAttrs->baseAddr);
    while(!(MAP_I2CFIFOStatus(hwAttrs->baseAddr) & I2C_FIFOSTATUS_RXFE)) {}

    /* Specify the I2C slave address in receive mode */
    MAP_I2CMasterSlaveAddrSet(hwAttrs->baseAddr,
                              object->currentTransaction->slaveAddress,
                              true);

    /*
     * Set peripheral to expect burstReadCountIdx bytes. The I2CMBLEN burst
     * register only holds a value from 0 to 255, so if our transaction involves
     * more than 255 bytes we will have to setup a new burst in the hwiFxn
     * after we read the first 255 bytes. We use object->burstReadCount
     * to keep track of our progress for the current burst transaction. We use
     * object->totalReadCountIdx to keep track of our progress for the
     * overall read transaction.
     */
    if(object->totalReadCountIdx > 255) {
        object->burstReadCountIdx = 255;
    }
    else {
        object->burstReadCountIdx = object->totalReadCountIdx;
    }

    MAP_I2CMasterBurstLengthSet(hwAttrs->baseAddr,
                                object->burstReadCountIdx);

    /*
     * If we are reading less than 8 bytes, set the RX FIFO REQ
     * interrupt to trigger when we finish burstReadCountIdx bytes.
     */
    if(object->burstReadCountIdx < 8) {
        MAP_I2CRxFIFOConfigSet(hwAttrs->baseAddr,
                               I2C_FIFO_CFG_RX_MASTER |
                               (object->burstReadCountIdx <<
                                       I2C_FIFOCTL_RXTRIG_S));
    }
    /*
     * Else we are reading more than 8 bytes. set the RX FIFO REQ
     * interrupt to trigger when full
     */
    else {
        MAP_I2CRxFIFOConfigSet(hwAttrs->baseAddr,
                               I2C_FIFO_CFG_RX_MASTER |
                               I2C_FIFO_CFG_RX_TRIG_8);
    }

    /* Clear and enable relevant interrupts */
    MAP_I2CMasterIntClearEx(hwAttrs->baseAddr,
                            I2C_MASTER_INT_RX_FIFO_REQ);
    MAP_I2CMasterIntEnableEx(hwAttrs->baseAddr,
                             I2C_MASTER_INT_RX_FIFO_REQ);
}

/*
 *  ======== primeWriteBurst =======
 */
static void primeWriteBurst(I2CCC32XX_Object *object,
                            I2CCC32XX_HWAttrsV1 const *hwAttrs)
{

    /* Update the I2C mode */
    object->mode = I2CCC32XX_WRITE_MODE;

    /* Disable the TX FIFO empty interrupt */
    MAP_I2CMasterIntDisableEx(hwAttrs->baseAddr,
                              I2C_MASTER_INT_TX_FIFO_EMPTY);

    /* Flush the TX FIFO and wait for it to finish flushing */
    MAP_I2CTxFIFOFlush(hwAttrs->baseAddr);
    while(!(MAP_I2CFIFOStatus(hwAttrs->baseAddr) & I2C_FIFOSTATUS_TXFE)) {}

    /* Specify the I2C slave address */
    MAP_I2CMasterSlaveAddrSet(hwAttrs->baseAddr,
                              object->currentTransaction->slaveAddress,
                              false);

    /*
     * Set peripheral to send burstWriteCountIdx bytes. The I2CMBLEN burst
     * register only holds a value from 0 to 255, so if our transaction involves
     * more than 255 bytes we will have to setup a new burst in the hwiFxn
     * after we service the first 255 bytes. We use object->burstWriteCount
     * to keep track of our progress for the current burst transaction. We use
     * object->totalWriteCountIdx to keep track of our progress for the
     * overall write transaction.
     */
    if(object->totalWriteCountIdx > 255) {
        object->burstWriteCountIdx = 255;
    }
    else {
        object->burstWriteCountIdx = object->totalWriteCountIdx;
    }

    MAP_I2CMasterBurstLengthSet(hwAttrs->baseAddr,
                                object->burstWriteCountIdx);

    /*
     * Fill the FIFO until it's full or until we
     * have no data left to give it
     */
    while(object->burstWriteCountIdx) {
        if(!MAP_I2CFIFODataPutNonBlocking(hwAttrs->baseAddr,
                                          *(object->writeBufIdx))){
            /* The FIFO is full, break out of while loop */
            break;
        }
        object->writeBufIdx++;
        object->totalWriteCountIdx--;
        object->burstWriteCountIdx--;
    }

    MAP_I2CMasterIntClearEx(hwAttrs->baseAddr,
                            I2C_MASTER_INT_TX_FIFO_EMPTY);
    MAP_I2CMasterIntEnableEx(hwAttrs->baseAddr,
                             I2C_MASTER_INT_TX_FIFO_EMPTY);
}

/*
 *  ======== I2CCC32XX_primeTransfer =======
 */
static void I2CCC32XX_primeTransfer(I2CCC32XX_Object *object,
                                    I2CCC32XX_HWAttrsV1 const *hwAttrs,
                                    I2C_Transaction *transaction)
{
    /* Counter to keep track of totalWriteCount before we fill the TX FIFO */
    size_t originalWriteCount = transaction->writeCount;

    /* Store the new internal counters and pointers */
    object->currentTransaction = transaction;

    object->writeBufIdx = transaction->writeBuf;
    object->totalWriteCountIdx = transaction->writeCount;

    object->readBufIdx = transaction->readBuf;
    object->totalReadCountIdx = transaction->readCount;

    DebugP_log2("I2C:(%p) Starting transaction to slave: 0x%x",
                hwAttrs->baseAddr,
                object->currentTransaction->slaveAddress);

    /* Start transfer in Transmit mode */
    if (object->totalWriteCountIdx) {
        primeWriteBurst(object, hwAttrs);

        /*
         * If we are sending more than 255 bytes in this transaction, or if we
         * are going to read data, don't send a STOP bit.
         */
        if(object->totalReadCountIdx || (originalWriteCount > 255)) {
            /*
             * Send a START condition followed by N FIFO-serviced TRANSMITs. Master
             * changes to TRANSMIT state. If we have data to read, we will change to
             * RECEIVE state in the hwiFxn. N is defined as what we passed
             * into I2CMasterBurstLengthSet(). See TRM decoding of I2CMCS register.
             */
            MAP_I2CMasterControl(hwAttrs->baseAddr,
                                 I2C_MASTER_CMD_FIFO_BURST_SEND_START);
        }
        else {
            /*
             * We are only writing. Send a START condition followed by
             * N FIFO-serviced TRANSMITs and a STOP condition. Master remains
             * in idle state. See TRM decoding of I2CMCS register. N is defined
             * as what we passed into I2CMasterBurstLengthSet().
             */
            MAP_I2CMasterControl(hwAttrs->baseAddr,
                                 I2C_MASTER_CMD_FIFO_SINGLE_SEND);
        }
    }
    /* Start transfer in Receive mode */
    else {
        primeReadBurst(object, hwAttrs);

        /*
         * If we are reading more than 255 bytes, don't send a STOP condition.
         * The STOP condition will be sent in another burst started in the
         * hwiFxn.
         */
        if(object->totalReadCountIdx > 255) {
            /*
             * Send START condition followed by N FIFO-serviced
             * RECEIVE operations. Master goes to master receive state.
             * See TRM decoding of I2CMCS register.
             */
            MAP_I2CMasterControl(hwAttrs->baseAddr,
                                 I2C_MASTER_CMD_FIFO_BURST_RECEIVE_START);
        }
        else {
            /*
             * Send START condition followed by N FIFO-serviced
             * RECEIVE operations with a negative ACK on the last RECEIVE
             * and STOP condition (master remains in idle state)
             * See TRM decoding of I2CMCS register.
             */
            MAP_I2CMasterControl(hwAttrs->baseAddr,
                                 I2C_MASTER_CMD_FIFO_SINGLE_RECEIVE);
        }
    }
}

/*
 *  ======== I2CC32XX_reset ========
 */
void I2CCC32XX_reset()
{
    PRCMPeripheralReset(PRCM_I2CA0);
    while(!PRCMPeripheralStatusGet(PRCM_I2CA0)) {}
}

/*
 *  ======== I2CCC32XX_transfer ========
 */
int_fast16_t I2CCC32XX_transfer(I2C_Handle handle, I2C_Transaction *transaction, uint32_t timeout)
{
    uintptr_t                  key;
    int_fast16_t               ret = I2C_STATUS_ERROR;
    I2CCC32XX_Object          *object = handle->object;
    I2CCC32XX_HWAttrsV1 const *hwAttrs = handle->hwAttrs;

    /* Check if anything needs to be written or read */
    if ((!transaction->writeCount) && (!transaction->readCount)) {
        /* Nothing to write or read */
        return (ret);
    }

    key = HwiP_disable();

    if (object->transferMode == I2C_MODE_CALLBACK) {
        /* Check if a transfer is in progress */
        if (object->headPtr) {
            /* Transfer in progress */

            /*
             * Update the message pointed by the tailPtr to point to the next
             * message in the queue
             */
            object->tailPtr->nextPtr = transaction;

            /* Update the tailPtr to point to the last message */
            object->tailPtr = transaction;

            /* I2C is still being used */
            HwiP_restore(key);
            return (I2C_STATUS_SUCCESS);
        }
    }

    /* Store the headPtr indicating I2C is in use */
    object->headPtr = transaction;
    object->tailPtr = transaction;

    HwiP_restore(key);

    /* Get the lock for this I2C handle */
    if (SemaphoreP_pend(object->mutex, SemaphoreP_NO_WAIT)
            == SemaphoreP_TIMEOUT) {

        /* An I2C_transfer() may complete before the calling thread post the
         * mutex due to preemption. We must not block in this case. */
        if (object->transferMode == I2C_MODE_CALLBACK) {
            return (I2C_STATUS_ERROR);
        }

        /* Wait for the I2C lock. If it times-out before we retrieve it, then
         * return I2C_STATUS_TIMEOUT to cancel the transaction. */
        if(SemaphoreP_pend(object->mutex, timeout) == SemaphoreP_TIMEOUT) {
            return (I2C_STATUS_TIMEOUT);
        }
    }

    if (object->transferMode == I2C_MODE_BLOCKING) {
       /*
        * In the case of a timeout, It is possible for the timed-out transaction
        * to call the hwi function and post the object->transferComplete Semaphore
        * To clear this, we simply do a NO_WAIT pend on (binary)
        * object->transferComplete, so that it resets the Semaphore count.
        */
        SemaphoreP_pend(object->transferComplete, SemaphoreP_NO_WAIT);
    }

    Power_setConstraint(PowerCC32XX_DISALLOW_LPDS);

    /*
     * I2CCC32XX_primeTransfer is a longer process and
     * protection is needed from the I2C interrupt
     */
    HwiP_disableInterrupt(hwAttrs->intNum);
    I2CCC32XX_primeTransfer(object, hwAttrs, transaction);
    HwiP_enableInterrupt(hwAttrs->intNum);

    if (object->transferMode == I2C_MODE_BLOCKING) {
        DebugP_log1("I2C:(%p) Pending on transferComplete semaphore",
                    hwAttrs->baseAddr);
        /*
         * Wait for the transfer to complete here.
         * It's OK to block from here because the I2C's Hwi will unblock
         * upon errors
         */

        if (SemaphoreP_pend(object->transferComplete, timeout) == SemaphoreP_TIMEOUT) {
            /*
             * Timeout occurred, set current state to TIMEOUT and
             * return status to I2C_STATUS_TIMEOUT
             */
            DebugP_log1("I2C:(%p) Transfer timed-out!", hwAttrs->baseAddr);

            object->mode = I2CCC32XX_TIMEOUT;
            ret = I2C_STATUS_TIMEOUT;

            /* Disable interrupts, send STOP to complete any transfer */
            key = HwiP_disable();
            MAP_I2CMasterIntDisableEx(hwAttrs->baseAddr, DESIRED_INTERRUPTS);
            MAP_I2CMasterControl(hwAttrs->baseAddr,
                                 I2C_MASTER_CMD_FIFO_BURST_SEND_ERROR_STOP);

            /*
             * Since we timed-out, we want to cancel any existing transaction.
             * But because we run the risk of leaving the I2C hardware in a bad
             * state, we need to hard-reset the peripheral
             */
            I2CCC32XX_reset();

            /* Re-initialize the hardware */
            initHw(handle);

            HwiP_restore(key);
        }

        DebugP_log1("I2C:(%p) Transaction completed",
                    hwAttrs->baseAddr);

        /* Hwi handle has posted a 'transferComplete' check for Errors */
        if (object->mode == I2CCC32XX_IDLE_MODE) {
            DebugP_log1("I2C:(%p) Transfer OK", hwAttrs->baseAddr);
            ret = I2C_STATUS_SUCCESS;
        }
    }
    else {
        /* Always return true if in Asynchronous mode */
        ret = I2C_STATUS_SUCCESS;
    }

    /* Release the lock for this particular I2C handle */
    SemaphoreP_post(object->mutex);

    /* Return the number of bytes transfered by the I2C */
    return (ret);
}

/*
 *  ======== I2CCC32XX_blockingCallback ========
 */
static void I2CCC32XX_blockingCallback(I2C_Handle handle,
                                       I2C_Transaction *msg,
                                       bool transferStatus)
{
    I2CCC32XX_Object  *object = handle->object;

    DebugP_log1("I2C:(%p) posting transferComplete semaphore",
                ((I2CCC32XX_HWAttrsV1 const *)(handle->hwAttrs))->baseAddr);

    /* Indicate transfer complete */
    SemaphoreP_post(object->transferComplete);
}

/*
 *  ======== initHw ========
 */
static void initHw(I2C_Handle handle)
{
    ClockP_FreqHz              freq;
    I2CCC32XX_Object          *object = handle->object;
    I2CCC32XX_HWAttrsV1 const *hwAttrs = handle->hwAttrs;
    uint32_t                   ulRegVal;

    /*
     *  Take I2C hardware semaphore.  This is needed when coming out
     *  of LPDS.  This is done in initHw() instead of postNotify(), in
     *  case no I2C is open when coming out of LPDS, so that the open()
     *  call will take the semaphore.
     */
    ulRegVal = HWREG(0x400F7000);
    ulRegVal = (ulRegVal & ~0x3) | 0x1;
    HWREG(0x400F7000) = ulRegVal;

    ClockP_getCpuFreq(&freq);
    MAP_I2CMasterInitExpClk(hwAttrs->baseAddr, freq.lo,
                            bitRate[object->bitRate]);


    /* Disable select I2C interrupts */
    MAP_I2CMasterIntDisableEx(hwAttrs->baseAddr,
                              I2C_MASTER_INT_DATA |
                              I2C_MASTER_INT_TX_FIFO_EMPTY |
                              I2C_MASTER_INT_RX_FIFO_REQ |
                              I2C_MASTER_INT_START);

    /* Flush the FIFOs. They must be empty before re-assignment */
    MAP_I2CTxFIFOFlush(hwAttrs->baseAddr);
    MAP_I2CRxFIFOFlush(hwAttrs->baseAddr);

    /* Set TX and RX FIFOs to master mode */
    MAP_I2CTxFIFOConfigSet(hwAttrs->baseAddr,
                           I2C_FIFO_CFG_TX_MASTER);
    MAP_I2CRxFIFOConfigSet(hwAttrs->baseAddr,
                           I2C_FIFO_CFG_RX_MASTER);

    /* Clear any pending interrupts */
    MAP_I2CMasterIntClearEx(hwAttrs->baseAddr,
                            DESIRED_INTERRUPTS |
                            I2C_MASTER_INT_TX_FIFO_EMPTY |
                            I2C_MASTER_INT_RX_FIFO_REQ);

    /* Enable desired interrupts */
    MAP_I2CMasterIntEnableEx(hwAttrs->baseAddr,
                             DESIRED_INTERRUPTS);

    /* Enable the I2C Master for operation */
    MAP_I2CMasterEnable(hwAttrs->baseAddr);
}

/*
 *  ======== postNotify ========
 *  This functions is called to notify the I2C driver of an ongoing transition
 *  out of LPDS mode.
 *  clientArg should be pointing to a hardware module which has already
 *  been opened.
 */
static int postNotify(unsigned int eventType,
                      uintptr_t eventArg, uintptr_t clientArg)
{
    /* Reconfigure the hardware when returning from LPDS */
    initHw((I2C_Handle)clientArg);

    return (Power_NOTIFYDONE);
}
