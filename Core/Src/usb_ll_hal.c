////
//// Created by Gabriel Zanella on 21/06/25.
////
//#include "usb_ll_hal.h"
//
//#include "stm32f1xx.h"
//#include "stm32f1xx_hal.h"
//
//#include <stdbool.h>
//#include <string.h>
//
//#include "usbd_def.h"
//#include "usbd_core.h"
//#include "stm32f1xx_ll_usb.h"
//#include "stm32f1xx_ll_rcc.h"
//#include "stm32f1xx_ll_system.h"
//#include "stm32f1xx_hal_def.h"
//
//#define EP_TYPE_CTRL                           0U
//#define EP_TYPE_ISOC                           1U
//#define EP_TYPE_BULK                           2U
//#define EP_TYPE_INTR                           3U
//#define EP_TYPE_MSK                            3U
//
//// MSC RAM TEST ALLOCATION
//uint8_t msc_storage[MSC_NUM_BLOCKS][MSC_BLOCK_SIZE]; // RAM disk
//
//uint8_t EP0_Rx_Buffer[64], EP0_Tx_Buffer[64];
//uint8_t EP1_Tx_Buffer[64], EP2_Rx_Buffer[64];
//
//static volatile uint8_t new_address = 0;
//static volatile bool pending_address = 0;
//static volatile uint8_t config_value = 0;
//
//static uint8_t write_buffer[MSC_BLOCK_SIZE];  		   // Blocos são 512, porém USB é 64 bytes
//static volatile uint16_t write_buffer_offset = 0;      // Offset atual do buffer
//static volatile uint32_t write_lba = 0;                // Endereço lógico do bloco
//static volatile uint16_t write_blocks_remaining = 0;   // Quantos blocos faltam
//static volatile uint32_t cbw_tag = 0;
//
//static uint8_t read_buffer[MSC_BLOCK_SIZE];
//static volatile uint16_t read_buffer_offset = 0;
//static volatile uint32_t read_lba = 0;
//static volatile uint16_t read_blocks_remaining = 0;
//static volatile uint32_t read_cbw_tag = 0;
//
//typedef struct {
//    uint32_t tag;
//    uint32_t residue;
//    uint8_t status;
//    bool pending;
//} msc_csw_t;
//
//volatile msc_csw_t pending_csw;
//
//static const inquiry_response_t inquiry_response = {
//    .peripheral = 0x00,          // Direct-access device
//    .rmb = 0x80,                 // Removable
//    .version = 0x00,
//    .response_data_format = 0x01,
//    .additional_length = 31,
//    .reserved = {0, 0, 0},
//    .vendor_id = "STM32   ",
//    .product_id = "USB Flash Drive ",
//    .product_rev = "1.00"
//};
//
//
//// Configuration Descriptor Tree (total 32 bytes)
//const uint8_t config_descriptor[] = {
//    // === Configuration Descriptor ===
//    0x09,       // bLength
//    0x02,       // bDescriptorType = CONFIGURATION
//    32, 0x00,   // wTotalLength = 32 bytes
//    0x01,       // bNumInterfaces = 1
//    0x01,       // bConfigurationValue
//    0x00,       // iConfiguration
//    0x80,       // bmAttributes = Bus powered
//    0x32,       // bMaxPower = 100mA
//
//    // === Interface Descriptor ===
//    0x09,       // bLength
//    0x04,       // bDescriptorType = INTERFACE
//    0x00,       // bInterfaceNumber = 0
//    0x00,       // bAlternateSetting
//    0x02,       // bNumEndpoints = 2
//    0x08,       // bInterfaceClass = Mass Storage
//    0x06,       // bInterfaceSubClass = SCSI
//    0x50,       // bInterfaceProtocol = Bulk-Only Transport
//    0x00,       // iInterface
//
//    // === Endpoint Descriptor (Bulk IN) ===
//    0x07,       // bLength
//    0x05,       // bDescriptorType = ENDPOINT
//    0x81,       // bEndpointAddress = IN endpoint 1
//    0x02,       // bmAttributes = BULK
//    0x40, 0x00, // wMaxPacketSize = 64
//    0x00,       // bInterval = N/A for BULK
//
//    // === Endpoint Descriptor (Bulk OUT) ===
//    0x07,       // bLength
//    0x05,       // bDescriptorType = ENDPOINT
//    0x01,       // bEndpointAddress = OUT endpoint 1
//    0x02,       // bmAttributes = BULK
//    0x40, 0x00, // wMaxPacketSize = 64
//    0x00        // bInterval
//};
//
//// Device Descriptor (mínimo)
//const uint8_t device_descriptor[] = {
//    0x12,       // bLength
//    0x01,       // bDescriptorType (Device)
//    0x00, 0x02, // bcdUSB 2.00
//    0x00,       // bDeviceClass (Defined in Interface)
//    0x00,       // bDeviceSubClass
//    0x00,       // bDeviceProtocol
//    0x40,   	// bMaxPacketSize
//    0x83, 0x04, // idVendor (0x0483 - STMicro)
//    0x78, 0x56, // idProduct
//    0x00, 0x01, // bcdDevice
//    0x01,       // iManufacturer
//    0x02,       // iProduct
//    0x03,       // iSerialNumber
//    0x01        // bNumConfigurations
//};
//
//
//// MSC functions
//
//void msc_init(void);
//
//void msc_handle_write(const MSC_CmdBlockWrapper_t *cbw);
//
//void msc_handle_start_stop_unit(const MSC_CmdBlockWrapper_t *cbw);
//
//void msc_handle_read(const MSC_CmdBlockWrapper_t *cbw);
//
//void msc_handle_mode_sense6(const MSC_CmdBlockWrapper_t *cbw);
//
//void msc_handle_request_sense(const MSC_CmdBlockWrapper_t *cbw);
//
//void msc_handle_read_capacity(const MSC_CmdBlockWrapper_t *cbw);
//
//void msc_handle_test_unit_ready(const MSC_CmdBlockWrapper_t *cbw);
//
//void msc_handle_inquiry(const MSC_CmdBlockWrapper_t *cbw);
//
//// End MSC functions
//
//// USB functions
//
//void usb_handle_ctr(void);
//
//void usb_init(void);
//
//// End USB functions
//void USB_LL_InitEndpoints(USB_TypeDef *USBx)
//{
//    // Flush all FIFOs
//    USB_FlushRxFifo(USBx);
//    USB_FlushTxFifo(USBx, 0x10); // Flush all TX FIFOs
//
//    // EP0 OUT (Control RX)
//    USB_EPTypeDef EP0_OUT = {
//        .num = 0,
//        .is_in = 0,
//        .type = EP_TYPE_CTRL,
//        .maxpacket = 64,
//        .xfer_buff = EP0_Rx_Buffer,
//        .xfer_len = 64
//    };
//
//    // EP0 IN (Control TX)
//    USB_EPTypeDef EP0_IN = {
//        .num = 0,
//        .is_in = 1,
//        .type = EP_TYPE_CTRL,
//        .maxpacket = 64,
//        .xfer_buff = EP0_Tx_Buffer,
//        .xfer_len = 64
//    };
//
//    // EP1 IN (Bulk TX)
//    USB_EPTypeDef EP1_IN = {
//        .num = 1,
//        .is_in = 1,
//        .type = EP_TYPE_BULK,
//        .maxpacket = 64,
//        .xfer_buff = EP1_Tx_Buffer,
//        .xfer_len = 64
//    };
//
//    // EP2 OUT (Bulk RX)
//    USB_EPTypeDef EP2_OUT = {
//        .num = 2,
//        .is_in = 0,
//        .type = EP_TYPE_INTR,
//        .maxpacket = 64,
//        .xfer_buff = EP2_Rx_Buffer,
//        .xfer_len = 64
//    };
//
//    USB_ActivateEndpoint(USB, &EP0_OUT);
//    USB_ActivateEndpoint(USB, &EP0_IN);
//    USB_ActivateEndpoint(USB, &EP1_IN);
//    USB_ActivateEndpoint(USB, &EP2_OUT);
//}
//
//void usb_init(void) {
//
//	// Reset flags
//	new_address = 0;
//	pending_address = 0;
//	config_value = 0;
//	msc_init();
//
////	(void)USB_DevInit(USB, 0x0);
//
//	// Enable USB-peripheral clock
//	RCC->APB1ENR |= RCC_APB1ENR_USBEN; for (volatile uint16_t i = 0; i < 10000; i++) __NOP();
//
//	/* Init Device */
//	/* CNTR_FRES = 1 */
//	USB->CNTR = (uint16_t)USB_CNTR_FRES;
//
//	/* CNTR_FRES = 0 */
//	USB->CNTR = 0U;
//
//	/* Clear pending interrupts */
//	USB->ISTR = 0U;
//
//	/*Set Btable Address*/
//	USB->BTABLE = BTABLE_ADDRESS;
//
//    USB_LL_InitEndpoints(USB);
//
////    HAL_StatusTypeDef USB_EnableGlobalInt(USB_TypeDef *USBx)
//
//    USB->CNTR = USB_CNTR_RESETM | USB_CNTR_CTRM;
//    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
//}
//
//void USB_Send_EP0(const uint8_t *data, uint16_t len) {
//    // Copy data to EP0 TX buffer
//    memcpy(EP0_Tx_Buffer, data, len);
//
//    // Update EP0 IN transfer parameters
//    USB_EPTypeDef EP0_IN = {
//        .num = 0,
//        .is_in = 1,
//        .type = EP_TYPE_CTRL,
//        .maxpacket = 64,
//        .xfer_buff = EP0_Tx_Buffer,
//        .xfer_len = len,
//    };
//
//    // Start transmission
//    USB_EPStartXfer(USB, &EP0_IN);
//}
//
//void msc_init(void) {
//    for (int i = 0; i < MSC_NUM_BLOCKS; i++) {
//        for (int j = 0; j < MSC_BLOCK_SIZE; j++) {
//            msc_storage[i][j] = 0U;
//        }
//    }
//}
//
//void usb_process_cbw(const MSC_CmdBlockWrapper_t *cbw) {
//    switch (cbw->CBWCB[0]) {
//
//		case 0x00: // TEST UNIT READY
//			msc_handle_test_unit_ready(cbw);
//			break;
//
//		case 0x05:
//			msc_handle_request_sense(cbw);
//			break;
//
//		case 0x12: // INQUIRY
//			msc_handle_inquiry(cbw);
//			break;
//
//		case 0x1A:
//			msc_handle_mode_sense6(cbw);
//			break;
//
//        case 0x1B:
//        	msc_handle_start_stop_unit(cbw); //
//        	break;
//
//		case 0x25: // READ CAPACITY (10)
//			msc_handle_read_capacity(cbw);
//			break;
//
//		case 0x28: // READ(10)
//			msc_handle_read(cbw);
//			break;
//
//		case 0x2A: // WRITE(10)
//			msc_handle_write(cbw);
//			break;
//
//		default:
//			usb_send_csw(cbw->dCBWTag, cbw->dCBWDataTransferLength, 0x01); // Failed
//			break;
//    }
//}
//
//void usb_handle_ctr(void) {
//	MSC_CmdBlockWrapper_t cbw;
//
////	HAL_StatusTypeDef USB_EPStartXfer(USB_TypeDef *USBx, USB_EPTypeDef *ep)
////	HAL_StatusTypeDef  USB_SetDevAddress(USB_TypeDef *USBx, uint8_t address)
//
//
//	// Register is cleared on read  (Order matters)
//    uint16_t ep_idx = USB->ISTR & USB_ISTR_EP_ID;
//    switch(ep_idx) {
//    case(0): { // Configuration
//        uint16_t ep0_val = USB->EP0R;
//		if ((ep0_val & USB_EP_CTR_RX)) {
//			if ((ep0_val & USB_EP_SETUP)) {
//				usb_setup_packet_t* setup = EP0_Rx_Buffer;
//
//				if (setup->bRequest == 0x06) { // GET_DESCRIPTOR
//					uint8_t desc_type = setup->wValue >> 8;
//					uint8_t desc_index = (uint8_t)(setup->wValue & 0xFF);
//
//					if (desc_type == 1) {
//						// Device Descriptor
//						USB_Send_EP0((uint8_t*)device_descriptor, sizeof(device_descriptor));
//					}
//					else if (desc_type == 2) {
//						// Configuration Descriptor
//						uint16_t len = setup->wLength > sizeof(config_descriptor) ? sizeof(config_descriptor) : setup->wLength;
//						USB_Send_EP0((uint8_t*)config_descriptor, len);
//					}
//				}
//				else if (setup->bRequest == 0x05) { // SET_ADDRESS
//					new_address = setup->wValue & 0x7F;
//					pending_address = true;
//
//					// ACK com ZPL (zero-length packet)
//					USB_Send_EP0(NULL, 0);
//				}
//				else if (setup->bRequest == 0x09) { // SET_CONFIGURATION
//					config_value = setup->wValue & 0xFF;
//
//					// ACK com ZLP
//					USB_Send_EP0(NULL, 0);
//				}
//				else if (setup->bRequest == 0x08) { // GET_CONFIGURATION
//					USB_Send_EP0((uint8_t*)&config_value, sizeof(config_value));
//				}
//			}
//		}
//		else if ((ep0_val & USB_EP_CTR_TX) && pending_address) {
//			USB->DADDR = USB_DADDR_EF | new_address;
//			pending_address = false;
//		}
//
//		USB->EP0R = USB->EP0R & (~USB_EP_CTR_RX & USB_EPREG_MASK);
//		break;
//    }
////    case(1): { // IN -> Dispositivo para host
////        uint16_t ep1_val = USB->EP1R;
////
////		if (ep1_val & USB_EP_CTR_TX) {
////			if (pending_csw.pending) {
////				usb_send_csw(pending_csw.tag, pending_csw.residue, pending_csw.status);
////				pending_csw.pending = false;
////			}
////			else if (read_blocks_remaining > 0) {
////				if (read_buffer_offset < MSC_BLOCK_SIZE) {
////					// Envia próximo pacote de 64 bytes
////					usb_ep1_send(&read_buffer[read_buffer_offset], BULK_PACKET_SIZE);
////					read_buffer_offset += BULK_PACKET_SIZE;
////				}
////				else {
////					// Carregar próximo bloco
////					read_lba++;
////					read_blocks_remaining--;
////
////					if (read_blocks_remaining > 0) {
////						memcpy(read_buffer, msc_storage[read_lba], MSC_BLOCK_SIZE);
////						read_buffer_offset = 0;
////						usb_ep1_send(&read_buffer[read_buffer_offset], BULK_PACKET_SIZE);
////						read_buffer_offset += BULK_PACKET_SIZE;
////					}
////					else {
////						// Tudo enviado, envia CSW
////						usb_send_csw(read_cbw_tag, 0, 0x00); // Success
////					}
////				}
////			}
////		}
////        USB->EP1R = (USB->EP1R & USB_EPREG_MASK) & ~USB_EP_CTR_TX;
////		break;
////    }
////    case(2): { // OUT -> Host para dispositivo
////        uint16_t ep2_val = USB->EP2R;
////
////		if (ep2_val & USB_EP_CTR_RX) {
////			if (write_blocks_remaining > 0) {
////		        uint8_t temp_buf[BULK_PACKET_SIZE];
////		        usb_pma_read(USB_PMA_EP2_OUT, temp_buf, BULK_PACKET_SIZE);
////
////		        // Copia para o buffer de escrita
////		        memcpy(&write_buffer[write_buffer_offset], temp_buf, BULK_PACKET_SIZE);
////		        write_buffer_offset += BULK_PACKET_SIZE;
////
////		        // Quando preencher 512 bytes:
////		        if (write_buffer_offset >= MSC_BLOCK_SIZE) {
////		            // Escreve na memória de armazenamento
////		            memcpy(msc_storage[write_lba], write_buffer, MSC_BLOCK_SIZE);
////
////		            write_lba++;
////		            write_blocks_remaining--;
////		            write_buffer_offset = 0; // Zera para acumular o próximo bloco
////
////		            if (write_blocks_remaining == 0) {
////		                usb_send_csw(cbw_tag, 0, 0x00); // Success
////		            }
////		        }
////			}
////			else {
////				usb_pma_read(USB_PMA_EP2_OUT, (uint8_t*)&cbw, sizeof(cbw));
////
////				if (cbw.dCBWSignature == CBW_SIGNATURE) {
////					usb_process_cbw(&cbw);
////				}
////			}
////			USB->EP2R = (USB->EP2R & USB_EPREG_MASK) | USB_EP_RX_VALID;
////		}
////		break;
////    }
//    default:
//    	break;
//    }
//}
//
//
////void USB_LP_CAN1_RX0_IRQHandler(void) {
////    if (USB->ISTR & USB_ISTR_RESET) {
////        USB->ISTR &= ~USB_ISTR_RESET;
////        USB->DADDR = USB_DADDR_EF;
////        usb_init();
////    }
////    else if (USB->ISTR & USB_ISTR_CTR) {
////        usb_handle_ctr();
////
////    }
////}
//
