//
// Created by fatih on 3/20/18.
//

#pragma once

#include <boost/asio/io_service.hpp>

extern "C"
{
    void tos_set_stack_ptr(void*);
}

boost::asio::io_service& get_io();