#include <avr/io.h>
#include <LowPower.h>
#include <Arduino.h>

namespace ft
{
namespace impl
{
void set_stack_ptr(void* ptr)
{
  SP = ptr;
}
void power_down()
{
  Serial.flush();
  LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER2_OFF, TIMER1_ON, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
}
}
}

