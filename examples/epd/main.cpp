//
// Created by fatih on 5/21/19.
//

#include <tos/ft.hpp>

#include <tos/semaphore.hpp>

#include <arch/drivers.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>

auto delay = [](std::chrono::microseconds us) {
    uint32_t end = (us.count() * (rcc_ahb_frequency / 1'000'000)) / 13.3;
    for (volatile int i = 0; i < end; ++i)
    {
        __asm__ __volatile__ ("nop");
    }
};
constexpr uint8_t WF_LUT[] =
{
    0x82,0x00,0x00,0x00,0xAA,0x00,0x00,0x00,
    0xAA,0xAA,0x00,0x00,0xAA,0xAA,0xAA,0x00,
    0x55,0xAA,0xAA,0x00,0x55,0x55,0x55,0x55,
    0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55,
    0xAA,0xAA,0xAA,0xAA,0x15,0x15,0x15,0x15,
    0x05,0x05,0x05,0x05,0x01,0x01,0x01,0x01,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x41,0x45,0xF1,0xFF,0x5F,0x55,0x01,0x00,
    0x00,0x00
};

#define EPD_REG_0             0x00   /* Status Read */
#define EPD_REG_1             0x01   /* Driver Output Control */
#define EPD_REG_3             0x03   /* Gate driving voltage control */
#define EPD_REG_4             0x04   /* Source driving coltage control */
#define EPD_REG_7             0x07   /* Display Control */
#define EPD_REG_11            0x0B   /* Gate and Sorce non overlap period COntrol */
#define EPD_REG_15            0x0F   /* Gate scan start */
#define EPD_REG_16            0x10   /* Deep Sleep mode setting */
#define EPD_REG_17            0x11   /* Data Entry Mode Setting */
#define EPD_REG_18            0x12   /* SWRESET */
#define EPD_REG_26            0x1A   /* Temperature Sensor Control (Write to Temp Register) */
#define EPD_REG_27            0x1B   /* Temperature Sensor Control(Read from Temp Register) */
#define EPD_REG_28            0x1C   /* Temperature Sensor Control(Write Command  to Temp sensor) */
#define EPD_REG_29            0x1D   /* Temperature Sensor Control(Load temperature register with temperature sensor reading) */
#define EPD_REG_32            0x20   /* Master activation */
#define EPD_REG_33            0x21   /* Display update */
#define EPD_REG_34            0x22   /* Display update control 2 */
#define EPD_REG_36            0x24   /* write RAM */
#define EPD_REG_37            0x25   /* Read RAM */
#define EPD_REG_40            0x28   /* VCOM sense */
#define EPD_REG_41            0x29   /* VCOM Sense duration */
#define EPD_REG_42            0x2A   /* VCOM OTP program */
#define EPD_REG_44            0x2C   /* Write VCOMregister */
#define EPD_REG_45            0x2D   /* Read OTP registers */
#define EPD_REG_48            0x30   /* Program WS OTP */
#define EPD_REG_50            0x32   /* Write LUT register */
#define EPD_REG_51            0x33   /* Read LUT register */
#define EPD_REG_54            0x36   /* Program OTP selection */
#define EPD_REG_55            0x37   /* Proceed OTP selection */
#define EPD_REG_58            0x3A   /* Set dummy line pulse period */
#define EPD_REG_59            0x3B   /* Set Gate line width */
#define EPD_REG_60            0x3C   /* Select Border waveform */
#define EPD_REG_68            0x44   /* Set RAM X - Address Start / End Position */
#define EPD_REG_69            0x45   /* Set RAM Y - Address Start / End Position */
#define EPD_REG_78            0x4E   /* Set RAM X Address Counter */
#define EPD_REG_79            0x4F   /* Set RAM Y Address Counter */
#define EPD_REG_240           0xF0   /* Booster Set Internal Feedback Selection */
#define EPD_REG_255           0xFF   /* NOP */


template <class SpiT>
class epd
{
public:
    using PinT = tos::stm32::gpio::pin_type;

    static constexpr auto HEIGHT = 72;
    static constexpr auto WIDTH = 172;

    explicit epd(SpiT spi, PinT cs, PinT dc, PinT reset, PinT busy)
        : m_spi{std::move(spi)}
        , m_cs{cs}
        , m_dc{dc}
        , m_reset{reset}
        , m_busy{busy}
        {
            m_g.set_pin_mode(m_reset, tos::pin_mode::in);
            m_g.write(m_reset, tos::digital::low);
            m_g.set_pin_mode(m_busy, tos::pin_mode::out);
        }

