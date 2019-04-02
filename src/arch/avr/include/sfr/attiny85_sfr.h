#ifndef attiny85_sfr_h
#define attiny85_sfr_h

#include <stdint.h>

/*
Source            Atmel.ATtiny_DFP.1.3.229.atpack
Family            tinyAVR
Architecture      AVR8
Device name       ATtiny85
*/

namespace SFR {

    // ADC Control and Status Register B
    struct ADCSRB {
        static const uint8_t address = 0x23;
        enum bits : uint8_t {
            ADTS0 = 1,    // ADC Auto Trigger Sources
            ADTS1 = 2,    // ADC Auto Trigger Sources
            ADTS2 = 4,    // ADC Auto Trigger Sources
            IPR = 32,    // Input Polarity Mode
            ACME = 64,    // Analog Comparator Multiplexer Enable
            BIN = 128,    // Bipolar Input Mode
        };
    };

    // ADC Data Register  Bytes
    struct ADC {
        static const uint16_t address = 0x24;
    };

    // The ADC Control and Status register
    struct ADCSRA {
        static const uint8_t address = 0x26;
        enum bits : uint8_t {
            ADPS0 = 1,    // ADC  Prescaler Select Bits
            ADPS1 = 2,    // ADC  Prescaler Select Bits
            ADPS2 = 4,    // ADC  Prescaler Select Bits
            ADIE = 8,    // ADC Interrupt Enable
            ADIF = 16,    // ADC Interrupt Flag
            ADATE = 32,    // ADC Auto Trigger Enable
            ADSC = 64,    // ADC Start Conversion
            ADEN = 128,    // ADC Enable
        };
    };

    // The ADC multiplexer Selection Register
    struct ADMUX {
        static const uint8_t address = 0x27;
        enum bits : uint8_t {
            MUX0 = 1,    // Analog Channel and Gain Selection Bits
            MUX1 = 2,    // Analog Channel and Gain Selection Bits
            MUX2 = 4,    // Analog Channel and Gain Selection Bits
            MUX3 = 8,    // Analog Channel and Gain Selection Bits
            REFS2 = 16,    // Reference Selection Bit 2
            ADLAR = 32,    // Left Adjust Result
            REFS0 = 64,    // Reference Selection Bits
            REFS1 = 128,    // Reference Selection Bits
        };
    };

    // Analog Comparator Control And Status Register
    struct ACSR {
        static const uint8_t address = 0x28;
        enum bits : uint8_t {
            ACIS0 = 1,    // Analog Comparator Interrupt Mode Select bits
            ACIS1 = 2,    // Analog Comparator Interrupt Mode Select bits
            ACIE = 8,    // Analog Comparator Interrupt Enable
            ACI = 16,    // Analog Comparator Interrupt Flag
            ACO = 32,    // Analog Compare Output
            ACBG = 64,    // Analog Comparator Bandgap Select
            ACD = 128,    // Analog Comparator Disable
        };
    };

    // USI Control Register
    struct USICR {
        static const uint8_t address = 0x2d;
        enum bits : uint8_t {
            USITC = 1,    // Toggle Clock Port Pin
            USICLK = 2,    // Clock Strobe
            USICS0 = 4,    // USI Clock Source Select Bits
            USICS1 = 8,    // USI Clock Source Select Bits
            USIWM0 = 16,    // USI Wire Mode Bits
            USIWM1 = 32,    // USI Wire Mode Bits
            USIOIE = 64,    // Counter Overflow Interrupt Enable
            USISIE = 128,    // Start Condition Interrupt Enable
        };
    };

    // USI Status Register
    struct USISR {
        static const uint8_t address = 0x2e;
        enum bits : uint8_t {
            USICNT0 = 1,    // USI Counter Value Bits
            USICNT1 = 2,    // USI Counter Value Bits
            USICNT2 = 4,    // USI Counter Value Bits
            USICNT3 = 8,    // USI Counter Value Bits
            USIDC = 16,    // Data Output Collision
            USIPF = 32,    // Stop Condition Flag
            USIOIF = 64,    // Counter Overflow Interrupt Flag
            USISIF = 128,    // Start Condition Interrupt Flag
        };
    };

    // USI Data Register
    struct USIDR {
        static const uint8_t address = 0x2f;
    };

