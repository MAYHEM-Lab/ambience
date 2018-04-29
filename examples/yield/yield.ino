#include <tos.hpp>
#include <ft/include/tos/ft.hpp>

void foo()
{
  int x = 0;
  while(true)
  {
  	Serial.print(ft::this_thread::get_id());
  	Serial.print(" : ");
    Serial.println(x++);
    // even though this is concurrent, it's still a busy wait
    delay(1000);
  }
}

void bar()
{
  int x = 0;
  while(true)
  {
  	Serial.print(ft::this_thread::get_id());
  	Serial.print(" : ");
    Serial.println(x++);
    // even though this is concurrent, it's still a busy wait
    delay(500);
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  ft::start(foo);
  ft::start(bar);
}

void loop() {
  ft::schedule();
}