    void gde021a1_Init(void)
    {
        uint8_t nb_bytes = 0;

        /* Initialize the GDE021A11 */

        _writeCommand(EPD_REG_16);  /* Deep sleep mode disable */
        _writeData(0x00);
        _writeCommand(EPD_REG_17);  /* Data Entry Mode Setting */
        _writeData(0x03);
        _writeCommand(EPD_REG_68);  /* Set the RAM X start/end address */
        _writeData(0x00);       /* RAM X address start = 00h */
        _writeData(0x11);       /* RAM X adress end = 11h (17 * 4pixels by address = 72 pixels) */
        _writeCommand(EPD_REG_69);  /* Set the RAM Y start/end address */
        _writeData(0x00);       /* RAM Y address start = 0 */
        _writeData(0xAB);       /* RAM Y adress end = 171 */
        _writeCommand(EPD_REG_78);  /* Set RAM X Address counter */
        _writeData(0x00);
        _writeCommand(EPD_REG_79);  /* Set RAM Y Address counter */
        _writeData(0x00);
        _writeCommand(EPD_REG_240); /* Booster Set Internal Feedback Selection */
        _writeData(0x1F);
        _writeCommand(EPD_REG_33);  /* Disable RAM bypass and set GS transition to GSA = GS0 and GSB = GS3 */
        _writeData(0x03);
        _writeCommand(EPD_REG_44);  /* Write VCOMregister */
        _writeData(0xA0);
        _writeCommand(EPD_REG_60);  /* Border waveform */
        _writeData(0x64);
        _writeCommand(EPD_REG_50);  /* Write LUT register */

        for (nb_bytes=0; nb_bytes<90; nb_bytes++)
        {
            _writeData(WF_LUT[nb_bytes]);
        }
    }

    void gde021a1_WritePixel(uint8_t HEX_Code)
    {
        /* Prepare the register to write data on the RAM */
        _writeCommand(EPD_REG_36);

        /* Send the data to write */
        _writeData(HEX_Code);
    }

    void gde021a1_WriteReg(uint8_t EPD_Reg, uint8_t EPD_RegValue)
    {
        _writeCommand(EPD_Reg);

        _writeData(EPD_RegValue);
    }

    void gde021a1_RefreshDisplay(void)
    {
        /* Write on the Display update control register */
        _writeCommand(EPD_REG_34);

        /* Display update data sequence option */
        _writeData(0xC4);

        /* Launching the update: Nothing should interrupt this sequence in order
           to avoid display corruption */
        _writeCommand(EPD_REG_32);
    }

    void gde021a1WriteRegArray(uint8_t cmd, uint8_t *data, uint8_t numDataBytes) {
        tos::stm32::gpio g;
        g.write(m_dc, tos::digital::low);
        g.write(m_cs, tos::digital::low);
        m_spi->exchange(cmd);
        g.write(m_dc, tos::digital::high);
        m_spi->write({data, numDataBytes});
        g.write(m_cs, tos::digital::high);
    }

    void writeReg(uint8_t cmd, uint8_t data) {
        gde021a1WriteRegArray(cmd, &data, 1);
    }

    void refreshDisplay() {
        writeReg(EPD_REG_34, 0xC4);
        gde021a1WriteRegArray(EPD_REG_32, NULL, 0);
    }

    void gde021a1Init() {
        writeReg(EPD_REG_16, 0x00);     // Deep sleep mode disable
        writeReg(EPD_REG_17, 0x03);     // Data Entry Mode Setting

        // This sets the start and end addresses in the Y direction
        // RAM X address start = 0x00
        // RAM X address end = 0x11 (17 * 4 px/addr = 72 pixels)
        uint8_t temp[2] = {0x00, 0x11};
        gde021a1WriteRegArray(EPD_REG_68, temp, 2);

        // This sets the start and end addresses in the Y direction
        // RAM Y address start = 0x00
        // RAM Y address end = 0xAB
        temp[0] = 0x00;
        temp[1] = 0xAB;
        gde021a1WriteRegArray(EPD_REG_69, temp, 2);

        writeReg(EPD_REG_78, 0x00); // Set RAM X Address counter
        writeReg(EPD_REG_79, 0x00); // Set RAM Y Address counter
        writeReg(EPD_REG_240, 0x1F); // Booster Set Internal Feedback Selection
        writeReg(EPD_REG_33, 0x03); // Disable RAM bypass and set GS transition
        // to GSA = GS0 and GSB = GS3
        writeReg(EPD_REG_44, 0xA0); // Write VCOM Register
        writeReg(EPD_REG_60, 0x64); // Border waveform
        gde021a1WriteRegArray(EPD_REG_50, (uint8_t *)WF_LUT, 90); //Write LUT
    }

    void _Init_Full()
    {
        _InitDisplay();
        _writeCommandDataPGM(WF_LUT, sizeof(WF_LUT));
        _PowerOn();
        //_using_partial_mode = false;
    }

    void _Update_Full()
    {
        _writeCommand(0x22);
        _writeData(0xc4);
        _writeCommand(0x20);
        _waitWhileBusy();//"_Update_Full", full_refresh_time);
        _writeCommand(0xff);
    }

