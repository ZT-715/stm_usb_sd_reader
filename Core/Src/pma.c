/*
 * pma.c
 *
 *  Created on: Jun 22, 2025
 *      Author: Gabriel Zanella
 */

#include "pma.h"

volatile uint16_t* PMA = ((volatile uint16_t*) USB_PMAADDR);


inline void usb_pma_write(usb_pma_offset_t pma_offset, const uint8_t *data, uint16_t length) {
    volatile uint16_t *p = &PMA[pma_offset];

    for (uint16_t i = 0; i < (length + 1) / 2; i++) {
        uint16_t word = data[2 * i];
        if ((2 * i + 1) < length) {
            word |= ((uint16_t)data[2 * i + 1] << 8);
        }
        *p++ = word;
    }
}

inline void usb_pma_read(usb_pma_offset_t pma_offset, uint8_t *data, uint16_t length) {
    volatile uint16_t *p = &PMA[pma_offset];

    for (uint16_t i = 0; i < (length + 1) / 2; i++) {
        uint16_t word = *p++;
        data[2 * i] = word & 0xFF;
        if ((2 * i + 1) < length) {
            data[2 * i + 1] = (word >> 8) & 0xFF;
        }
    }
}

inline void usb_set_tx_count(uint8_t ep_num, uint16_t len) {
    PMA[USB_BTABLE_ENTRY(ep_num) + 1] = len; // COUNT_TX
}