    // USI Buffer Register
    struct USIBR {
        static const uint8_t address = 0x30;
    };

    // General purpose register 0
    struct GPIOR0 {
        static const uint8_t address = 0x31;
    };

    // General Purpose register 1
    struct GPIOR1 {
        static const uint8_t address = 0x32;
    };

    // General Purpose IO register 2
    struct GPIOR2 {
        static const uint8_t address = 0x33;
    };

    // 
    struct DIDR0 {
        static const uint8_t address = 0x34;
        enum bits : uint8_t {
            AIN0D = 1,    // AIN0 Digital Input Disable
            AIN1D = 2,    // AIN1 Digital Input Disable
            ADC1D = 4,    // ADC1 Digital input Disable
            ADC3D = 8,    // ADC3 Digital input Disable
            ADC2D = 16,    // ADC2 Digital input Disable
            ADC0D = 32,    // ADC0 Digital input Disable
        };
    };

    // Pin Change Enable Mask
    struct PCMSK {
        static const uint8_t address = 0x35;
    };

    // Input Pins, Port B
    struct PINB {
        static const uint8_t address = 0x36;
    };

    // Data Direction Register, Port B
    struct DDRB {
        static const uint8_t address = 0x37;
    };

    // Data Register, Port B
    struct PORTB {
        static const uint8_t address = 0x38;
    };

    // EEPROM Control Register
    struct EECR {
        static const uint8_t address = 0x3c;
        enum bits : uint8_t {
            EERE = 1,    // EEPROM Read Enable
            EEPE = 2,    // EEPROM Write Enable
            EEMPE = 4,    // EEPROM Master Write Enable
            EERIE = 8,    // EEPROM Ready Interrupt Enable
            EEPM0 = 16,    // EEPROM Programming Mode Bits
            EEPM1 = 32,    // EEPROM Programming Mode Bits
        };
    };

    // EEPROM Data Register
    struct EEDR {
        static const uint8_t address = 0x3d;
    };

    // EEPROM Address Register  Bytes
    struct EEAR {
        static const uint16_t address = 0x3e;
    };

    // Power Reduction Register
    struct PRR {
        static const uint8_t address = 0x40;
        enum bits : uint8_t {
            PRADC = 1,    // Power Reduction ADC
            PRUSI = 2,    // Power Reduction USI
            PRTIM0 = 4,    // Power Reduction Timer/Counter0
            PRTIM1 = 8,    // Power Reduction Timer/Counter1
        };
    };

    // Watchdog Timer Control Register
    struct WDTCR {
        static const uint8_t address = 0x41;
        enum bits : uint8_t {
            WDP0 = 1,    // Watchdog Timer Prescaler Bits
            WDP1 = 2,    // Watchdog Timer Prescaler Bits
            WDP2 = 4,    // Watchdog Timer Prescaler Bits
            WDE = 8,    // Watch Dog Enable
            WDCE = 16,    // Watchdog Change Enable
            WDP3 = 32,    // Watchdog Timer Prescaler Bits
            WDIE = 64,    // Watchdog Timeout Interrupt Enable
            WDIF = 128,    // Watchdog Timeout Interrupt Flag
        };
    };

    // debugWire data register
    struct DWDR {
        static const uint8_t address = 0x42;
    };

    // Dead time prescaler register
    struct DTPS {
        static const uint8_t address = 0x43;
        enum bits : uint8_t {
            DTPS0 = 1,    // 
            DTPS1 = 2,    // 
        };
    };

    // Dead time value B
    struct DT1B {
        static const uint8_t address = 0x44;
        enum bits : uint8_t {
            DTVL0 = 1,    // 
            DTVL1 = 2,    // 
            DTVL2 = 4,    // 
            DTVL3 = 8,    // 
            DTVH0 = 16,    // 
            DTVH1 = 32,    // 
            DTVH2 = 64,    // 
            DTVH3 = 128,    // 
        };
    };

    // Dead time value register
    struct DT1A {
        static const uint8_t address = 0x45;
        enum bits : uint8_t {
            DTVL0 = 1,    // 
            DTVL1 = 2,    // 
            DTVL2 = 4,    // 
            DTVL3 = 8,    // 
            DTVH0 = 16,    // 
            DTVH1 = 32,    // 
            DTVH2 = 64,    // 
            DTVH3 = 128,    // 
        };
    };

