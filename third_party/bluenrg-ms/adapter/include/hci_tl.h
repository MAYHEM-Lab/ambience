#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "bluenrg_conf.h"

#include <ble_list.h>
#include <bluenrg_types.h>

struct hci_request {
    uint16_t ogf;
    uint16_t ocf;
    int event;
    void* cparam;
    int clen;
    void* rparam;
    int rlen;
};

typedef struct _tHciDataPacket {
    tListNode currentNode;
    uint8_t dataBuff[HCI_READ_PACKET_SIZE];
    uint8_t data_len;
} tHciDataPacket;

/**
 * @brief  Send an HCI request either in synchronous or in asynchronous mode.
 *
 * @param  r: The HCI request
 * @param  async: TRUE if asynchronous mode, FALSE if synchronous mode
 * @retval int: 0 when success, -1 when failure
 */
int hci_send_req(struct hci_request* r, BOOL async);

/**
 * Processing function that must be called after an event is received from
 * HCI interface. Must be called outside ISR. It will call HCI_Event_CB if
 * necessary.
*/
void HCI_Process(void);

void HCI_Isr(void);

#ifdef __cplusplus
}
#endif
