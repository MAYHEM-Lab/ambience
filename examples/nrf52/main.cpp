//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>

#include <nrf_delay.h>
#include <nrf_gpio.h>

#define NRF_GPIO NRF_P0

#define LOW 0

void digitalWrite( uint32_t ulPin, uint32_t ulVal )
{
    switch ( ulVal )
    {
        case LOW:
            NRF_GPIO->OUTCLR = (1UL << ulPin);
            break ;

        default:
            NRF_GPIO->OUTSET = (1UL << ulPin);
            break ;
    }
}

void ledOn(uint32_t pin)
{
    digitalWrite(pin, 1);
}

void ledOff(uint32_t pin)
{
    digitalWrite(pin, 0);
}

void led1_task()
{
    while (true)
    {
        ledOn(17);
        //nrf_delay_ms(100);
        for (int i = 0; i < 1000; ++i)
        {
            nrf_delay_us(100);
            tos::this_thread::yield();
        }

        ledOff(17);
        nrf_delay_ms(100);
        for (int i = 0; i < 1000; ++i)
        {
            nrf_delay_us(100);
            tos::this_thread::yield();
        }
    }
}

void led2_task()
{
    while (true)
    {
        ledOn(19);
        for (int i = 0; i < 100; ++i)
        {
            nrf_delay_us(100);
            tos::this_thread::yield();
        }

        ledOff(19);
        for (int i = 0; i < 100; ++i)
        {
            nrf_delay_us(100);
            tos::this_thread::yield();
        }
    }
}

void tos_main()
{
    NRF_CLOCK->LFCLKSRC = (uint32_t)((CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos) & CLOCK_LFCLKSRC_SRC_Msk);
    NRF_CLOCK->TASKS_LFCLKSTART = 1UL;

    // RTC1 could be enabled by bootloader. Disable it
    NVIC_DisableIRQ(RTC1_IRQn);
    NRF_RTC1->EVTENCLR    = RTC_EVTEN_COMPARE0_Msk;
    NRF_RTC1->INTENCLR    = RTC_INTENSET_COMPARE0_Msk;
    NRF_RTC1->TASKS_STOP  = 1;
    NRF_RTC1->TASKS_CLEAR = 1;

    NRF_P0->OUTSET = UINT32_MAX;

    NRF_P0->PIN_CNF[17] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

    NRF_P0->PIN_CNF[19] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

    tos::launch(led1_task);
    tos::launch(led2_task);
}