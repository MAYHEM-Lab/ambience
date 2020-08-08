#pragma once

#include <cstdint>
#include <common/alarm.hpp>

struct bluenrg_interface {
    virtual int32_t BlueNRG_SPI_Read_All(uint8_t *buffer,
                                 uint8_t buff_size) = 0;
    virtual int32_t BlueNRG_SPI_Write(uint8_t* data1,
                              uint8_t* data2,
                              uint8_t Nb_bytes1,
                              uint8_t Nb_bytes2) = 0;
    virtual void Hal_Write_Serial(const void* data1, const void* data2, int32_t n_bytes1,
                          int32_t n_bytes2) = 0;
    virtual void Disable_SPI_IRQ() = 0;
    virtual void Enable_SPI_IRQ() = 0;
    virtual uint8_t BlueNRG_DataPresent() = 0;

    virtual tos::any_alarm* get_alarm() = 0;

    virtual void HCI_Event_CB(void* pckt) = 0;

    virtual ~bluenrg_interface() = default;
};

void HCI_Init(bluenrg_interface& arg_interface);