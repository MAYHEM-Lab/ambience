#include <tos.hpp>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <tos/semaphore.hpp>

ft::semaphore timer_sem(0);

void proc()
{
  while (true)
  {
    timer_sem.down();
    Serial.println("tick");
  }
}

int tot_overflow = 0;
ISR(TIMER1_OVF_vect)
{
  tot_overflow++;
  timer_sem.up();
}

void timer1_init()
{
  TCCR1A = 0;
  TCCR1B = (1 << CS12);
  TIMSK1 |= (1 << TOIE1);
  // initialize counter
  TCNT1 = 0;
}

void setup() {
  Serial.begin(9600);
  Serial.println("hiya");
  ft::start(proc);
  timer1_init();
}

void loop() {
  ft::schedule();
}
