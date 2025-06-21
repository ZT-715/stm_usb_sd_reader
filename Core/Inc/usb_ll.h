//
// Created by Gabriel Zanella on 21/06/25.
//

#ifndef USB_LL_H
#define USB_LL_H
#include <stdint.h>


/**
 * Setup package from host
 *
 *
 */
typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} usb_setup_packet_t;





void usb_init(void);
void usb_poll(void);

#endif //USB_LL_H
