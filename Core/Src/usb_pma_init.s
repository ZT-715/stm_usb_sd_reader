.syntax unified
.thumb

.global usb_pma_init
.type usb_pma_init, %function

usb_pma_init:
    ldr r0, =0x40006000     // PMA base address

    // EP0 TX
    movs r1, #0x80          // 0x0080
    strh r1, [r0, #0]       // BTABLE + 0

    // EP0 RX
    movs r1, #0x40
    strh r1, [r0, #4]       // BTABLE + 4


    // EP1 TX
    movs r1, #0xC0
    strh r1, [r0, #8]       // BTABLE + 8


    // EP2 RX
    movw r1, #0x0100
    strh r1, [r0, #12]      // BTABLE + 12


    bx lr
