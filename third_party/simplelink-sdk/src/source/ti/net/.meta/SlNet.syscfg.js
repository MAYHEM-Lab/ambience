/*
 * Copyright (c) 2019 Texas Instruments Incorporated - http://www.ti.com
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

/*
 *  ======== SlNet.syscfg.js ========
 *  General configuration support
 */

"use strict";

/* global vars */

/* Sysconfig string specifying the current device ID */
let deviceId = system.deviceData.deviceId;

/* used in the networkIfFxnList config object */
let cc32xxFunctionList = [
    {
        name: "SimpleLink WiFi"
    },
    {
        name: "Custom"
    }
];

/* used in the networkIfFxnList config object */
let e4FunctionList = [
    {
        name: "NDK"
    },
    {
        name: "Custom"
    }
];

/* hold default values for config objects */
let defval = {
    customFuncList: ""
};

/* global exports, system */

/*
 *  ======== getLibs ========
 */
function getLibs(mod)
{
    let GenLibs = system.getScript("/ti/utils/build/GenLibs.syscfg.js");

    /* create a GenLibs input argument */
    var linkOpts = {
        name: "/ti/net",
        deps: [],
        libs: [GenLibs.libPath("ti/net", "slnetsock_release.a")]
    };

    return (linkOpts);
}

/*
 *  ======== modules ========
 *  Express dependencies for other modules
 *
 *  Invoked on any configuration change to the given instance.
 */
function modules(inst)
{
    let modules = new Array();

    /* NDK is only required if you are using it on this interface */
    if(inst.networkIfFxnList == "NDK") {
        modules.push({
            name: "general",
            moduleName: "/ti/ndk/General"
        });
    }

    return (modules);
}

/*
 *  ======== onChange_enableSecureSocks ========
 */
function onChange_enableSecureSocks(inst, ui)
{
    if(inst.enableSecureSocks) {
        ui.secObjs.hidden = false;
    }
    else {
        ui.secObjs.hidden = true;
        inst.secObjs = 0;
    }
}

/*
 *  ======== onChange_networkIfFxnList ========
 */
function onChange_networkIfFxnList(inst, ui)
{
    if(inst.networkIfFxnList == "NDK") {
        ui.ifName.readOnly = true;
        ui.ifName.hidden = true;
        ui.customFuncList.readOnly = true;
        ui.customFuncList.hidden = true;
        inst.customFuncList = defval.customFuncList;
        inst.ifName = "eth0";
    }
    else if(inst.networkIfFxnList == "SimpleLink WiFi") {
        ui.ifName.readOnly = false;
        ui.ifName.hidden = false;
        ui.customFuncList.readOnly = true;
        ui.customFuncList.hidden = true;
        inst.customFuncList = defval.customFuncList;
        inst.ifName = "wlan0";
    }
    else {
        ui.ifName.readOnly = false;
        ui.ifName.hidden = false;
        ui.customFuncList.readOnly = false;
        ui.customFuncList.hidden = false;
    }
}


/*
 *  ======== validate ========
 *  Validate given instance and report conflicts
 *
 *  This function is not allowed to modify the instance state.
 */
function validate(inst, vo, getRef)
{
    /* config.id */
    let id = inst.id;
    if (id < 1 || id > 16) {
        vo["id"].errors.push("Invalid interface ID. Must be 1-16.");
    }

    let instances = inst.$module.$instances;
    for(let i = 0; i < instances.length; i++) {
        if(instances[i].id == id && instances[i].$name != inst.$name) {
            vo["id"].errors.push(instances[i].$name + " is already using this interface ID");
        }
    }

    /* config.priority */
    if (inst.priority < 1 || inst.priority > 15) {
        vo["priority"].errors.push("Invalid interface priority. Must be 1-15.");
    }

    if (inst.networkIfFxnList == "Custom" && !inst.customFuncList) {
        vo["customFuncList"].errors.push("A custom function list must be specified");
    }
}

/*
 *  ======== longDescription ========
 *  Intro splash on GUI
 */
let longDescription =
    "High level NS configuration.";


/*
 *  ======== config_instance ========
 *  Define the config params of the module (module-wide)
 */
