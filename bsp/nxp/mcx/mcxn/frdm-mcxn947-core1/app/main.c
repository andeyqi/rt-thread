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
    //(void)MCMGR_EarlyInit();
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

#if 0
static int32_t counter @ 0x2004C000 = 0;

unsigned int mutex_test(void)
{
    int i =0x7ffffff;
    
    for(;i != 0;i--)
    {
      while(MAILBOX_GetMutex(MAILBOX) == 0)
        ;
      
      counter--;
      
      MAILBOX_SetMutex(MAILBOX);
      
    }

    return 1;
}
#endif

size_t __write(int handle, const unsigned char *buffer, size_t size)
{

     while(MAILBOX_GetMutex(MAILBOX) == 0);
      

     RingBuffer_Write(&p_virtual_uart0->core0_rx_core1_tx,(uint8_t *)buffer,size);       

     MAILBOX_SetMutex(MAILBOX);
     
     return size;
}

/*!
 * @brief Main function
 */
int main(void)
{
    //uint32_t startupData, i;
    //mcmgr_status_t status;

    /* Init board hardware.*/
    /* enable clock for GPIO */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    BOARD_InitBootPins();
    
    MAILBOX_Init(MAILBOX);
    NVIC_EnableIRQ(MAILBOX_IRQn);

    /* Initialize MCMGR, install generic event handlers */
    //(void)MCMGR_Init();

    /* Get the startup data */
    //do
    //{
    //    status = MCMGR_GetStartupData(&startupData);
    //} while (status != kStatus_MCMGR_Success);

    /* Make a noticable delay after the reset */
    /* Use startup parameter from the master core... */
    //for (i = 0; i < startupData; i++)
    //{
    //    SDK_DelayAtLeastUs(1000000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    //}
    
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
    

    for (;;)
    {
        SDK_DelayAtLeastUs(500000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
        LED_TOGGLE();
        printf("\r\n [core1] led toggle.\r\n");
    }

}