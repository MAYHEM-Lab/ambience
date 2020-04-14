/*
 * Copyright (c) 2018-2019 Texas Instruments Incorporated - http://www.ti.com
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
 *
 */

/*
 *  ======== PWMTimerCC32XX.js ========
 */

"use strict";

/* get Common /ti/drivers utility functions */
let Common = system.getScript("/ti/drivers/Common.js");

/*
 *  ======== pwmPins ========
 *  Pins in a 64-pin QFN package that can configured as PWM signals.  (See
 *  the CC3200 TRM chapter on I/O Pads and Pin Multiplexing).
 */
let pwmPins = ["1", "2", "17", "19", "21", "64"];

/*
 *  ======== devSpecific ========
 *  Device-specific extensions to be added to base PWM configuration
 */
let devSpecific = {
    config: [],

    /* override generic requirements with  device-specific reqs (if any) */
    pinmuxRequirements: pinmuxRequirements,

    filterHardware: filterHardware,

    /* override device-specific templates */
    templates: {
        boardc: "/ti/drivers/pwm/PWMTimerCC32XX.Board.c.xdt",
        boardh: "/ti/drivers/pwm/PWMTimer.Board.h.xdt"
    },

    _getPinResources: _getPinResources
};

/*
 *  ======== _getPinResources ========
 */
function _getPinResources(inst)
{
    let pin;

    if (inst.timer) {
       pin = "P" + inst.timer.pwmPin.$solution.packagePinName.padStart("0", 2);

       if (inst.$hardware && inst.$hardware.displayName) {
            pin += ", " + inst.$hardware.displayName;
        }
    }

    return (pin);
}

/*
 *  ======== pinmuxRequirements ========
 *  Returns peripheral pin requirements of the specified instance
 *
 *  param inst    - a fully configured PWM instance
 *
 *  returns req[] - an array of pin requirements needed by inst
 */
function pinmuxRequirements(inst)
{
    let timer = {
        name: "timer",
        displayName: "Timer Peripheral",
        interfaceName: "Timer",
        canShareWith: "PWM",
        resources: [
            {
                name: "pwmPin",
                displayName: "PWM Output",
                interfaceNames: [
                    "GT_PWM00", "GT_PWM01"
                ]
            }
        ],
        signalTypes: { pwmPin: ["PWM"] }
    };

    return ([timer]);
}

/*
 *  ========= filterHardware ========
 *  Check 'component' signals for compatibility with PWM
 *
 *  param component - hardware object describing signals and
 *                    resources they're attached to
 *  returns Boolean indicating whether or not to allow the component to
 *           be assigned to an instance's $hardware config
 */
function filterHardware(component)
{
    for (let sig in component.signals) {
        let type = component.signals[sig].type;
        if (Common.typeMatches(type, ["PWM"])) {
            let devicePin = component.signals[sig].devicePin;
            if (devicePin) {
                if (isValidPwmPin(devicePin.name)) {
                    return (true);
                }
            }
        }
    }

    return (false);
}

/*
 *  ======== isValidPwmPin ========
 *  Verify pin can support PWM
 */
function isValidPwmPin(devicePin)
{
    for (let i = 0; i < pwmPins.length; i++) {
        if (devicePin == pwmPins[i]) {
            return (true);
        }
    }
    return (false);
}

/*
 *  ======== extend ========
 *  Extends a base exports object to include any device specifics
 *
 *  This function is invoked by the generic PWM module to
 *  allow us to augment/override as needed for the CC32XX.
 */
function extend(base)
{
    /* merge and overwrite base module attributes */
    let result = Object.assign({}, base, devSpecific);

    /* concatenate device-specific configs */
    result.config = base.config.concat(devSpecific.config);

    return (result);
}

/*
 *  ======== exports ========
 *  Export device-specific extensions to base exports
 */
exports = {
    /* required function, called by base PWM module */
    extend: extend
};
