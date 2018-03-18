#pragma once

namespace fs
{
  class timer
  {
  public:
    void sleep(long milliseconds);
  private:
    void start(long mil);
    void stop();
    long m_millis;
  };
}

