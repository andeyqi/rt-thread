#ifndef __VIRTUAL_UART_H__
#define __VIRTUAL_UART_H__

#include <stdint.h>
#include "ringbuffer.h"

#define VUART_CORE0_RX_CORE1_TX_BUFFER_SIZE   256
#define VUART_CORE0_TX_CORE1_RX_BUFFER_SIZE   128

//#pragma pack(1)
struct virual_uart
{
    char magic[4];/* VURT */
    RingBuffer core0_rx_core1_tx;
    RingBuffer core0_tx_core1_rx;
    uint8_t core0_rx_core1_tx_buff[VUART_CORE0_RX_CORE1_TX_BUFFER_SIZE];
    uint8_t core0_tx_core1_rx_buff[VUART_CORE0_TX_CORE1_RX_BUFFER_SIZE]; 
};
//#pragma pack()




#endif /* end of  __VIRTUAL_UART_H__ */