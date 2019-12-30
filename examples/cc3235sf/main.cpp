//
// Created by fatih on 10/31/19.
//

#include "tos/stack_storage.hpp"
#include "tos/thread.hpp"

#include <arch/detail/sock_rt.hpp>
#include <arch/drivers.hpp>
#include <arch/tcp.hpp>
#include <arch/udp.hpp>
#include <arch/wlan.hpp>
#include <common/inet/tcp_ip.hpp>
#include <common/usart.hpp>
#include <cstring>
#include <nonstd/variant.hpp>
#include <ti/drivers/crypto/CryptoCC32XX.h>
#include <ti/drivers/net/wifi/simplelink.h>
#include <tos/fixed_fifo.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/streams.hpp>

std::array<uint8_t, 32> hashv;
void hash(tos::span<const uint8_t> buffer) {
    CryptoCC32XX_init();
    CryptoCC32XX_HmacParams params;
    CryptoCC32XX_HmacParams_init(&params);
    auto cryptoHandle = CryptoCC32XX_open(0, CryptoCC32XX_HMAC);
    if (!cryptoHandle) {
        tos::debug::panic("can't open crypto");
    }

    CryptoCC32XX_sign(cryptoHandle,
                      CryptoCC32XX_HMAC_SHA256,
                      const_cast<uint8_t*>(buffer.data()),
                      buffer.size(),
                      hashv.data(),
                      &params);
    CryptoCC32XX_close(cryptoHandle);
}

typedef enum
{
    APP_EVENT_NULL,
    APP_EVENT_STARTED,
    APP_EVENT_CONNECTED,
    APP_EVENT_IP_ACQUIRED,
    APP_EVENT_DISCONNECT, /* used also for IP lost */
    APP_EVENT_PROVISIONING_STARTED,
    APP_EVENT_PROVISIONING_SUCCESS,
    APP_EVENT_PROVISIONING_STOPPED,
    APP_EVENT_PING_COMPLETE,
    APP_EVENT_OTA_START,
    APP_EVENT_CONTINUE,
    APP_EVENT_OTA_CHECK_DONE,
    APP_EVENT_OTA_DOWNLOAD_DONE,
    APP_EVENT_OTA_ERROR,
    APP_EVENT_TIMEOUT,
    APP_EVENT_ERROR,
    APP_EVENT_RESTART,
    APP_EVENT_MAX
} events;
const char* Roles[] = {"STA", "STA", "AP", "P2P"};
const char* WlanStatus[] = {"DISCONNECTED", "SCANING", "CONNECTING", "CONNECTED"};

tos::basic_fixed_fifo<tos::cc32xx::events, 20, tos::ring_buf> evq;
void SignalEvent(events ev) {
}

tos::any_usart* uart;
#define UART_PRINT(...) tos::print(uart, __VA_ARGS__)

tos::semaphore loop_sem{0};

