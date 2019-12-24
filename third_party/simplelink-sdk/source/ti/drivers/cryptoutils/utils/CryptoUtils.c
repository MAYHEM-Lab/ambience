/*
 * Copyright (c) 2019, Texas Instruments Incorporated
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
#include <string.h>

#include <ti/drivers/cryptoutils/utils/CryptoUtils.h>

/*
 *  ======== CryptoUtils_buffersMatch ========
 */
bool CryptoUtils_buffersMatch(const void *buffer0,
                              const void *buffer1,
                              size_t bufferByteLength) {
    uint8_t tempResult = 0;
    size_t i;

    /* XOR each byte of the buffer together and OR the results.
     * If the OR'd result is non-zero, the buffers do not match.
     * There is no branch based on the content of the buffers here to avoid
     * timing attacks.
     */
    for (i = 0; i < bufferByteLength; i++) {
        tempResult |= ((uint8_t *)buffer0)[i] ^ ((uint8_t *)buffer1)[i];
    }

    return tempResult == 0 ? true : false;
}

/*
 *  ======== CryptoUtils_buffersMatchWordAligned ========
 */
bool CryptoUtils_buffersMatchWordAligned(const uint32_t *buffer0,
                                         const uint32_t *buffer1,
                                         size_t bufferByteLength) {
    uint32_t tempResult = 0;
    size_t i;

    /* We could skip the branch and just set tempResult equal to the
     * statement below for the same effect but this is more explicit.
     */
    if (bufferByteLength % sizeof(uint32_t) != 0) {
        return false;
    }

    /* XOR each 32-bit word of the buffer together and OR the results.
     * If the OR'd result is non-zero, the buffers do not match.
     * There is no branch based on the content of the buffers here to avoid
     * timing attacks.
     */
    for (i = 0; i < bufferByteLength / sizeof(uint32_t); i++) {
        tempResult |= buffer0[i] ^ buffer1[i];
    }

    return tempResult == 0 ? true : false;
}
