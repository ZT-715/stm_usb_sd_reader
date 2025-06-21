//
// Created by Gabriel Zanella on 21/06/25.
//

#include "stm32f1xx.h"
#include "usb_ll.h"
#include <stdbool.h>

#define USB_BASE_ADDR 0x40005C00
#define PMA_ADDR ((uint16_t *)0x40006000)

#define EP0_SIZE 64

static uint8_t new_address = 0;
static bool pending_address = 0;
static uint8_t config_value = 0;

// Configuration Descriptor Tree (total 32 bytes)
const uint8_t config_descriptor[] = {
    // === Configuration Descriptor ===
    0x09,       // bLength
    0x02,       // bDescriptorType = CONFIGURATION
    32, 0x00,   // wTotalLength = 32 bytes
    0x01,       // bNumInterfaces = 1
    0x01,       // bConfigurationValue
    0x00,       // iConfiguration
    0x80,       // bmAttributes = Bus powered
    0x32,       // bMaxPower = 100mA

    // === Interface Descriptor ===
    0x09,       // bLength
    0x04,       // bDescriptorType = INTERFACE
    0x00,       // bInterfaceNumber = 0
    0x00,       // bAlternateSetting
    0x02,       // bNumEndpoints = 2
    0x08,       // bInterfaceClass = Mass Storage
    0x06,       // bInterfaceSubClass = SCSI
    0x50,       // bInterfaceProtocol = Bulk-Only Transport
    0x00,       // iInterface

    // === Endpoint Descriptor (Bulk IN) ===
    0x07,       // bLength
    0x05,       // bDescriptorType = ENDPOINT
    0x81,       // bEndpointAddress = IN endpoint 1
    0x02,       // bmAttributes = BULK
    0x40, 0x00, // wMaxPacketSize = 64
    0x00,       // bInterval = N/A for BULK

    // === Endpoint Descriptor (Bulk OUT) ===
    0x07,       // bLength
    0x05,       // bDescriptorType = ENDPOINT
    0x01,       // bEndpointAddress = OUT endpoint 1
    0x02,       // bmAttributes = BULK
    0x40, 0x00, // wMaxPacketSize = 64
    0x00        // bInterval
};

// Device Descriptor (mínimo)
const uint8_t device_descriptor[] = {
    0x12,       // bLength
    0x01,       // bDescriptorType (Device)
    0x00, 0x02, // bcdUSB 2.00
    0x00,       // bDeviceClass (Defined in Interface)
    0x00,       // bDeviceSubClass
    0x00,       // bDeviceProtocol
    EP0_SIZE,   // bMaxPacketSize
    0x83, 0x04, // idVendor (0x0483 - STMicro)
    0x40, 0x57, // idProduct
    0x00, 0x01, // bcdDevice
    0x01,       // iManufacturer
    0x02,       // iProduct
    0x03,       // iSerialNumber
    0x01        // bNumConfigurations
};

static void usb_ep0_setup(void) {
    // PMA: Endereço 0x40 para RX e 0x80 para TX
    USB->BTABLE = 0; // PMA começa no offset 0

    // Endereço do buffer EP0 OUT (recepção)
    PMA_ADDR[0] = 0x0040;      // Endereço buffer
    PMA_ADDR[1] = 0x8000 | EP0_SIZE; // Count RX

    // Endereço do buffer EP0 IN (transmissão)
    PMA_ADDR[2] = 0x0080;      // Endereço buffer
    PMA_ADDR[3] = 0;           // Count TX

    // EP0: tipo controle, estado válido de RX
    USB->EP0R = USB_EP_CONTROL | USB_EP_RX_VALID;
}

void usb_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG; // Libera PA11/12

    // PA11/12 como AF
    GPIOA->CRH &= ~(GPIO_CRH_CNF11 | GPIO_CRH_CNF12);
    GPIOA->CRH |= (GPIO_CRH_CNF11_1 | GPIO_CRH_CNF12_1); // AF push-pull
    GPIOA->CRH |= (GPIO_CRH_MODE11 | GPIO_CRH_MODE12);   // 50 MHz

    USB->CNTR = USB_CNTR_FRES;
    USB->CNTR = 0;
    USB->ISTR = 0;

    usb_ep0_setup();
    USB->CNTR = USB_CNTR_RESETM | USB_CNTR_CTRM;
    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}

