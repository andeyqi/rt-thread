#ifndef __VIRTUAL_UART_H__
#define __VIRTUAL_UART_H__

#include <stdint.h>
#include "ringbuffer.h"

#define VUART_TX_BUFFER_SIZE   256
#define VUART_RX_BUFFER_SIZE   128

//#pragma pack(1)
struct virual_uart
{
    char magic[4];/* VURT */
    RingBuffer rx;
    RingBuffer tx;
    uint8_t rx_buff[VUART_RX_BUFFER_SIZE];
    uint8_t tx_buff[VUART_TX_BUFFER_SIZE]; 
};
//#pragma pack()




#endif /* end of  __VIRTUAL_UART_H__ */