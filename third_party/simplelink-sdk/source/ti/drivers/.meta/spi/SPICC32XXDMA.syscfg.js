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
 *  ======== SPICC32XXDMA.syscfg.js ========
 */

"use strict";

/* get Common /ti/drivers utility functions */
let Common = system.getScript("/ti/drivers/Common.js");

let intPriority = Common.newIntPri()[0];
intPriority.name = "dmaInterruptPriority";
intPriority.displayName = "DMA Interrupt Priority";

/*
 *  ======== devSpecific ========
 *  Device-specific extensions to be added to base SPI configuration
 */
let devSpecific = {
    config: [
        {
            name: "turboMode",
            displayName: "Turbo Mode",
            description: "Enable to maximize the throughput for multiple "
                + "word SPI transfers",
            default: false
        },
        intPriority
    ],

    moduleStatic: {
        displayName: "SPI Driver Configuration",
        config : [{
            name: "includeNWP",
            displayName: "Include Network Processor",
            description: "A spi instance used to communicate with the NWP"
            + " will be generated.",
            default: true
        }],

        /* bring in DMA and Power modules */
        modules: Common.autoForceModules(["Board", "Power", "DMA"])
    },

    /* override generic requirements with  device-specific reqs (if any) */
    pinmuxRequirements: pinmuxRequirements,

    templates: {
        boardc: "/ti/drivers/spi/SPICC32XXDMA.Board.c.xdt",
        boardh: "/ti/drivers/spi/SPICC32XXDMA.Board.h.xdt"
    },

    uiAdd : "staticAndInstance",

    _getPinResources: _getPinResources
};

/*
 *  ======== _getPinResources ========
 */
function _getPinResources(inst)
{
    let pin;
    let mosi = "Unassigned";
    let miso = "Unassigned";
    let sclk;
    let ss;

    if (inst.spi) {
        if (inst.spi.mosiPin) {
            mosi = inst.spi.mosiPin.$solution.packagePinName.padStart(2, "0");
        }
        if (inst.spi.misoPin) {
            miso = inst.spi.misoPin.$solution.packagePinName.padStart(2, "0");
        }

        pin = "\nMOSI: " + mosi + "\nMISO: " + miso;

        if (inst.spi.sclkPin) {
            sclk = inst.spi.sclkPin.$solution.packagePinName.padStart(2, "0");
            pin += "\nSCLK: " + sclk;
        }
        if (inst.spi.ssPin) {
            ss = inst.spi.ssPin.$solution.packagePinName.padStart(2, "0");
            pin += "\nSS: " + ss;
        }

        if (inst.$hardware && inst.$hardware.displayName) {
            pin += "\n" + inst.$hardware.displayName;
        }
    }

    return (pin);
}

/*
 *  ======== lspiFilter ========
 *  The LSPI SPI interface is hard coded to SPI instance 0.
 *  Remove LSPI from the list of SPI choices
 */
function lspiFilter(iface, peripheralPin)
{
    /* At this moment, only SPI0 (GSPI) is available */
    if (iface.name == "SPI0") return (true);

    return (false);
}

/*
 *  ======== pinmuxRequirements ========
 *  Return peripheral pin requirements as a function of config
 */
function pinmuxRequirements(inst)
{
    let misoRequired = false;
    let mosiRequired = false;
    let txRequired   = true;
    let rxRequired   = true;

    switch (inst.duplex) {
        case "Full":
            misoRequired = true;
            mosiRequired = true;
            break;
        case "Master TX Only":
            misoRequired = false;
            mosiRequired = true;
            rxRequired   = false;
            break;
        case "Slave RX Only":
            misoRequired = false;
            mosiRequired = true;
            txRequired   = false;
            break;
        case "Master RX Only":
            misoRequired = true;
            mosiRequired = false;
            txRequired   = false;
            break;
        case "Slave TX Only":
            misoRequired = true;
            mosiRequired = false;
            rxRequired   = false;
            break;
    }

    let spi = {
        name: "spi",
        displayName: "SPI Peripheral",
        interfaceName: "SPI",
        canShareWith: "SPI",
        filter: lspiFilter,
        resources: [
            {
                name: "sclkPin",
                displayName: "SCLK Pin",
                description: "SPI Serial Clock",
                interfaceNames: ["CLK"]
            }
        ]
    };

    if (misoRequired) {
        spi.resources.push({
            name: "misoPin",
            displayName: "MISO Pin",
            description: "Master Input Slave Output pin",
            interfaceNames: ["DIN"]});
    }

    if (mosiRequired) {
        spi.resources.push({
            name: "mosiPin",
            displayName: "MOSI Pin",
            description: "Master Output Slave Input pin",
            interfaceNames: ["DOUT"]});
    }

    /* add SS pin if one of the four pin modes is selected */
    if (inst.mode != "Three Pin") {
        spi.resources.push({
                    name: "ssPin",
                    displayName: "SS Pin",
                    description: "Slave Select / Chip Select",
                    interfaceNames: ["CS"]
                });
    }

    if (rxRequired) {
        spi.resources.push({
            name: "dmaRxChannel",
            displayName: "DMA RX Channel",
            description: "DMA channel used to receive data",
            interfaceNames: ["DMA_RX"]});
    }

    if (txRequired) {
        spi.resources.push({
            name: "dmaTxChannel",
            displayName: "DMA TX Channel",
            description: "DMA channel used to send data",
            interfaceNames: ["DMA_TX"]});
    }

    spi.signalTypes = {
        sclkPin: ["SPI_SCLK"],
        mosiPin: ["SPI_MOSI"],
        misoPin: ["SPI_MISO"],
        ssPin:   ["DOUT", "SPI_SS"]
    };

    return ([spi]);
}

/*
 *  ======== extend ========
 *  Extends a base exports object to include any device specifics
 *
 *  This function is invoked by the generic SPI module to
 *  allow us to augment/override as needed for the CC32XX
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
    /* required function, called by base SPI module */
    extend: extend
};