void usb_handle_ctr(void) {
    uint16_t ep_val = USB->EP0R;

    if ((ep_val & USB_EP_CTR_RX) == USB_EP_CTR_RX) {
        uint16_t status = ep_val & USB_EP0R_STAT_RX;
        uint16_t type = ep_val & USB_EP_KIND;

        if ((ep_val & USB_EP_SETUP) == USB_EP_SETUP) {
            // Pacote SETUP (tipo de controle)
            usb_setup_packet_t setup;

            uint16_t *pma = &PMA_ADDR[0x40 >> 1]; // EP0 RX buffer
            uint8_t *buf = (uint8_t *)&setup;

            for (uint8_t i = 0; i < 8U; i++) {
                buf[i] = pma[i];
            }


            if (setup.bRequest == 0x06) { // GET_DESCRIPTOR
                uint8_t desc_type = setup.wValue >> 8;
                uint8_t desc_index = setup.wValue & 0xFF;

                if (desc_type == 1) {
                    // Device Descriptor
                    uint16_t *tx_buf = &PMA_ADDR[0x80 >> 1];
                    for (int i = 0; i < sizeof(device_descriptor); i++) {
                        ((uint8_t *)tx_buf)[i] = device_descriptor[i];
                    }

                    PMA_ADDR[3] = sizeof(device_descriptor);
                    USB->EP0R = (USB->EP0R & USB_EPREG_MASK) | USB_EP_TX_VALID;
                }
                else if (desc_type == 2) {
                    // Configuration Descriptor
                    uint16_t *tx_buf = &PMA_ADDR[0x80 >> 1];
                    uint16_t len = sizeof(config_descriptor);
                    if (setup.wLength < len) len = setup.wLength;

                    for (int i = 0; i < len; i++) {
                        ((uint8_t *)tx_buf)[i] = config_descriptor[i];
                    }

                    PMA_ADDR[3] = len;
                    USB->EP0R = (USB->EP0R & USB_EPREG_MASK) | USB_EP_TX_VALID;
                }
            }
            else if (setup.bRequest == 0x05) { // SET_ADDRESS
                new_address = setup.wValue & 0x7F;
                pending_address = true;

                // ACK com ZPL(zero-length packet)
                PMA_ADDR[3] = 0; // TX count zero
                USB->EP0R = (USB->EP0R & USB_EPREG_MASK) | USB_EP_TX_VALID;
            }
            else if (setup.bRequest == 0x09) { // SET_CONFIGURATION
                config_value = setup.wValue & 0xFF;

                // ACK com ZLP
                PMA_ADDR[3] = 0;
                USB->EP0R = (USB->EP0R & USB_EPREG_MASK) | USB_EP_TX_VALID;
            }
            else if (setup.bRequest == 0x08) { // GET_CONFIGURATION
                uint16_t *tx_buf = &PMA_ADDR[0x80 >> 1];
                ((uint8_t *)tx_buf)[0] = config_value;

                PMA_ADDR[3] = 1; // apenas 1 byte
                USB->EP0R = (USB->EP0R & USB_EPREG_MASK) | USB_EP_TX_VALID;
            }

            // Libera o RX
            USB->EP0R = (USB->EP0R & USB_EPREG_MASK) | USB_EP_RX_VALID;
        }

        USB->EP0R &= ~USB_EP_CTR_RX;
    }

    if ((ep_val & USB_EP_CTR_TX) == USB_EP_CTR_TX) {
        USB->EP0R &= ~USB_EP_CTR_TX;

        if (pending_address) {
            USB->DADDR = USB_DADDR_EF | new_address;
            pending_address = false;
        }
    }
}


void USB_LP_CAN1_RX0_IRQHandler(void) {
    if (USB->ISTR & USB_ISTR_CTR) {
        usb_handle_ctr();
        USB->ISTR &= ~USB_ISTR_CTR;
    }
}
