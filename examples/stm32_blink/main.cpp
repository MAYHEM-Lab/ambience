//
// Created by fatih on 10/25/18.
//

#include <tos/ft.hpp>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/dma.h>
#include <tos/semaphore.hpp>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/usb/usbd.h>

constexpr usb_device_descriptor dev = []{
    usb_device_descriptor r{};
    r.bLength = USB_DT_DEVICE_SIZE,
    r.bDescriptorType = USB_DT_DEVICE;
    r.bcdUSB = 0x0200;
    r.bDeviceClass = USB_CLASS_CDC;
    r.bDeviceSubClass = 0;
    r.bDeviceProtocol = 0;
    r.bMaxPacketSize0 = 64;
    r.idVendor = 0x0483;
    r.idProduct = 0x5740;
    r.bcdDevice = 0x0200;
    r.iManufacturer = 1;
    r.iProduct = 2;
    r.iSerialNumber = 3;
    r.bNumConfigurations = 1;
    return r;
}();

/*
 * This notification endpoint isn't implemented. According to CDC spec its
 * optional, but its absence causes a NULL pointer dereference in Linux
 * cdc_acm driver.
 */
constexpr auto comm_endp = []{
    std::array<usb_endpoint_descriptor, 1> r{};
    r[0].bLength = USB_DT_ENDPOINT_SIZE;
    r[0].bDescriptorType = USB_DT_ENDPOINT;
    r[0].bEndpointAddress = 0x83;
    r[0].bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT;
    r[0].wMaxPacketSize = 16;
    r[0].bInterval = 255;
    return r;
}();

constexpr auto data_endp = []{
    std::array<usb_endpoint_descriptor, 2> r{};

    r[0].bLength = USB_DT_ENDPOINT_SIZE;
    r[0].bDescriptorType = USB_DT_ENDPOINT;
    r[0].bEndpointAddress = 0x01;
    r[0].bmAttributes = USB_ENDPOINT_ATTR_BULK;
    r[0].wMaxPacketSize = 64;
    r[0].bInterval = 1;


    r[1].bLength = USB_DT_ENDPOINT_SIZE;
    r[1].bDescriptorType = USB_DT_ENDPOINT;
    r[1].bEndpointAddress = 0x82;
    r[1].bmAttributes = USB_ENDPOINT_ATTR_BULK;
    r[1].wMaxPacketSize = 64;
    r[1].bInterval = 1;

    return r;
}();

static const struct {
    struct usb_cdc_header_descriptor header;
    struct usb_cdc_call_management_descriptor call_mgmt;
    struct usb_cdc_acm_descriptor acm;
    struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
    {
        sizeof(struct usb_cdc_header_descriptor),
        CS_INTERFACE,
        USB_CDC_TYPE_HEADER,
        0x0110
    },
    {
        sizeof(struct usb_cdc_call_management_descriptor),
        CS_INTERFACE,
        USB_CDC_TYPE_CALL_MANAGEMENT,
        0,
        1
    },
    {
        sizeof(struct usb_cdc_acm_descriptor),
        CS_INTERFACE,
        USB_CDC_TYPE_ACM,
        0
    },
    {
        sizeof(struct usb_cdc_union_descriptor),
        CS_INTERFACE,
        USB_CDC_TYPE_UNION,
        0,
        1
    }
};

constexpr auto comm_iface = []{
    std::array<usb_interface_descriptor, 1> r{};
    r[0].bLength = USB_DT_INTERFACE_SIZE;
    r[0].bDescriptorType = USB_DT_INTERFACE;
    r[0].bInterfaceNumber = 0;
    r[0].bAlternateSetting = 0;
    r[0].bNumEndpoints = 1;
    r[0].bInterfaceClass = USB_CLASS_CDC;
    r[0].bInterfaceSubClass = USB_CDC_SUBCLASS_ACM;
    r[0].bInterfaceProtocol = USB_CDC_PROTOCOL_AT;
    r[0].iInterface = 0;
    r[0].endpoint = comm_endp.data();
    r[0].extra = &cdcacm_functional_descriptors;
    r[0].extralen = sizeof(cdcacm_functional_descriptors);
    return r;
}();

constexpr auto data_iface = []{
    std::array<usb_interface_descriptor, 1> r{};

    r[0].bLength = USB_DT_INTERFACE_SIZE;
    r[0].bDescriptorType = USB_DT_INTERFACE;
    r[0].bInterfaceNumber = 1;
    r[0].bAlternateSetting = 0;
    r[0].bNumEndpoints = 2;
    r[0].bInterfaceClass = USB_CLASS_DATA;
    r[0].bInterfaceSubClass = 0;
    r[0].bInterfaceProtocol = 0;
    r[0].iInterface = 0;
    r[0].endpoint = data_endp.data();

    return r;
}();

