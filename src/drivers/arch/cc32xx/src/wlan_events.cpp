//
// Created by fatih on 12/30/19.
//

#include "shared_variables.hpp"

#include <arch/detail/events.hpp>
#include <common/usart.hpp>
#include <ti/drivers/net/wifi/simplelink.h>
#include <tos/fixed_fifo.hpp>
#include <tos/ring_buf.hpp>
#include <tos/semaphore.hpp>

using namespace tos::cc32xx;

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
} tevents;

const char* Roles[] = {"STA", "STA", "AP", "P2P"};
const char* WlanStatus[] = {"DISCONNECTED", "SCANING", "CONNECTING", "CONNECTED"};

void SignalEvent(tevents ev) {}

static tos::null_usart uart;
#define UART_PRINT(...) tos::print(uart, __VA_ARGS__)

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