extern "C" {
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t* pDevEvent) {
    if (NULL == pDevEvent) {
        return;
    }
    switch (pDevEvent->Id) {
    default: {
        if (pDevEvent->Data.Error.Code == SL_ERROR_LOADING_CERTIFICATE_STORE) {
            /* Ignore it */
            /*UART_PRINT("GeneralEventHandler: EventId=%d,"
                       "SL_ERROR_LOADING_CERTIFICATE_STORE, ErrorCode=%d\r\n",
                       pDevEvent->Id,
                       pDevEvent->Data.Error.Code);*/
            break;
        }
        /*UART_PRINT("Received unexpected General Event with code [0x%x]\r\n",
                   pDevEvent->Data.Error.Code);
        SignalEvent;*/
    } break;
    }
}
void SimpleLinkWlanEventHandler(SlWlanEvent_t* pWlanEvent) {
    SlWlanEventData_u* pWlanEventData = NULL;

    if (NULL == pWlanEvent) {
        return;
    }

    pWlanEventData = &pWlanEvent->Data;

    switch (pWlanEvent->Id) {
    case SL_WLAN_EVENT_CONNECT: {
        UART_PRINT("Got connection ev\r\n");
        evq.push(tos::cc32xx::wifi_connected{pWlanEvent->Data.Connect});
    } break;

    case SL_WLAN_EVENT_DISCONNECT: {
        SlWlanEventDisconnect_t* pDiscntEvtData = NULL;
        pDiscntEvtData = &pWlanEventData->Disconnect;

        /** If the user has initiated 'Disconnect' request, 'ReasonCode'
         * is SL_USER_INITIATED_DISCONNECTION
         */
        if (SL_WLAN_DISCONNECT_USER_INITIATED == pDiscntEvtData->ReasonCode) {
            UART_PRINT("Device disconnected from the AP on request\r\n");
        } else {
            UART_PRINT("Device disconnected from the AP on an ERROR\r\n");
        }

        SignalEvent(APP_EVENT_DISCONNECT);
    } break;

    case SL_WLAN_EVENT_PROVISIONING_PROFILE_ADDED:
        UART_PRINT(" [Provisioning] Profile Added: SSID: %s\r\n",
                   pWlanEvent->Data.ProvisioningProfileAdded.Ssid);
        if (pWlanEvent->Data.ProvisioningProfileAdded.ReservedLen > 0) {
            UART_PRINT(" [Provisioning] Profile Added: PrivateToken:%s\r\n",
                       pWlanEvent->Data.ProvisioningProfileAdded.Reserved);
        }
        break;

    case SL_WLAN_EVENT_PROVISIONING_STATUS: {
        switch (pWlanEvent->Data.ProvisioningStatus.ProvisioningStatus) {
        case SL_WLAN_PROVISIONING_GENERAL_ERROR:
        case SL_WLAN_PROVISIONING_ERROR_ABORT:
        case SL_WLAN_PROVISIONING_ERROR_ABORT_INVALID_PARAM:
        case SL_WLAN_PROVISIONING_ERROR_ABORT_HTTP_SERVER_DISABLED:
        case SL_WLAN_PROVISIONING_ERROR_ABORT_PROFILE_LIST_FULL:
        case SL_WLAN_PROVISIONING_ERROR_ABORT_PROVISIONING_ALREADY_STARTED:
            UART_PRINT(" [Provisioning] Provisioning Error status=%d\r\n",
                       pWlanEvent->Data.ProvisioningStatus.ProvisioningStatus);
            SignalEvent(APP_EVENT_ERROR);
            break;

        case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_FAIL_NETWORK_NOT_FOUND:
            UART_PRINT(" [Provisioning] Profile confirmation failed: network"
                       "not found\r\n");
            SignalEvent(APP_EVENT_PROVISIONING_STARTED);
            break;

        case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_FAIL_CONNECTION_FAILED:
            UART_PRINT(" [Provisioning] Profile confirmation failed: Connection "
                       "failed\r\n");
            SignalEvent(APP_EVENT_PROVISIONING_STARTED);
            break;

        case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_CONNECTION_SUCCESS_IP_NOT_ACQUIRED:
            UART_PRINT(" [Provisioning] Profile confirmation failed: IP address not "
                       "acquired\r\n");
            SignalEvent(APP_EVENT_PROVISIONING_STARTED);
            break;

        case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_SUCCESS_FEEDBACK_FAILED:
            UART_PRINT(" [Provisioning] Profile Confirmation failed (Connection "
                       "Success, feedback to Smartphone app failed)\r\n");
            SignalEvent(APP_EVENT_PROVISIONING_STARTED);
            break;

        case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_SUCCESS:
            UART_PRINT(" [Provisioning] Profile Confirmation Success!\r\n");
            SignalEvent(APP_EVENT_PROVISIONING_SUCCESS);
            break;

        case SL_WLAN_PROVISIONING_AUTO_STARTED:
            UART_PRINT(" [Provisioning] Auto-Provisioning Started\r\n");
            SignalEvent(APP_EVENT_PROVISIONING_STARTED);
            break;

        case SL_WLAN_PROVISIONING_STOPPED:
            UART_PRINT("\r\n Provisioning stopped:");
            UART_PRINT(" Current Role: %s\r\n",
                       Roles[pWlanEvent->Data.ProvisioningStatus.Role]);
            if (ROLE_STA == pWlanEvent->Data.ProvisioningStatus.Role) {
                UART_PRINT("                       WLAN Status: %s\r\n",
                           WlanStatus[pWlanEvent->Data.ProvisioningStatus.WlanStatus]);

                if (SL_WLAN_STATUS_CONNECTED ==
                    pWlanEvent->Data.ProvisioningStatus.WlanStatus) {
                    UART_PRINT("                       Connected to SSID: %s\r\n",
                               pWlanEvent->Data.ProvisioningStatus.Ssid);
                    SignalEvent(APP_EVENT_PROVISIONING_STOPPED);
                } else {
                    SignalEvent(APP_EVENT_PROVISIONING_STARTED);
                }
            } else {
                SignalEvent(APP_EVENT_PROVISIONING_STOPPED);
            }
            // g_StopInProgress = 0;
            break;

        case SL_WLAN_PROVISIONING_SMART_CONFIG_SYNCED:
            UART_PRINT(" [Provisioning] Smart Config Synced!\r\n");
            break;

        case SL_WLAN_PROVISIONING_SMART_CONFIG_SYNC_TIMEOUT:
            UART_PRINT(" [Provisioning] Smart Config Sync Timeout!\r\n");
            break;

        case SL_WLAN_PROVISIONING_CONFIRMATION_WLAN_CONNECT:
            UART_PRINT(" [Provisioning] Profile confirmation: WLAN Connected!\r\n");
            break;

        case SL_WLAN_PROVISIONING_CONFIRMATION_IP_ACQUIRED:
            UART_PRINT(" [Provisioning] Profile confirmation: IP Acquired!\r\n");
            break;

        case SL_WLAN_PROVISIONING_EXTERNAL_CONFIGURATION_READY:
            UART_PRINT(" [Provisioning] External configuration is ready! \r\n");
            break;

        default:
            UART_PRINT(" [Provisioning] Unknown Provisioning Status: %d\r\n",
                       pWlanEvent->Data.ProvisioningStatus.ProvisioningStatus);
            break;
        }
    } break;

    default: {
        UART_PRINT("Unexpected WLAN event with Id [0x%x]\r\n", pWlanEvent->Id);
        SignalEvent(APP_EVENT_ERROR);
    } break;
    }
}
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t* pNetAppEvent) {
    SlNetAppEventData_u* pNetAppEventData = NULL;

    if (NULL == pNetAppEvent) {
        return;
    }

    pNetAppEventData = &pNetAppEvent->Data;

    switch (pNetAppEvent->Id) {
    case SL_NETAPP_EVENT_IPV4_ACQUIRED: {
        evq.push(tos::cc32xx::ip_acquired{
            .address =
                tos::ipv4_addr_t{
                    (uint8_t)SL_IPV4_BYTE(pNetAppEventData->IpAcquiredV4.Ip, 3),
                    (uint8_t)SL_IPV4_BYTE(pNetAppEventData->IpAcquiredV4.Ip, 2),
                    (uint8_t)SL_IPV4_BYTE(pNetAppEventData->IpAcquiredV4.Ip, 1),
                    (uint8_t)SL_IPV4_BYTE(pNetAppEventData->IpAcquiredV4.Ip, 0)},
            .gateway = tos::ipv4_addr_t{
                (uint8_t)SL_IPV4_BYTE(pNetAppEventData->IpAcquiredV4.Gateway, 3),
                (uint8_t)SL_IPV4_BYTE(pNetAppEventData->IpAcquiredV4.Gateway, 2),
                (uint8_t)SL_IPV4_BYTE(pNetAppEventData->IpAcquiredV4.Gateway, 1),
                (uint8_t)SL_IPV4_BYTE(pNetAppEventData->IpAcquiredV4.Gateway, 0)}});

        SignalEvent(APP_EVENT_IP_ACQUIRED);
    } break;

    case SL_NETAPP_EVENT_IPV4_LOST:
    case SL_NETAPP_EVENT_DHCP_IPV4_ACQUIRE_TIMEOUT: {
        UART_PRINT("IPv4 lost Id or timeout, Id [0x%x]!!!\r\n", pNetAppEvent->Id);
        /* use existing disconnect event, no need for another event */
        SignalEvent(APP_EVENT_DISCONNECT);
    } break;

    default: {
        UART_PRINT("Unexpected NetApp event with Id [0x%x] \r\n", pNetAppEvent->Id);
        SignalEvent(APP_EVENT_ERROR);
    } break;
    }
}
void SimpleLinkFatalErrorEventHandler(SlDeviceFatal_t* slFatalErrorEvent) {
    switch (slFatalErrorEvent->Id) {
    case SL_DEVICE_EVENT_FATAL_DEVICE_ABORT: {
        UART_PRINT("[ERROR] - FATAL ERROR: Abort NWP event detected: AbortType=%d,"
                   "AbortData=0x%x\r\n",
                   slFatalErrorEvent->Data.DeviceAssert.Code,
                   slFatalErrorEvent->Data.DeviceAssert.Value);
    } break;

    case SL_DEVICE_EVENT_FATAL_DRIVER_ABORT: {
        UART_PRINT("[ERROR] - FATAL ERROR: Driver Abort detected. \r\n");
    } break;

    case SL_DEVICE_EVENT_FATAL_NO_CMD_ACK: {
        UART_PRINT("[ERROR] - FATAL ERROR: No Cmd Ack detected [cmd opcode = 0x%x] \r\n",
                   slFatalErrorEvent->Data.NoCmdAck.Code);
    } break;

    case SL_DEVICE_EVENT_FATAL_SYNC_LOSS: {
        UART_PRINT("[ERROR] - FATAL ERROR: Sync loss detected n\r");
        SignalEvent(APP_EVENT_RESTART);
        return;
    }

    case SL_DEVICE_EVENT_FATAL_CMD_TIMEOUT: {
        UART_PRINT("[ERROR] - FATAL ERROR: Async event timeout detected "
                   "[event opcode =0x%x]  \r\n",
                   slFatalErrorEvent->Data.CmdTimeout.Code);
    } break;

    default:
        UART_PRINT("[ERROR] - FATAL ERROR: Unspecified error detected \r\n");
        break;
    }
    SignalEvent(APP_EVENT_ERROR);
}
void SimpleLinkSockEventHandler(SlSockEvent_t* pSock) {
    switch (pSock->Event) {
    case SL_SOCKET_ASYNC_EVENT:
        pSock->SocketAsyncEvent.SockAsyncData.pExtraInfo;
        break;
    }
    if (pSock->Event == SL_SOCKET_TX_FAILED_EVENT) {
        /* on socket error Restart OTA */
        UART_PRINT("SL_SOCKET_TX_FAILED_EVENT socket event %d, do restart\r\n",
                   pSock->Event);
        SignalEvent(APP_EVENT_RESTART);
    } else if (pSock->Event == SL_SOCKET_ASYNC_EVENT) {
        /* on socket error Restart OTA */
        UART_PRINT("SL_SOCKET_ASYNC_EVENT socket event %d, do restart\r\n", pSock->Event);
        SignalEvent(APP_EVENT_RESTART);
    } else {
        /* Unused in this application */
        UART_PRINT("Unexpected socket event %d\r\n", pSock->Event);
        SignalEvent(APP_EVENT_ERROR);
    }
}
void SimpleLinkHttpServerEventHandler(SlNetAppHttpServerEvent_t* pHttpEvent,
                                      SlNetAppHttpServerResponse_t* pHttpResponse) {
    /* Unused in this application */
    UART_PRINT("Unexpected HTTP server event \r\n");
    SignalEvent(APP_EVENT_ERROR);
}
void SimpleLinkNetAppRequestHandler(SlNetAppRequest_t* pNetAppRequest,
                                    SlNetAppResponse_t* pNetAppResponse) {
    /* Unused in this application */
    UART_PRINT("Unexpected NetApp request event \r\n");
    SignalEvent(APP_EVENT_ERROR);
}
void SimpleLinkNetAppRequestMemFreeEventHandler(uint8_t* buffer) {
    /* do nothing... */
}

