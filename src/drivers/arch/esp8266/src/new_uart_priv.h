#pragma once

extern "C"
{
#include <c_types.h>

void uart0_open(uint32 baud_rate, uint32 flags);
void uart0_reset();

uint16 uart0_available();
void uart0_tx_buffer_sync(const uint8_t *buf, uint16_t len);

void uart1_open(uint32 baud_rate, uint32 flags);
void uart1_reset(uint32 baud_rate, uint32 flags);
uint8 uart1_write_byte(uint8 byte);
}
