/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "mcmgr.h"

#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_mailbox.h"
#include "virtual_uart.h"
#include <stdio.h>
#include "littleshell.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LED_INIT()   LED_RED_INIT(LOGIC_LED_ON)
#define LED_TOGGLE() LED_RED_TOGGLE()
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* Initialize MCMGR - low level multicore management library. Call this
       function as close to the reset entry as possible to allow CoreUp event
       triggering. The SystemInitHook() weak function overloading is used in this
       application. */
}


struct virual_uart * p_virtual_uart0 = NULL;

void MAILBOX_IRQHandler()
{
    if(NULL == p_virtual_uart0)
    {
          p_virtual_uart0 = (struct virual_uart *)MAILBOX_GetValue(MAILBOX,kMAILBOX_CM33_Core1);
    }

    MAILBOX_ClearValueBits(MAILBOX,kMAILBOX_CM33_Core1,0xffffffff);
    MAILBOX_SetValue(MAILBOX,kMAILBOX_CM33_Core0,100);
}

size_t __write(int handle, const unsigned char *buffer, size_t size)
{

     while(MAILBOX_GetMutex(MAILBOX) == 0);
      
     RingBuffer_Write(&p_virtual_uart0->core0_rx_core1_tx,(uint8_t *)buffer,size);       

     MAILBOX_SetMutex(MAILBOX);
     
     return size;
}


uint8_t uartgetchar(uint8_t* pdata)
{
     uint8_t ret  = 0;
     while(MAILBOX_GetMutex(MAILBOX) == 0);
      
     ret = RingBuffer_Read(&p_virtual_uart0->core0_tx_core1_rx,pdata,1);       

     MAILBOX_SetMutex(MAILBOX);  
     
     return ret;
}
/*!
 * @brief Main function
 */
int main(void)
{
    /* enable clock for GPIO */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    BOARD_InitBootPins();
    
    MAILBOX_Init(MAILBOX);
    NVIC_EnableIRQ(MAILBOX_IRQn);
    
    while(p_virtual_uart0 == NULL);
    
    if(p_virtual_uart0->magic[0] == 'V' && 
       p_virtual_uart0->magic[1] == 'U' && 
       p_virtual_uart0->magic[2] == 'R' && 
       p_virtual_uart0->magic[3] == 'T')
    {
        printf("\r\n [core1] virtual uart init ok.\r\n");
    }

    /* Configure LED */
    LED_INIT();
    
    littleshell_main_entry(NULL);
#if 0
    for (;;)
    {
        SDK_DelayAtLeastUs(500000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
        LED_TOGGLE();
        printf("\r\n [core1] led toggle.\r\n");
    }
#endif    

}

unsigned int hello(char argc,char ** argv)
{
    printf("hello i am core1 \r\n");
    return 0;
}

LTSH_FUNCTION_EXPORT(hello, "core1 hello");