void SimpleLinkNetAppRequestEventHandler(SlNetAppRequest_t* pNetAppRequest,
                                         SlNetAppResponse_t* pNetAppResponse) {
    /* do nothing... */
}

void cc32xx_notify_loop() {
    loop_sem.up_isr();
}
}

void udp_socket() {
    tos::cc32xx::udp_socket sock;
    sock.bind({5001});

    sock.send_to(tos::raw_cast<const uint8_t>(tos::span<const char>("hello")),
                 {.addr = {{192, 168, 43, 120}}, .port = 5001});

    while (true) {
        std::array<uint8_t, 32> buffer;
        tos::udp_endpoint_t from;
        auto res = sock.receive_from(buffer, from);
        tos::println(uart, "Received:", tos::raw_cast<const char>(force_get(res)));
    }
}

tos::cc32xx::tcp_socket* get_connection() {
    tos::cc32xx::tcp_listener listener({8080});
    listener.listen();
    auto sock = listener.accept();
    tos::println(uart, "accept returned", bool(sock));
    if (!sock) {
        tos::println(uart, "accept error!");
        return nullptr;
    }
    return force_get(sock);
}

void tcp_socket() {
    auto socket_ptr = get_connection();
    tos::println(uart, "socket:", socket_ptr->native_handle());
    std::array<char, 32> buffer;
    auto line = tos::read_until<char>(socket_ptr, "\n", buffer);
    tos::println(uart, "Socket received:", line);
    socket_ptr->write(tos::raw_cast<const uint8_t>(line));
}

