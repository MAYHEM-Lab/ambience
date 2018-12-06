//
// Created by fatih on 12/5/18.
//

#include <chrono>
#include <drivers/common/gpio.hpp>
#include <algorithm>
#include <common/driver_base.hpp>

namespace tos
{
    template <class GpioT, class DelayT>
    class hx711 : DelayT, public self_pointing<hx711<GpioT, DelayT>>
    {
    public:
        using pin_type = typename GpioT::pin_type;

        hx711(GpioT& g, DelayT& d, pin_type clk, pin_type data)
            : DelayT{d}, m_g{&g}, m_clk{clk}, m_data{data}
        {
            using namespace std::chrono_literals;

            g.set_pin_mode(m_data, tos::pin_mode::in);
            g.set_pin_mode(m_clk, tos::pin_mode::out);

            g.write(m_clk, tos::digital::high);
            (*static_cast<DelayT*>(this))(100us);
            g.write(m_clk, tos::digital::low);

            m_off = average(32);
        }

        uint32_t get_val()
        {
            return read() - m_off;
        }

    private:

        uint32_t average(int n)
        {
            uint32_t sum = 0;
            for (int i = 0; i < n; ++i)
            {
                sum += read();
            }
            return sum / n;
        }

        GpioT& get_gpio() { return *m_g; }

        uint32_t read()
        {
            uint8_t d[3];
            using std::begin, std::end;
            std::fill(begin(d), end(d), 0);

            auto& g = get_gpio();

            while (g.read(m_data));

            for (int j = 3; j--;)
            {
                for (char i = 8; i--;)
                {
                    g.write(m_clk, tos::digital::high);
                    if (g.read(m_data))
                    {
                        d[j] |= (1 << i);
                    }
                    g.write(m_clk, tos::digital::low);
                }
            }

            g.write(m_clk, tos::digital::high);
            g.write(m_clk, tos::digital::low);

            d[2] ^= 0x80;

            return ((uint32_t) d[2] << 16) | ((uint32_t) d[1] << 8)
                   | (uint32_t) d[0];
        }

        uint32_t m_off;
        GpioT* m_g;
        pin_type m_clk, m_data;
    };

    namespace devs
    {
        using hx711_t = dev<struct _hx711_t, 0>;
        static constexpr hx711_t hx711{};
    } // namespace devs

    template <class GpioT, class DelayT>
    hx711<GpioT, DelayT> open_impl(devs::hx711_t,
            GpioT& g, DelayT& d,
            typename GpioT::pin_type clk_pin,
            typename GpioT::pin_type data_pin)
    {
        return { g, d, clk_pin, data_pin };
    }
} // namespace tos