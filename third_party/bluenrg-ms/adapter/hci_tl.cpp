#include "hci_tl.h"

#include "bluenrg_conf.h"
#include "bluenrg_interface.hpp"

#include <common/alarm.hpp>
extern "C" {
#include "hci_const.h"
}

namespace {
bluenrg_interface* interface;

tListNode hciReadPktPool;
tListNode hciReadPktRxQueue;

struct timer {
    timer(tos::any_alarm& a)
        : alarm{&a} {
    }
    tos::any_alarm* alarm;
    std::optional<tos::sleeper> sleeper;
    std::optional<tos::any_alarm::alarm_handle> handle;
    ~timer() {
        if (handle) {
            alarm->cancel(*handle);
        }
    }
};

void Timer_Set(timer* t, int interval) {
    using namespace std::chrono_literals;
    t->sleeper.emplace(std::chrono::milliseconds(interval) / t->alarm->resolution(),
                       tos::function_ref<void()>(
                           [](void* tptr) {
                               auto t = static_cast<timer*>(tptr);
                               t->sleeper.reset();
                               t->handle.reset();
                           },
                           t));
    t->handle = t->alarm->set_alarm(*t->sleeper);
}

int Timer_Expired(timer* t) {
    return !bool(t->sleeper);
}

void hci_write(const void* data1, const void* data2, uint8_t n_bytes1, uint8_t n_bytes2) {
#if HCI_LOG_ON
    PRINTF("HCI <- ");
    for (int i = 0; i < n_bytes1; i++) PRINTF("%02X ", *((uint8_t*)data1 + i));
    for (int i = 0; i < n_bytes2; i++) PRINTF("%02X ", *((uint8_t*)data2 + i));
    PRINTF("\n");
#endif

    interface->write(data1, data2, n_bytes1, n_bytes2);
}

BOOL HCI_Queue_Empty(void) {
    return list_is_empty(&hciReadPktRxQueue);
}

#define HCI_PCK_TYPE_OFFSET 0
#define EVENT_PARAMETER_TOT_LEN_OFFSET 2

/**
 * Verify if HCI packet is correctly formatted.
 *
 * @param[in] hciReadPacket    The packet that is received from HCI interface.
 * @return 0 if HCI packet is as expected
 */
int HCI_verify(const tHciDataPacket* hciReadPacket) {
    const uint8_t* hci_pckt = hciReadPacket->dataBuff;

    if (hci_pckt[HCI_PCK_TYPE_OFFSET] != HCI_EVENT_PKT)
        return 1; /* Incorrect type. */

    if (hci_pckt[EVENT_PARAMETER_TOT_LEN_OFFSET] !=
        hciReadPacket->data_len - (1 + HCI_EVENT_HDR_SIZE))
        return 2; /* Wrong length (packet truncated or too long). */

    return 0;
}

void hci_send_cmd(uint16_t ogf, uint16_t ocf, uint8_t plen, void* param) {
    hci_command_hdr hc;

    hc.opcode = htobs(cmd_opcode_pack(ogf, ocf));
    hc.plen = plen;

    uint8_t header[HCI_HDR_SIZE + HCI_COMMAND_HDR_SIZE];
    header[0] = HCI_COMMAND_PKT;
    memcpy(header + 1, &hc, sizeof(hc));

    hci_write(header, param, sizeof(header), plen);
}
/* It ensures that we have at least half of the free buffers in the pool. */
static void free_event_list(void) {
    tHciDataPacket* pckt;

    interface->disable_irq();

    constexpr auto HCI_READ_PACKET_NUM_MAX = 5;

    while (list_get_size(&hciReadPktPool) < HCI_READ_PACKET_NUM_MAX / 2) {
        list_remove_head(&hciReadPktRxQueue, (tListNode**)&pckt);
        list_insert_tail(&hciReadPktPool, (tListNode*)pckt);
        /* Explicit call to HCI_Isr(), since it cannot be called by ISR if IRQ is kept
        high by BlueNRG */
        HCI_Isr();
    }

    interface->enable_irq();
}

static void move_list(tListNode* dest_list, tListNode* src_list) {
    pListNode tmp_node;

    while (!list_is_empty(src_list)) {
        list_remove_tail(src_list, &tmp_node);
        list_insert_head(dest_list, tmp_node);
    }
}

constexpr auto HCI_READ_PACKET_NUM_MAX = 5;
tHciDataPacket hciReadPacketBuffer[HCI_READ_PACKET_NUM_MAX];
} // namespace

void HCI_Init(bluenrg_interface& arg_interface) {
    ::interface = &arg_interface;

    /* Initialize list heads of ready and free hci data packet queues */
    list_init_head(&hciReadPktPool);
    list_init_head(&hciReadPktRxQueue);

    /* Initialize the queue of free hci data packets */
    for (int index = 0; index < HCI_READ_PACKET_NUM_MAX; index++) {
        list_insert_tail(&hciReadPktPool, (tListNode*)&hciReadPacketBuffer[index]);
    }
}

