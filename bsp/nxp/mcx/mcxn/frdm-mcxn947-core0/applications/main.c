/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-24     Magicoe      first version
 * 2020-01-10     Kevin/Karl   Add PS demo
 * 2020-09-21     supperthomas fix the main.c
 *
 */

#include <rtdevice.h>
#include <rtthread.h>
#include "drv_pin.h"
#include "board.h"
#include "fsl_mailbox.h"


#define LEDB_PIN        ((1*32)+2)
#define BUTTON_PIN      ((0*32)+23)

#ifndef BOARD_LED_RED_GPIO
#define BOARD_LED_RED_GPIO GPIO0
#endif
#ifndef BOARD_LED_RED_GPIO_PIN
#define BOARD_LED_RED_GPIO_PIN 10U
#endif

static void sw_pin_cb(void *args);


void MAILBOX_IRQHandler()
{
    rt_kprintf("recive from core1 ack mailbox value %d.\n",MAILBOX_GetValue(MAILBOX,kMAILBOX_CM33_Core0));
    MAILBOX_ClearValueBits(MAILBOX,kMAILBOX_CM33_Core0,0xffffffff);
}

int main(void)
{
#if defined(__CC_ARM)
    rt_kprintf("using armcc, version: %d\n", __ARMCC_VERSION);
#elif defined(__clang__)
    rt_kprintf("using armclang, version: %d\n", __ARMCC_VERSION);
#elif defined(__ICCARM__)
    rt_kprintf("using iccarm, version: %d\n", __VER__);
#elif defined(__GNUC__)
    rt_kprintf("using gcc, version: %d.%d\n", __GNUC__, __GNUC_MINOR__);
#endif

    //rt_pin_mode(LEDB_PIN, PIN_MODE_OUTPUT);  /* Set GPIO as Output */

    rt_pin_mode(BUTTON_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(BUTTON_PIN, PIN_IRQ_MODE_FALLING, sw_pin_cb, RT_NULL);
    rt_pin_irq_enable(BUTTON_PIN, 1);

    rt_kprintf("MCXN947 HelloWorld\r\n");
    
    MAILBOX_Init(MAILBOX);
    NVIC_EnableIRQ(MAILBOX_IRQn);

#ifdef RT_USING_SDIO
    rt_thread_mdelay(2000);
    if (dfs_mount("sd", "/", "elm", 0, NULL) == 0)
    {
        rt_kprintf("sd mounted to /\n");
    }
    else
    {
        rt_kprintf("sd mount to / failed\n");
    }
#endif

    while (1)
    {
        //rt_pin_write(LEDB_PIN, PIN_HIGH);    /* Set GPIO output 1 */
        rt_thread_mdelay(500);               /* Delay 500mS */
        //rt_pin_write(LEDB_PIN, PIN_LOW);     /* Set GPIO output 0 */
        //rt_thread_mdelay(500);               /* Delay 500mS */
        rt_kprintf("send to core1 req.\n");
        MAILBOX_SetValue(MAILBOX,kMAILBOX_CM33_Core1,1);
    }
}

static void sw_pin_cb(void *args)
{
    rt_kprintf("sw pressed\r\n");
}



#include <rtthread.h>
#include "fsl_common.h"

#define CORE1_IMAGE_COPY_TO_RAM

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PRIMARY_CORE_MAILBOX_CPU_ID   kMAILBOX_CM33_Core0
#define SECONDARY_CORE_MAILBOX_CPU_ID kMAILBOX_CM33_Core1

/* Address of RAM, where the image for core1 should be copied */
#define CORE1_BOOT_ADDRESS 0x2004E000

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
extern unsigned char core1_image_start[];
#define CORE1_IMAGE_START core1_image_start
#elif (defined(__GNUC__)) && (!defined(__MCUXPRESSO))
extern const char core1_image_start[];
extern const char *core1_image_end;
extern int core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif

void start_secondary_core(uint32_t sec_core_boot_addr);


#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void)
{
    uint32_t image_size;
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    image_size = (uint32_t)&Image$$CORE1_REGION$$Length;
#elif defined(__ICCARM__)
#pragma section = "__core1_image"
    image_size = (uint32_t)__section_end("__core1_image") - (uint32_t)&core1_image_start;
#elif defined(__GNUC__)
    image_size = (uint32_t)core1_image_size;
#endif
    return image_size;
}
#endif

void start_secondary_core(uint32_t sec_core_boot_addr)
{
    /* Boot source for Core 1 from flash */
    SYSCON->CPBOOT = (sec_core_boot_addr & SYSCON_CPBOOT_CPBOOT_MASK);

    int32_t temp = SYSCON->CPUCTRL;
    temp |= 0xc0c48000;
    SYSCON->CPUCTRL = temp | SYSCON_CPUCTRL_CPU1RSTEN_MASK | SYSCON_CPUCTRL_CPU1CLKEN_MASK;
    SYSCON->CPUCTRL = (temp | SYSCON_CPUCTRL_CPU1CLKEN_MASK) & (~SYSCON_CPUCTRL_CPU1RSTEN_MASK);
}

static int kick_core1(void)
{
    CLOCK_EnableClock(kCLOCK_Gpio0);
  
    GPIO_EnablePinControlNonSecure(BOARD_LED_RED_GPIO, (1 << BOARD_LED_RED_GPIO_PIN));
  
#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* Calculate size of the image */
    uint32_t core1_image_size;
    core1_image_size = get_core1_image_size();
    rt_kprintf("Copy CORE1 image to address: 0x%x, size: %d\r\n", CORE1_BOOT_ADDRESS, core1_image_size);

    /* Copy application from FLASH to RAM */
    memcpy((void *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#endif

    /* Start the secondary core */
    start_secondary_core(CORE1_BOOT_ADDRESS);
    
    return 0;
}

INIT_ENV_EXPORT(kick_core1);

// end file