    void clear_init(uint8_t val)
    {
        _Init_Full();
        _setPartialRamArea(0, 0, WIDTH, HEIGHT);
        _writeCommand(0x24);
        for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++)
        {
            _writeData(val);
        }
        _Update_Full();
    }

    void _PowerOn()
    {
        //if (!_power_is_on)
        {
            _writeCommand(0x22);
            _writeData(0xc0);
            _writeCommand(0x20);
            _waitWhileBusy();//"_PowerOn", power_on_time);
        }
        //_power_is_on = true;
    }


    void _InitDisplay()
    {
        _writeCommand(0x01); // Panel configuration, Gate selection
        _writeData((HEIGHT - 1) % 256);
        _writeData((HEIGHT - 1) / 256);
        _writeData(0x00);
        _writeCommand(0x0c); // softstart
        _writeData(0xd7);
        _writeData(0xd6);
        _writeData(0x9d);
        _writeCommand(0x2c); // VCOM setting
        _writeData(0xa8);    // * different
        _writeCommand(0x3a); // DummyLine
        _writeData(0x1a);    // 4 dummy line per gate
        _writeCommand(0x3b); // Gatetime
        _writeData(0x08);    // 2us per line
        _setPartialRamArea(0, 0, WIDTH, HEIGHT);

    }

    void _setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
    {
        _writeCommand(0x11); // set ram entry mode
        _writeData(0x01);    // x increase, y decrease : as in demo code
        _writeCommand(0x44);
        _writeData(x / 8);
        _writeData((x + w - 1) / 8);
        _writeCommand(0x45);
        _writeData((y + h - 1) % 256);
        _writeData((y + h - 1) / 256);
        _writeData(y % 256);
        _writeData(y / 256);
        _writeCommand(0x4e);
        _writeData(x / 8);
        _writeCommand(0x4f);
        _writeData((y + h - 1) % 256);
        _writeData((y + h - 1) / 256);
    }

    void _reset()
    {
        m_g.write(m_reset, tos::digital::high);
        using namespace std::chrono_literals;
        delay(20ms);
        m_g.write(m_reset, tos::digital::low);
        delay(20ms);
        m_g.write(m_reset, tos::digital::high);
        delay(200ms);
    }

    void _waitWhileBusy()
    {
        while (m_g.read(m_busy)) {
            tos::this_thread::yield();
        }
    }

private:

    void _writeCommand(uint8_t c)
    {
        tos::stm32::gpio g;
        g.write(m_dc, tos::digital::low);
        g.write(m_cs, tos::digital::low);
        m_spi->exchange(c);
        g.write(m_cs, tos::digital::high);
        g.write(m_dc, tos::digital::high);
    }

    void _writeData(uint8_t d)
    {
        tos::stm32::gpio g;
        g.write(m_cs, tos::digital::low);
        m_spi->exchange(d);
        g.write(m_cs, tos::digital::high);
    }

    void _writeData(const uint8_t* data, uint16_t n) {
        tos::stm32::gpio g;
        g.write(m_cs, tos::digital::low);
        m_spi->write({data, n});
        g.write(m_cs, tos::digital::high);
    }

    void _writeCommandData(const uint8_t* pCommandData, uint8_t datalen)
    {
        tos::stm32::gpio g;
        g.write(m_dc, tos::digital::low);
        g.write(m_cs, tos::digital::low);
        m_spi->exchange(*pCommandData++);
        datalen--;
        g.write(m_dc, tos::digital::high);
        m_spi->write({pCommandData, datalen});
        g.write(m_cs, tos::digital::high);
    }

    void _writeCommandDataPGM(const uint8_t* pCommandData, uint8_t datalen)
    {
        _writeCommandData(pCommandData, datalen);
    }

    void _writeDataPGM(const uint8_t* data, uint16_t n)
    {
        _writeData(data, n);
    }

    SpiT m_spi;
    tos::stm32::gpio m_g;
    tos::stm32::gpio::pin_type m_cs, m_dc, m_reset, m_busy;
};

void blink_task()
{
    using namespace tos::tos_literals;

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);

    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN,
                    GPIO3 | GPIO4 | GPIO5);
    gpio_set_af(GPIOB, GPIO_AF5, GPIO3 | GPIO4 | GPIO5);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ,
                            GPIO4 | GPIO5);

    auto cs = 15_pin;
    auto dc = 27_pin;

    auto g = tos::open(tos::devs::gpio);
    g.set_pin_mode(cs, tos::pin_mode::out);
    g.write(cs, tos::digital::high);

    g.set_pin_mode(dc, tos::pin_mode::out);
    g.write(dc, tos::digital::high);

    auto tmr = tos::open(tos::devs::timer<2>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    tos::stm32::spi spi(tos::stm32::detail::spis[0]);

    epd<decltype(&spi)> display(&spi, cs, dc, 18_pin, 8_pin);
    display.gde021a1Init();
    display.refreshDisplay();

    g.set_pin_mode(5_pin, tos::pin_mode::out);

    g.write(5_pin, tos::digital::high);
    while (true)
    {
        using namespace std::chrono_literals;
        alarm.sleep_for(1s);
        g.write(5_pin, tos::digital::low);

        alarm.sleep_for(1s);
        g.write(5_pin, tos::digital::high);
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, blink_task);
}
