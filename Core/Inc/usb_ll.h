//
// Created by Gabriel Zanella on 21/06/25.
//

#ifndef USB_LL_H
#define USB_LL_H
#include <stdint.h>

#define BULK_PACKET_SIZE 64
#define MSC_NUM_BLOCKS 16
#define MSC_BLOCK_SIZE 512

#define CBW_SIGNATURE 			  0x43425355U
#define CSW_SIGNATURE             0x53425355U


/**
 * Setup package from host
 */
typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} usb_setup_packet_t;

// Command Block Wrapper structure
typedef struct {
    uint32_t dCBWSignature;
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t  bmCBWFlags;
    uint8_t  bCBWLUN;
    uint8_t  bCBWCBLength;
    uint8_t  CBWCB[16];
} __attribute__((packed)) MSC_CmdBlockWrapper_t;

typedef struct {
    uint8_t peripheral;         // 0x00 (Direct Access Block Device)
    uint8_t rmb;                // 0x80 = removível
    uint8_t version;            // 0x00 (sem padrão SCSI)
    uint8_t response_data_format; // 0x01
    uint8_t additional_length;  // 31 (nº de bytes após este campo)
    uint8_t reserved[3];        // zeros
    char vendor_id[8];          // "STM32   "
    char product_id[16];        // "USB Flash Drive "
    char product_rev[4];        // "1.00"
} __attribute__((packed)) inquiry_response_t;


typedef struct {
    uint32_t dCSWSignature; // 0x53425355 ("USBS")
    uint32_t dCSWTag;       // Igual ao CBW
    uint32_t dCSWDataResidue;
    uint8_t  bCSWStatus;    // 0x00 = Passed
} __attribute__((packed)) MSC_CmdStatusWrapper_t;

typedef struct {
    uint32_t last_lba;    // Último bloco válido
    uint32_t block_size;  // Tamanho do bloco em bytes
} __attribute__((packed)) read_capacity_response_t;

typedef struct {
    uint8_t  response_code;        // 0x70 = current error
    uint8_t  obsolete;
    uint8_t  sense_key;            // Ex: 0x00 = no error
    uint8_t  information[4];
    uint8_t  additional_length;    // = número de bytes restantes (normalmente 10)
    uint8_t  cmd_specific_info[4];
    uint8_t  asc;                  // Additional Sense Code
    uint8_t  ascq;                 // Additional Sense Code Qualifier
    uint8_t  fru;
    uint8_t  sense_key_specific[3];
} __attribute__((packed)) sense_response_t;

typedef struct {
    uint8_t mode_data_length;   // Número de bytes restantes (excluindo este)
    uint8_t medium_type;        // Geralmente 0x00
    uint8_t device_specific;    // bit 7 = Write protect
    uint8_t block_descriptor_length; // 0x00 se não houver descritor
    // ... pode haver mais dados
} __attribute__((packed)) mode_sense6_response_t;


void usb_init(void);

#endif //USB_LL_H