    // Clock Prescale Register
    struct CLKPR {
        static const uint8_t address = 0x46;
        enum bits : uint8_t {
            CLKPS0 = 1,    // Clock Prescaler Select Bits
            CLKPS1 = 2,    // Clock Prescaler Select Bits
            CLKPS2 = 4,    // Clock Prescaler Select Bits
            CLKPS3 = 8,    // Clock Prescaler Select Bits
            CLKPCE = 128,    // Clock Prescaler Change Enable
        };
    };

    // PLL Control and status register
    struct PLLCSR {
        static const uint8_t address = 0x47;
        enum bits : uint8_t {
            PLOCK = 1,    // PLL Lock detector
            PLLE = 2,    // PLL Enable
            PCKE = 4,    // PCK Enable
            LSM = 128,    // Low speed mode
        };
    };

    // Timer/Counter0 Output Compare Register
    struct OCR0B {
        static const uint8_t address = 0x48;
    };

    // Timer/Counter0 Output Compare Register
    struct OCR0A {
        static const uint8_t address = 0x49;
    };

    // Timer/Counter  Control Register A
    struct TCCR0A {
        static const uint8_t address = 0x4a;
        enum bits : uint8_t {
            WGM00 = 1,    // Waveform Generation Mode
            WGM01 = 2,    // Waveform Generation Mode
            COM0B0 = 16,    // Compare Output Mode, Fast PWm
            COM0B1 = 32,    // Compare Output Mode, Fast PWm
            COM0A0 = 64,    // Compare Output Mode, Phase Correct PWM Mode
            COM0A1 = 128,    // Compare Output Mode, Phase Correct PWM Mode
        };
    };

    // Output Compare Register
    struct OCR1B {
        static const uint8_t address = 0x4b;
    };

    // General Timer/Counter Control Register
    struct GTCCR {
        static const uint8_t address = 0x4c;
        enum bits : uint8_t {
            PSR0 = 1,    // Prescaler Reset Timer/Counter1 and Timer/Counter0
            PSR1 = 2,    // Prescaler Reset Timer/Counter1
            FOC1A = 4,    // Force Output Compare 1A
            FOC1B = 8,    // Force Output Compare Match 1B
            COM1B0 = 16,    // Comparator B Output Mode
            COM1B1 = 32,    // Comparator B Output Mode
            PWM1B = 64,    // Pulse Width Modulator B Enable
            TSM = 128,    // Timer/Counter Synchronization Mode
        };
    };

    // Output compare register
    struct OCR1C {
        static const uint8_t address = 0x4d;
    };

    // Output Compare Register
    struct OCR1A {
        static const uint8_t address = 0x4e;
    };

    // Timer/Counter Register
    struct TCNT1 {
        static const uint8_t address = 0x4f;
    };

    // Timer/Counter Control Register
    struct TCCR1 {
        static const uint8_t address = 0x50;
        enum bits : uint8_t {
            CS10 = 1,    // Clock Select Bits
            CS11 = 2,    // Clock Select Bits
            CS12 = 4,    // Clock Select Bits
            CS13 = 8,    // Clock Select Bits
            COM1A0 = 16,    // Compare Output Mode, Bits
            COM1A1 = 32,    // Compare Output Mode, Bits
            PWM1A = 64,    // Pulse Width Modulator Enable
            CTC1 = 128,    // Clear Timer/Counter on Compare Match
        };
    };

    // Oscillator Calibration Register
    struct OSCCAL {
        static const uint8_t address = 0x51;
        enum bits : uint8_t {
            OSCCAL0 = 1,    // Oscillator Calibration
            OSCCAL1 = 2,    // Oscillator Calibration
            OSCCAL2 = 4,    // Oscillator Calibration
            OSCCAL3 = 8,    // Oscillator Calibration
            OSCCAL4 = 16,    // Oscillator Calibration
            OSCCAL5 = 32,    // Oscillator Calibration
            OSCCAL6 = 64,    // Oscillator Calibration
            OSCCAL7 = 128,    // Oscillator Calibration
        };
    };

