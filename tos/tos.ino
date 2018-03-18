#include "ft.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "semaphore.hpp"

ft::semaphore timer_sem(0);

ft::semaphore other_sem(0);

void t1()
{
  auto id = ft::this_thread::get_id();
  while (true)
  {
    timer_sem.down();
    Serial.print("Thread ");
    Serial.print(id);
    Serial.print(" ");
    Serial.print(SP);
    Serial.print(" : ");
    Serial.println(millis());
    other_sem.up();
  }
}

void t2()
{
  auto id = ft::this_thread::get_id();
  int x = 0;
  while (x < 5)
  {
    other_sem.down();
    Serial.print("Thread ");
    Serial.print(id);
    Serial.print(" ");
    Serial.print(SP);
    Serial.print(" : ");
    Serial.print(x++);
    Serial.print(" ");
    Serial.println(millis());
    ft::this_thread::yield();
  }
}

int tot_overflow = 0;
ISR(TIMER1_OVF_vect)
{
  // keep a track of number of overflows
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
  ft::start(t1);
  ft::start(t2);
  ft::start(t1);
  timer1_init();
}

void loop() {
  ft::schedule();
}
