/*
 * pma.h
 *
 *  Created on: Jun 22, 2025
 *      Author: wymac
 */

#ifndef INC_PMA_H_
#define INC_PMA_H_

#include <stdint.h>
#include "stm32f1xx.h"

//#define USB_PMAADDR  ((uint32_t)0x40006000) // defined in stm32f1xx.h
#define USB_BTABLE_BASE 		0x00

// EP0 OUT (RX)
#define EP0_RX_ADDR             0x40
#define EP0_RX_PMA_OFFSET       (EP0_RX_ADDR >> 1)  // 0x20 (words)

// EP0 IN (TX)
#define EP0_TX_ADDR             0x80
#define EP0_TX_PMA_OFFSET       (EP0_TX_ADDR >> 1)  // 0x40 (words)

// EP1 IN (BULK TX)
#define EP1_IN_ADDR             0xC0
#define EP1_IN_PMA_OFFSET       (EP1_IN_ADDR >> 1)  // 0x60 (words)

// EP2 OUT (BULK RX)
#define EP2_OUT_ADDR            0x100
#define EP2_OUT_PMA_OFFSET      (EP2_OUT_ADDR >> 1)  // 0x80 (words)

#define USB_BTABLE_ENTRY(n)   (USB_BTABLE_BASE + 8 * (n))

typedef enum {
    USB_PMA_EP0_RX = EP0_RX_PMA_OFFSET,
    USB_PMA_EP0_TX = EP0_TX_PMA_OFFSET,
    USB_PMA_EP1_IN = EP1_IN_PMA_OFFSET,
    USB_PMA_EP2_OUT = EP2_OUT_PMA_OFFSET,
} usb_pma_offset_t;

typedef struct {
    uint16_t tx_addr;
    uint16_t tx_count;
    uint16_t rx_addr;
    uint16_t rx_count;
} __attribute__((packed)) usb_btable_entry_t;

void usb_pma_read(usb_pma_offset_t pma_offset, uint8_t *data, uint16_t length);
void usb_pma_write(usb_pma_offset_t pma_offset, const uint8_t *data, uint16_t length);
void usb_set_tx_count(uint8_t ep_num, uint16_t len);


#endif /* INC_PMA_H_ */