void wifi(tos::any_usart& log) {
    using namespace tos::cc32xx;
    auto start_res = sl_Start(nullptr, nullptr, nullptr);
    tos::println(log, start_res);
    auto set_mode = sl_WlanSetMode(ROLE_STA);
    tos::println(log, set_mode);
    auto stop = sl_Stop(0);
    tos::println(log, stop);
    start_res = sl_Start(nullptr, nullptr, nullptr);
    tos::println(log, start_res);

    SlDeviceVersion_t firmwareVersion{};

    uint8_t ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    uint16_t ucConfigLen = sizeof(firmwareVersion);
    auto retVal = sl_DeviceGet(
        SL_DEVICE_GENERAL, &ucConfigOpt, &ucConfigLen, (uint8_t*)(&firmwareVersion));

    tos::println(log, "Host Driver Version:", SL_DRIVER_VERSION);
    tos::println(log,
                 "Build Version",
                 int(firmwareVersion.NwpVersion[0]),
                 int(firmwareVersion.NwpVersion[1]),
                 int(firmwareVersion.NwpVersion[2]),
                 int(firmwareVersion.NwpVersion[3]),
                 int(firmwareVersion.FwVersion[0]),
                 int(firmwareVersion.FwVersion[1]),
                 int(firmwareVersion.FwVersion[2]),
                 int(firmwareVersion.FwVersion[3]),
                 int(firmwareVersion.PhyVersion[0]),
                 int(firmwareVersion.PhyVersion[1]),
                 int(firmwareVersion.PhyVersion[2]),
                 int(firmwareVersion.PhyVersion[3]),
                 tos::separator('.'));

    auto set_res = set_mac_address({0xDA, 0x53, 0x83, 0x81, 0x41, 0x6B});
    if (!set_res) {
        tos::println(log, "Can't set mac address!");
    }

    auto mac = get_mac_address();
    tos::println(log, "Mac Address:", mac);

    const char* password = "mykonos1993";
    SlWlanSecParams_t SecParams;
    SecParams.Type = SL_WLAN_SEC_TYPE_WPA_WPA2;
    SecParams.Key = reinterpret_cast<int8_t*>(const_cast<char*>(password));
    SecParams.KeyLen = strlen(password);

    const char* name = "GoksuNetwork";
    auto res = sl_WlanConnect(reinterpret_cast<const int8_t*>(name),
                              std::strlen(name),
                              nullptr,
                              &SecParams,
                              nullptr);

    tos::println(log, "connect:", int(res));

    using namespace tos::cc32xx;

    tos::launch(tos::alloc_stack, [] {
        while (true) {
            socket_runtime::instance().run();
        }
    });

    while (true) {
        sl_Task(nullptr);

        while (!evq.empty()) {
            auto ev = evq.pop();
            mpark::visit(tos::make_overload(
                             [&](const wifi_connected& ev) {
                                 tos::println(log, "Connected:", ev.ssid());
                             },
                             [&](const ip_acquired& ev) {
                                 tos::println(log, "Acquired IP:", ev.address);
                                 tos::println(log, "Gateway IP:", ev.gateway);
                                 tos::launch(tos::alloc_stack, ::tcp_socket);
                             },
                             [&](const auto&) { tos::println(log, "Unhandled event"); }),
                         ev);
        }

        loop_sem.down();
    }
}

tos::any_usart* log;
tos::stack_storage<4096> wifistack;
void task() {
    uint8_t buff[] = {'a', 'b', 'c'};
    hash(buff);

    using namespace tos::tos_literals;
    auto pin = 4_pin;
    tos::cc32xx::gpio g;

    g.set_pin_mode(5_pin, tos::pin_mode::out);
    g.set_pin_mode(6_pin, tos::pin_mode::out);
    g.set_pin_mode(pin, tos::pin_mode::out);
    g.write(pin, tos::digital::high);

    tos::cc32xx::uart uart(0);
    auto erased = tos::erase_usart(&uart);
    ::uart = &erased;
    ::log = &erased;

    uart.write(hashv);
    tos::println(uart);

    tos::launch(wifistack, [] {
        SPI_init();
        wifi(*::uart);
    });

    tos::cc32xx::timer tim(0);
    tos::alarm alarm(tim);

    while (true) {
        using namespace std::chrono_literals;
        g.write(pin, tos::digital::high);
        // tos::println(uart, "high");
        tos::this_thread::sleep_for(alarm, 1000ms);
        g.write(pin, tos::digital::low);
        // tos::println(uart, "low");
        tos::this_thread::sleep_for(alarm, 1000ms);
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, task);
}
