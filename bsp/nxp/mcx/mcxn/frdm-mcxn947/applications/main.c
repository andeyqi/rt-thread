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

#define LEDB_PIN        ((1*32)+2)
#define BUTTON_PIN      ((0*32)+23)

static void sw_pin_cb(void *args);

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

    rt_pin_mode(LEDB_PIN, PIN_MODE_OUTPUT);  /* Set GPIO as Output */

    rt_pin_mode(BUTTON_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(BUTTON_PIN, PIN_IRQ_MODE_FALLING, sw_pin_cb, RT_NULL);
    rt_pin_irq_enable(BUTTON_PIN, 1);

    rt_kprintf("MCXN947 HelloWorld\r\n");


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
        rt_pin_write(LEDB_PIN, PIN_HIGH);    /* Set GPIO output 1 */
        rt_thread_mdelay(500);               /* Delay 500mS */
        rt_pin_write(LEDB_PIN, PIN_LOW);     /* Set GPIO output 0 */
        rt_thread_mdelay(500);               /* Delay 500mS */
    }
}

static void sw_pin_cb(void *args)
{
    rt_kprintf("sw pressed\r\n");
}


#if defined(__GNUC__)

static __attribute__((noinline)) int my_memcpy(void * src,void *  dst,int len)
{
    unsigned int tmp = 0;
    unsigned int end = (char *)src + len;

    asm volatile (
            "1: ldr %[tmp], [%[src]] , #4\n"
            "str %[tmp], [%[dst]], #4\n"
            "cmp %[src], %[end] \n"
            "bcc 1b"
            : [dst] "+r" (dst), [tmp] "+r" (tmp), [src] "+r" (src)
            : [end] "r" (end)
            : "memory");

    return len;
}


static int cmd_mymemcpy_test(int argc, char **argv)
{

    int src[8] = {0x11111111,0x22222222,0x33333333,0x44444444,
                  0x55555555,0x66666666,0x77777777,0x88888888};
    int dst[8] = {};

    rt_kprintf("my memcpy test ret = %d\r\n",my_memcpy(src,dst,sizeof(src)));

    if(memcmp(src,dst,sizeof(src)) == 0)
        rt_kprintf("my memcpy test ok.\r\n");

    int src1[32] = {0x11111111,0x22222222,0x33333333,0x44444444,
                  0x55555555,0x66666666,0x77777777,0x88888888};
    int dst1[32] = {};

    rt_kprintf("my memcpy test ret = %d\r\n",my_memcpy(src1,dst1,sizeof(src1)));

    if(memcmp(src1,dst1,sizeof(src1)) == 0)
        rt_kprintf("my memcpy test ok.\r\n");

    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_mymemcpy_test, mymemcpy, my memcpy test);
#endif

// end file
