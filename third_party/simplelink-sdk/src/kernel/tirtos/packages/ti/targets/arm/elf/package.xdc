/* 
 *  Copyright (c) 2008-2019 Texas Instruments Incorporated
 *  All rights reserved. This program and the accompanying materials
 *  are made available under the terms of the Eclipse Public License v1.0
 *  which accompanies this distribution, and is available at
 *  http://www.eclipse.org/legal/epl-v10.html
 *
 *  Contributors:
 *      Texas Instruments - initial implementation
 *
 * */

/*
 *  ======== package.xdc ========
 */

requires xdc.bld;
requires ti.targets;

/*!
 *  ======== ti.targets.arm.elf ========
 *  Package of Arm targets for TI code generation tools for ELF
 *
 */
package ti.targets.arm.elf [1,0,0] {
    interface IArm;
    interface IR4, IR5;
    interface IM3, IM4;
    module Arm9;
    module M3, M4, M4F;
    module R4Ft, R5F, R5Ft;
}
/*
 *  @(#) ti.targets.arm.elf; 1, 0, 0,0; 8-9-2019 17:15:05; /db/ztree/library/trees/xdctargets/xdctargets-v00/src/ xlibrary

 */

