/**
  ******************************************************************************
  * @file    SPBTLE_RF.cpp
  * @author  Wi6Labs
  * @version V1.0.0
  * @date    18-August-2017
  * @brief
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <SPBTLE_RF.h>
#include <libopencm3/stm32/exti.h>
#include "hci.h"
#include "stm32_bluenrg_ble.h"
#include "bluenrg_interface.h"
#include "debug.h"
#include "gp_timer.h"
#include "hal.h"
#include "../../arch/stm32l0/include/arch/spi.hpp"
#include "../../arch/stm32l0/include/arch/gpio.hpp"
#include <tos/mutex.hpp>

#define HEADER_SIZE 5
#define MAX_BUFFER_SIZE 255

uint8_t bnrg_expansion_board;

void (*HCI_callback)(void *);

namespace digital = tos::digital;

using namespace tos::stm32;
static spi* spi_ptr;
static gpio g;
static gpio::pin_type csPin; // PD13
static gpio::pin_type spiIRQPin; // PE6
static gpio::pin_type resetPin; // PA8
tos::any_alarm* alarm_ptr;

tos::semaphore isr_sem{0};
bool isr_enabled = false;
extern "C" void exti9_5_isr() {
    if (isr_enabled)
    {
        isr_sem.up_isr();
    }

    EXTI_PR = EXTI6;
}

void ble_isr()
{
    while (true)
    {
        isr_sem.down();
        if (!isr_enabled) continue;
        SPI_EXTI_Callback();
    }
}

SPBTLERFClass::SPBTLERFClass(spi *SPIx, uint8_t cs, uint8_t spiIRQ,
                              uint8_t reset)
{
    spi_ptr = SPIx;
  csPin = tos::tos_literals::operator""_pin((unsigned long long)cs);
  spiIRQPin =  tos::tos_literals::operator""_pin((unsigned long long)spiIRQ);
  resetPin =  tos::tos_literals::operator""_pin((unsigned long long)reset);
}

static tos::stack_storage<1024> sstorage;
SPBTLERF_state_t SPBTLERFClass::begin(void)
{
  tos::launch(sstorage, ble_isr);
  /* Initialize the BlueNRG SPI driver */
  // Configure SPI and CS pin
  g.set_pin_mode(csPin, tos::pin_mode::out);
  g.write(csPin, digital::high);

    g->set_pin_mode(spiIRQPin, tos::pin_mode::in_pulldown);
    exti_select_source(EXTI6, GPIOE);
    exti_set_trigger(EXTI6, EXTI_TRIGGER_RISING);
    exti_enable_request(EXTI6);
    nvic_enable_irq(NVIC_EXTI9_5_IRQ);
  // Enable SPI EXTI interrupt
  isr_enabled = true;

  // Configure Reset pin
    g.set_pin_mode(resetPin, tos::pin_mode::out);
    g.write(resetPin, digital::low);

  /* Initialize the BlueNRG HCI */
  HCI_Init();

  /* Reset BlueNRG hardware */
  BlueNRG_RST();

  return SPBTLERF_OK;
}

void SPBTLERFClass::end(void)
{
    g.write(resetPin, digital::low);
    isr_enabled = false;
}

void SPBTLERFClass::update(void)
{
  HCI_Process();
}

void Hal_Write_Serial(const void* data1, const void* data2, int32_t n_bytes1,
                       int32_t n_bytes2)
{
  while (true)
  {
      if (BlueNRG_SPI_Write((uint8_t *)data1,(uint8_t *)data2, n_bytes1, n_bytes2) == 0)
      {
          break;
      }
      tos::this_thread::yield();
  }
}

void BlueNRG_RST(void)
{
    using namespace std::chrono_literals;
    g.write(resetPin, digital::low);
    alarm_ptr->sleep_for(5ms);
    g.write(resetPin, digital::high);
    alarm_ptr->sleep_for(5ms);
}

uint8_t BlueNRG_DataPresent(void)
{
    return g.read(spiIRQPin);
}

void BlueNRG_HW_Bootloader(void)
{
  Disable_SPI_IRQ();

  g.set_pin_mode(spiIRQPin, tos::pin_mode::out);
  g.write(spiIRQPin, digital::high);

  BlueNRG_RST();
  Enable_SPI_IRQ();
}

static tos::mutex prot;


int32_t BlueNRG_SPI_Read_All(uint8_t *buffer,
                            uint8_t buff_size)
{
  tos::lock_guard lock(prot);

  g.write(csPin, digital::low);

    uint8_t header_master[HEADER_SIZE] = {0x0b, 0x00, 0x00, 0x00, 0x00};
    uint8_t header_slave[HEADER_SIZE];

  for (int i = 0; i < HEADER_SIZE; ++i)
  {
      header_slave[i] = spi_ptr->exchange8(header_master[i]);
  }

  uint16_t byte_count = 0;
  if (header_slave[0] == 0x02) {
    /* device is ready */
    byte_count = (uint16_t(header_slave[4]) << 8) | header_slave[3];

    if (byte_count > 0) {
      byte_count = std::min<uint16_t>(byte_count, buff_size);

      for (uint16_t len = 0; len < byte_count; len++){
        buffer[len] = spi_ptr->exchange8(0xff);
      }
    }
  }

  g.write(csPin, digital::high);

  // Add a small delay to give time to the BlueNRG to set the IRQ pin low
  // to avoid a useless SPI read at the end of the transaction
  for(int i = 0; i < 2; i++) asm("nop");

  return byte_count;
}

int32_t BlueNRG_SPI_Write(uint8_t* data1,
                         uint8_t* data2, uint8_t Nb_bytes1, uint8_t Nb_bytes2)
{
  int32_t result = 0;

    tos::lock_guard lock(prot);
  Disable_SPI_IRQ();

  g.write(csPin, digital::low);
    unsigned char header_master[HEADER_SIZE] = {0x0a, 0x00, 0x00, 0x00, 0x00};
    unsigned char header_slave[HEADER_SIZE] = {0xaa, 0x00, 0x00, 0x00, 0x00};
  for (int i = 0; i < HEADER_SIZE; ++i)
  {
      header_slave[i] = spi_ptr->exchange8(header_master[i]);
  }

  if (header_slave[0] == 0x02) {
    /* SPI is ready */
    if (header_slave[1] >= (Nb_bytes1+Nb_bytes2)) {

      /*  Buffer is big enough */
      if (Nb_bytes1 > 0) {
          for (int i = 0; i < Nb_bytes1; ++i)
          {
              spi_ptr->exchange8(data1[i]);
          }

      }
      if (Nb_bytes2 > 0) {
          for (int i = 0; i < Nb_bytes2; ++i)
          {
              spi_ptr->exchange8(data2[i]);
          }
      }

    } else {
      /* Buffer is too small */
      result = -2;
    }
  } else {
    /* SPI is not ready */
    result = -1;
  }

  /* Release CS line */

    g.write(csPin, digital::high);

  Enable_SPI_IRQ();

  return result;
}

/**
 * @brief  Enable SPI IRQ.
 * @param  None
 * @retval None
 */
void Enable_SPI_IRQ(void)
{
    isr_enabled = true;
}

/**
 * @brief  Disable SPI IRQ.
 * @param  None
 * @retval None
 */
void Disable_SPI_IRQ(void)
{
    isr_enabled = false;
}

void attach_HCI_CB(void (*callback)(void *pckt))
{
  HCI_callback = callback;
}

void HCI_Event_CB(void *pckt)
{
  HCI_callback(pckt);
}
