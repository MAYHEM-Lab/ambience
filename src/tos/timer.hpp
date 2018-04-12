#pragma once

namespace fs {
    class timer
    {
    public:
        void start(long mil);
        void stop();

    private:
        long m_millis;
    };
}

