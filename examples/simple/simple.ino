#include "tos.h"
#include "tos/semaphore.hpp"

ft::semaphore other_sem(0);

void t1()
{
  auto id = ft::this_thread::get_id();
  while (true)
  {
    Serial.print("Thread ");
    Serial.print(id);
    Serial.print(" ");
    Serial.print(SP);
    Serial.print(" : ");
    Serial.println(millis());
    other_sem.up();
    ft::this_thread::yield();
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

void setup() {
  Serial.begin(9600);
  Serial.println("hiya");
  ft::start(t1);
  ft::start(t2);
}

void loop() {
  ft::schedule();
  delay(1000);
}