constexpr auto ifaces = []{
    std::array<usb_interface, 2> r{};

    r[0].num_altsetting = 1;
    r[0].altsetting = comm_iface.data();

    r[1].num_altsetting = 1;
    r[1].altsetting = data_iface.data();

    return r;
}();

constexpr auto config = []{
    usb_config_descriptor r{};

    r.bLength = USB_DT_CONFIGURATION_SIZE;
    r.bDescriptorType = USB_DT_CONFIGURATION;
    r.wTotalLength = 0;
    r.bNumInterfaces = 2;
    r.bConfigurationValue = 1;
    r.iConfiguration = 0;
    r.bmAttributes = 0x80;
    r.bMaxPower = 0x32;
    r.interface = ifaces.data();

    return r;
}();

constexpr const char *usb_strings[] = {
    "Black Sphere Technologies",
    "CDC-ACM Demo",
    "DEMO",
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

static enum usbd_request_return_codes cdcacm_control_request(usbd_device *usbd_dev,
                                                             struct usb_setup_data *req,
                                                             uint8_t **buf,
                                                             uint16_t *len,
                                                             void (**complete)(usbd_device *usbd_dev,
                                                                               struct usb_setup_data *req))
{
    (void)complete;
    (void)buf;
    (void)usbd_dev;

    switch(req->bRequest) {
        case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
            /*
             * This Linux cdc_acm driver requires this to be implemented
             * even though it's optional in the CDC spec, and we don't
             * advertise it in the ACM functional descriptor.
             */
            char local_buf[10];
            auto *notif = (struct usb_cdc_notification *)local_buf;

            /* We echo signals back to host as notification. */
            notif->bmRequestType = 0xA1;
            notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
            notif->wValue = 0;
            notif->wIndex = 0;
            notif->wLength = 2;
            local_buf[8] = req->wValue & 3;
            local_buf[9] = 0;
            // usbd_ep_write_packet(0x83, buf, 10);
            return USBD_REQ_HANDLED;
        }
        case USB_CDC_REQ_SET_LINE_CODING:
            if(*len < sizeof(struct usb_cdc_line_coding)) {
                return USBD_REQ_NOTSUPP;
            }
            return USBD_REQ_HANDLED;
    }
    return USBD_REQ_NOTSUPP;
}

static void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
    (void)ep;
    char buf[64];
    int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);

    if (len) {
        usbd_ep_write_packet(usbd_dev, 0x82, buf, len);
        buf[len] = 0;
    }
}

static void cdcacm_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
    (void)wValue;

    usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
    usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK, 64, NULL);
    usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

    usbd_register_control_callback(usbd_dev,
                                   USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
                                   USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
                                   cdcacm_control_request);
}
usbd_device *usbd_dev;

void usb_task(void*)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    rcc_periph_clock_enable(RCC_AFIO);

    AFIO_MAPR |= AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON;

    usbd_dev = usbd_init(&st_usbfs_v1_usb_driver,
                         &dev,
                         &config,
                         usb_strings,
                         3,
                         usbd_control_buffer,
                         sizeof(usbd_control_buffer));
    usbd_register_set_config_callback(usbd_dev, cdcacm_set_config);

    nvic_enable_irq(NVIC_USB_LP_CAN_RX0_IRQ);
    nvic_enable_irq(NVIC_USB_HP_CAN_TX_IRQ);
    nvic_enable_irq(NVIC_USB_WAKEUP_IRQ);

    tos_enable_interrupts();

    tos::semaphore s{0};
    s.down();
}

extern "C"
{
void usb_wakeup_isr(void) {
    usbd_poll(usbd_dev);
}

void usb_lp_can_rx0_isr(void) {
    usbd_poll(usbd_dev);
}

void usb_hp_can_tx_isr(void)
{
    usbd_poll(usbd_dev);
}
}

constexpr auto RCC_LED1 = RCC_GPIOC;
constexpr auto PORT_LED1 = GPIOC;
constexpr auto PIN_LED1 = GPIO13;

tos::semaphore set{1}, clear{0};

void blink_task(void*)
{
    rcc_periph_clock_enable(RCC_LED1);
    gpio_set_mode(PORT_LED1, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, PIN_LED1);
    gpio_set(PORT_LED1, PIN_LED1);

    while (true)
    {
        set.down();
        gpio_set(PORT_LED1, PIN_LED1);
        for (int i = 0; i < 2'000'000; i++) {
            __asm__("nop");
        }
        clear.up();
    }
}

void off_task(void*)
{
    while (true)
    {
        clear.down();
        gpio_clear(PORT_LED1, PIN_LED1);
        for (int i = 0; i < 2'000'000; i++) {
            __asm__("nop");
        }
        set.up();
    }
}

void tos_main()
{
    //tos::launch(blink_task);
    //tos::launch(off_task);
    tos::launch(usb_task);
}