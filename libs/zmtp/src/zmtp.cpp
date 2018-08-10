//
// Created by fatih on 8/10/18.
//

#include <tos/zmtp/zmtplib.h>
#include <drivers/arch/lx106/tcp.hpp>
#include <drivers/common/inet/tcp_stream.hpp>

using namespace tos;

template <class SockT>
void tcp_send (tcp_stream<SockT>& stream, const void *buffer, size_t len)
{
    auto res = stream.write({ (const char*)buffer, len });
    if (res != len)
    {
        //err
    }
}

template <class SockT>
void tcp_recv (tcp_stream<SockT>& stream, void *buffer, size_t len)
{
    size_t len_recd = 0;
    while (len_recd < len)
    {
        auto cont = with(stream.read({ (const char*)buffer + len_recd, len - len_recd }),
        [&](span<char>& res){
            len_recd += res.size();
            return true;
        }, [](auto&){
            return false;
        });
        if (!cont)
        {
            // err
        }
    }
}

template <class SockT>
void zmtp_recv (tcp_stream<SockT&> handle, zmtp_msg_t *msg)
{
    tcp_recv (handle, (byte *) msg, 2);
    tcp_recv (handle, msg->data, msg->size);
}

template <class SockT>
void zmtp_send (tcp_stream<SockT>& handle, zmtp_msg_t *msg)
{
    tcp_send (handle, (byte *) msg, msg->size + 2);
}