let config_instance = [
    {
        name: "networkIfFxnList",
        displayName: "Network Interface Function List",
        default: deviceId.match(/CC32.*/) ? "SimpleLink WiFi" : "NDK",
        options: deviceId.match(/CC32.*/) ? cc32xxFunctionList : e4FunctionList,
        onChange: onChange_networkIfFxnList,
        description: 'Choose the desired interface function list',
        longDescription: `
Choose from a list of supported function lists on your device or use a custom
one

[More ...](/ns/ConfigDoc.html#ti_net_SlNet_networkIfFxnList)`,
        documentation: `
This setting is equivalent to setting the ifConf argument in
[SlNetIf_add](html/group__SlNetIf.html#gae09651b941726526788a932498d2d250).
`
    },
    {
        name: "customFuncList",
        displayName: "Custom Function List",
        default: defval.customFuncList,
        hidden: true,
        readOnly: true,
        description: 'C struct specifying the function list to use'
    },
    {
        name: "id",
        displayName: "ID",
        default: 1,
        description: "SLNETIF_ID_? value",
        longDescription: `
Specifies the interface which needs to be added.

The values of the interface identifier is defined with the prefix SLNETIF_ID_
which is defined in slnetif.h

[More ...](/ns/ConfigDoc.html#ti_net_SlNet_id)`,
        documentation: `
This setting is equivalent to setting the ifID argument in
[SlNetIf_add](html/group__SlNetIf.html#gae09651b941726526788a932498d2d250).
`
    },
    {
        name: "ifName",
        displayName: "Interface Name",
        default: deviceId.match(/CC32.*/) ? "wlan0" : "eth0",
        hidden: deviceId.match(/CC32.*/) ? false : true,
        readOnly: deviceId.match(/CC32.*/) ? false : true,
        description: 'Specifies the name for this interface"',
        longDescription: `
Specifies the name of the interface. Note: Can be set to NULL, but when set
to NULL cannot be used with SlNetIf_getIDByName.

[More ...](/ns/ConfigDoc.html#ti_net_SlNet_ifName)`,
        documentation: `
This setting is equivalent to setting the ifName argument in
[SlNetIf_add](html/group__SlNetIf.html#gae09651b941726526788a932498d2d250).
`
    },
    {
        name: "priority",
        displayName: "Priority",
        default: 5,
        description: 'Specifies the priority of the interface.',
        longDescription: `
Specifies the priority of the interface (In ascending order). Note: maximum
priority is 15

[More ...](/ns/ConfigDoc.html#ti_net_SlNet_priority)`,
        documentation: `
This setting is equivalent to setting the priority argument in
[SlNetIf_add](html/group__SlNetIf.html#gae09651b941726526788a932498d2d250).
`
    },
    {
        name: "enableSecureSocks",
        displayName: "Enable Secure Sockets",
        description: 'Enable secure sockets',
        default: true,
        onChange: onChange_enableSecureSocks,
        hidden: deviceId.match(/CC32.*/) ? true : false,
        readOnly: deviceId.match(/CC32.*/) ? true : false,
        longDescription: `
Enable secure sockets on this interface. You cannot add secure objects without
this option being selected.

[More ...](/ns/ConfigDoc.html#ti_net_SlNet_enableSecureSocks)`
    },
    /*
     * The following menu item is a bit of a hack to support submodules of tls
     * objects. Preferably we would like these submodules to work like the
     * overall modules with "add" and "remove" buttons to create more instances
     * This functionality is blocked on PMUX-1423.
     */
    {
        name: "secObjs",
        displayName: "Secure Objects",
        description: "Number of secure objects this app requires",
        hidden: deviceId.match(/CC32.*/) ? true : false,
        readOnly: deviceId.match(/CC32.*/) ? true : false,
        longDescription: `
The number of secure objects you would like to create and associate with this
interface`,
        default: 0,
    }
];


/*
 *  ======== moduleInstances ========
 */
function moduleInstances(inst)
{
    let result = [];

    /* Create a Sequence instance for each of the unique sequencers. */
    let secObjs = inst.secObjs;
    for (let i = 0; i < secObjs; i++) {
        let secObjName = "CONFIG_SECOBJ_" + i;
        result.push({
            name: secObjName,
            displayName: "Secure Object " + i,
            moduleName: "/ti/net/SecObj",
            args: {
                $name: secObjName
            }
        });
    }

    return (result);
}


/*
 *  ======== base ========
 *  Module definition object
 */
let base = {
    displayName: "Network Interfaces",
    description: "SlNet configuration",
    defaultInstanceName: "CONFIG_SLNET_",
    longDescription: longDescription,
    config: config_instance,
    moduleInstances: moduleInstances,
    modules: modules,
    validate: validate,
    /*
     * maxInstances must match the value of SLNETIF_MAX_IF in NS
     * If a non-32xx device limit to 1 interface until NDK-431 is resolved.
     */
    maxInstances: deviceId.match(/CC32.*/) ? 16 : 1,
    templates: {
        /* contribute NS libraries to linker command file */
        "/ti/utils/build/GenLibs.cmd.xdt"   :
            {modName: "/ti/net/SlNet", getLibs: getLibs},

        /* trigger generation of ti_ndk_config.c */
        "/ti/net/Config.c.xdt": "/ti/net/SlNet.Config.c.xdt"
    }
};

/* export the module */
exports = base;
