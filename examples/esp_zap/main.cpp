//
// Created by fatih on 2/22/19.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/utility.hpp>

#include <arch/lx106/drivers.hpp>
#include <tos/version.hpp>
#include <tos/fixed_fifo.hpp>

#include <lwip/init.h>
#include <common/inet/tcp_stream.hpp>

#include <flatbuffers/flatbuffers.h>
#include "req_generated.h"

auto zap_task = []{
    tos::esp82::wifi w;
    flatbuffers::FlatBufferBuilder builder(256);
    zap::cloud::RequestBuilder b(builder);
};

void tos_main()
{
    lwip_init();
    tos::launch(zap_task);
}