extern "C" {
void HCI_Isr() {
    tHciDataPacket* hciReadPacket = nullptr;
    uint8_t data_len;

    while (interface->data_present()) {
        if (list_is_empty(&hciReadPktPool) == FALSE) {

            /* enqueueing a packet for read */
            list_remove_head(&hciReadPktPool, (tListNode**)&hciReadPacket);

            data_len = interface->read(hciReadPacket->dataBuff, HCI_READ_PACKET_SIZE);
            if (data_len > 0) {
                hciReadPacket->data_len = data_len;
                if (HCI_verify(hciReadPacket) == 0)
                    list_insert_tail(&hciReadPktRxQueue, (tListNode*)hciReadPacket);
                else
                    list_insert_head(&hciReadPktPool, (tListNode*)hciReadPacket);
            } else {
                // Insert the packet back into the pool.
                list_insert_head(&hciReadPktPool, (tListNode*)hciReadPacket);
            }

            tos::this_thread::yield();
        } else {
            // HCI Read Packet Pool is empty, wait for a free packet.
            return;
        }
    }
}

void HCI_Process(void) {
    tHciDataPacket* hciReadPacket = NULL;

    interface->disable_irq();
    uint8_t list_empty = list_is_empty(&hciReadPktRxQueue);
    /* process any pending events read */
    while (list_empty == FALSE) {
        list_remove_head(&hciReadPktRxQueue, (tListNode**)&hciReadPacket);
        interface->enable_irq();
        interface->event_cb(hciReadPacket->dataBuff);
        interface->disable_irq();
        list_insert_tail(&hciReadPktPool, (tListNode*)hciReadPacket);
        list_empty = list_is_empty(&hciReadPktRxQueue);
    }
    /* Explicit call to HCI_Isr(), since it cannot be called by ISR if IRQ is kept high by
    BlueNRG. */
    HCI_Isr();
    interface->enable_irq();
}

int hci_send_req(struct hci_request* r, BOOL async) {
    uint8_t* ptr;
    uint16_t opcode = htobs(cmd_opcode_pack(r->ogf, r->ocf));
    hci_event_pckt* event_pckt;
    hci_uart_pckt* hci_hdr;
    int to = HCI_DEFAULT_TIMEOUT_MS;
    timer t{*interface->get_alarm()};

    tHciDataPacket* hciReadPacket = NULL;
    tListNode hciTempQueue;

    list_init_head(&hciTempQueue);

    free_event_list();

    hci_send_cmd(r->ogf, r->ocf, r->clen, r->cparam);

    if (async) {
        return 0;
    }

    /* Minimum timeout is 1. */
    if (to == 0)
        to = 1;

    Timer_Set(&t, to);

    while (1) {
        evt_cmd_complete* cc;
        evt_cmd_status* cs;
        evt_le_meta_event* me;
        int len;

        while (1) {
            if (Timer_Expired(&t)) {
                LOG_WARN("Timed out");
                goto failed;
            }
            if (!HCI_Queue_Empty()) {
                break;
            }
            tos::this_thread::yield();
        }

        /* Extract packet from HCI event queue. */
        interface->disable_irq();
        list_remove_head(&hciReadPktRxQueue, (tListNode**)&hciReadPacket);

        hci_hdr = (hci_uart_pckt*)hciReadPacket->dataBuff;

        if (hci_hdr->type == HCI_EVENT_PKT) {

            event_pckt = (hci_event_pckt*)(hci_hdr->data);

            ptr = hciReadPacket->dataBuff + (1 + HCI_EVENT_HDR_SIZE);
            len = hciReadPacket->data_len - (1 + HCI_EVENT_HDR_SIZE);

            switch (event_pckt->evt) {

            case EVT_CMD_STATUS:
                cs = (evt_cmd_status*)ptr;

                if (cs->opcode != opcode)
                    goto failed;

                if (r->event != EVT_CMD_STATUS) {
                    if (cs->status) {
                        goto failed;
                    }
                    break;
                }

                r->rlen = std::min(len, r->rlen);
                memcpy(r->rparam, ptr, r->rlen);
                goto done;

            case EVT_CMD_COMPLETE:
                cc = (evt_cmd_complete*)ptr;

                if (cc->opcode != opcode)
                    goto failed;

                ptr += EVT_CMD_COMPLETE_SIZE;
                len -= EVT_CMD_COMPLETE_SIZE;

                r->rlen = std::min(len, r->rlen);
                memcpy(r->rparam, ptr, r->rlen);
                goto done;

            case EVT_LE_META_EVENT:
                me = (evt_le_meta_event*)ptr;

                if (me->subevent != r->event)
                    break;

                len -= 1;
                r->rlen = std::min(len, r->rlen);
                memcpy(r->rparam, me->data, r->rlen);
                goto done;

            case EVT_HARDWARE_ERROR:
                goto failed;

            default:
                break;
            }
        }

        /* If there are no more packets to be processed, be sure there is at list one
           packet in the pool to process the expected event.
           If no free packets are available, discard the processed event and insert it
           into the pool. */
        if (list_is_empty(&hciReadPktPool) && list_is_empty(&hciReadPktRxQueue)) {
            list_insert_tail(&hciReadPktPool, (tListNode*)hciReadPacket);
            hciReadPacket = NULL;
        } else {
            /* Insert the packet in a different queue. These packets will be
            inserted back in the main queue just before exiting from send_req(), so that
            these events can be processed by the application.
          */
            list_insert_tail(&hciTempQueue, (tListNode*)hciReadPacket);
            hciReadPacket = NULL;
        }

        HCI_Isr();

        interface->enable_irq();
    }

failed:
    if (hciReadPacket != NULL) {
        list_insert_head(&hciReadPktPool, (tListNode*)hciReadPacket);
    }
    move_list(&hciReadPktRxQueue, &hciTempQueue);
    interface->enable_irq();
    return -1;

done:
    // Insert the packet back into the pool.
    list_insert_head(&hciReadPktPool, (tListNode*)hciReadPacket);
    move_list(&hciReadPktRxQueue, &hciTempQueue);

    interface->enable_irq();
    return 0;
}
}