/**
 * @file    MCU_Interface.h
 * @author  AMG - RF Application team
 * @version 3.2.4
 * @date    26-September-2016
 * @brief   Interface for the low level SPIRIT SPI driver.
 * @details
 * This header file constitutes an interface to the SPI driver used to
 * communicate with Spirit.
 * It exports some function prototypes to write/read registers and FIFOs
 * and to send command strobes.
 *  , the implementation
 * of these functions are not provided here. The user have to implement these functions
 * taking care to keep the exported prototypes.
 *
 * These functions are:
 *
 * <ul>
 * <li>SdkEvalSpiDeinit</li>
 * <li>SpiritSpiInit</li>
 * <li>SpiritSpiWriteRegisters</li>
 * <li>SpiritSpiReadRegisters</li>
 * <li>SpiritSpiCommandStrobes</li>
 * <li>SpiritSpiWriteLinearFifo</li>
 * <li>SpiritSpiReadLinearFifo</li>
 * </ul>
 *
 * @note An example of SPI driver implementation is available in the <i>Sdk_Eval</i> library.
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * THIS SOURCE CODE IS PROTECTED BY A LICENSE.
 * FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
 * IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
 *
 * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MCU_INTERFACE_H
#define __MCU_INTERFACE_H


/* Includes ------------------------------------------------------------------*/
#include "SPIRIT_Types.h"


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup SPIRIT_Libraries
 * @{
 */


/** @defgroup SPIRIT_SPI_Driver         SPI Driver
 * @brief Header file for low level SPIRIT SPI driver.
 * @details See the file <i>@ref MCU_Interface.h</i> for more details.
 * @{
 */



/** @defgroup SPI_Exported_Types        SPI Exported Types
 * @{
 */

/**
 * @}
 */



/** @defgroup SPI_Exported_Constants    SPI Exported Constants
 * @{
 */

/**
 * @}
 */



/** @defgroup SPI_Exported_Macros       SPI Exported Macros
 * @{
 */

/**
 * @}
 */



/** @defgroup SPI_Exported_Functions    SPI Exported Functions
 * @{
 */

typedef SpiritStatus StatusBytes;

void RadioSpiDeinit(void);
void RadioSpiInit(void);
StatusBytes RadioSpiWriteRegisters(uint8_t address, uint8_t n_regs, uint8_t *buffer);
StatusBytes RadioSpiReadRegisters (uint8_t address, uint8_t n_regs, uint8_t *buffer);
StatusBytes RadioSpiCommandStrobes(uint8_t cCommandCode);
StatusBytes RadioSpiWriteFifo(uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes RadioSpiReadFifo(uint8_t cNbBytes, uint8_t* pcBuffer);

void RadioEnterShutdown(void);
void RadioExitShutdown(void);
SpiritFlagStatus RadioCheckShutdown(void);
void RadioSpiSetBaudrate(uint32_t baudrate_prescaler);

#define SpiritEnterShutdown                                  RadioEnterShutdown
#define SpiritExitShutdown                                   RadioExitShutdown
#define SpiritCheckShutdown                                  (SpiritFlagStatus)RadioCheckShutdown


#define SpiritSpiDeinit                                                RadioSpiDeinit
#define SpiritSpiInit                                                  RadioSpiInit
#define SpiritSpiWriteRegisters(cRegAddress, cNbBytes, pcBuffer)       RadioSpiWriteRegisters(cRegAddress, cNbBytes, pcBuffer)
#define SpiritSpiReadRegisters(cRegAddress, cNbBytes, pcBuffer)        RadioSpiReadRegisters(cRegAddress, cNbBytes, pcBuffer)
#define SpiritSpiCommandStrobes(cCommandCode)                          RadioSpiCommandStrobes(cCommandCode)
#define SpiritSpiWriteLinearFifo(cNbBytes, pcBuffer)                   RadioSpiWriteFifo(cNbBytes, pcBuffer)
#define SpiritSpiReadLinearFifo(cNbBytes, pcBuffer)                    RadioSpiReadFifo(cNbBytes, pcBuffer)

/**
 * @}
 */

/**
 * @}
 */


/**
 * @}
 */



#ifdef __cplusplus
}
#endif

#endif

/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