    // Timer/Counter0
    struct TCNT0 {
        static const uint8_t address = 0x52;
    };

    // Timer/Counter Control Register B
    struct TCCR0B {
        static const uint8_t address = 0x53;
        enum bits : uint8_t {
            CS00 = 1,    // Clock Select
            CS01 = 2,    // Clock Select
            CS02 = 4,    // Clock Select
            WGM02 = 8,    // 
            FOC0B = 64,    // Force Output Compare B
            FOC0A = 128,    // Force Output Compare A
        };
    };

    // MCU Status register
    struct MCUSR {
        static const uint8_t address = 0x54;
        enum bits : uint8_t {
            PORF = 1,    // Power-On Reset Flag
            EXTRF = 2,    // External Reset Flag
            BORF = 4,    // Brown-out Reset Flag
            WDRF = 8,    // Watchdog Reset Flag
        };
    };

    // MCU Control Register
    struct MCUCR {
        static const uint8_t address = 0x55;
        enum bits : uint8_t {
            ISC00 = 1,    // Interrupt Sense Control 0 Bit 0
            ISC01 = 2,    // Interrupt Sense Control 0 Bit 1
            SM0 = 8,    // Sleep Mode Select Bits
            SM1 = 16,    // Sleep Mode Select Bits
            SE = 32,    // Sleep Enable
            PUD = 64,    // Pull-up Disable
        };
    };

    // Store Program Memory Control Register
    struct SPMCSR {
        static const uint8_t address = 0x57;
        enum bits : uint8_t {
            SPMEN = 1,    // Store Program Memory Enable
            PGERS = 2,    // Page Erase
            PGWRT = 4,    // Page Write
            RFLB = 8,    // Read fuse and lock bits
            CTPB = 16,    // Clear temporary page buffer
            RSIG = 32,    // Read Device Signature Imprint Table
        };
    };

    // Timer/Counter0 Interrupt Flag register
    struct TIFR {
        static const uint8_t address = 0x58;
        enum bits : uint8_t {
            TOV0 = 2,    // Timer/Counter0 Overflow Flag
            TOV1 = 4,    // Timer/Counter1 Overflow Flag
            OCF0B = 8,    // Timer/Counter0 Output Compare Flag 0B
            OCF0A = 16,    // Timer/Counter0 Output Compare Flag 0A
            OCF1B = 32,    // Timer/Counter1 Output Compare Flag 1B
            OCF1A = 64,    // Timer/Counter1 Output Compare Flag 1A
        };
    };

    // Timer/Counter Interrupt Mask Register
    struct TIMSK {
        static const uint8_t address = 0x59;
        enum bits : uint8_t {
            TOIE0 = 2,    // Timer/Counter0 Overflow Interrupt Enable
            TOIE1 = 4,    // Timer/Counter1 Overflow Interrupt Enable
            OCIE0B = 8,    // Timer/Counter0 Output Compare Match B Interrupt Enable
            OCIE0A = 16,    // Timer/Counter0 Output Compare Match A Interrupt Enable
            OCIE1B = 32,    // OCIE1A: Timer/Counter1 Output Compare B Interrupt Enable
            OCIE1A = 64,    // OCIE1A: Timer/Counter1 Output Compare Interrupt Enable
        };
    };

    // General Interrupt Flag register
    struct GIFR {
        static const uint8_t address = 0x5a;
        enum bits : uint8_t {
            PCIF = 32,    // Pin Change Interrupt Flag
            INTF0 = 64,    // External Interrupt Flag 0
        };
    };

    // General Interrupt Mask Register
    struct GIMSK {
        static const uint8_t address = 0x5b;
        enum bits : uint8_t {
            PCIE = 32,    // Pin Change Interrupt Enable
            INT0 = 64,    // External Interrupt Request 0 Enable
        };
    };

    // Status Register
    struct SREG {
        static const uint8_t address = 0x5f;
        enum bits : uint8_t {
            C = 1,    // Carry Flag
            Z = 2,    // Zero Flag
            N = 4,    // Negative Flag
            V = 8,    // Two's Complement Overflow Flag
            S = 16,    // Sign Bit
            H = 32,    // Half Carry Flag
            T = 64,    // Bit Copy Storage
            I = 128,    // Global Interrupt Enable
        };
    };


}

#endif