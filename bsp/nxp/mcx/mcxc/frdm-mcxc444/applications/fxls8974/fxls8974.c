#include <rtdevice.h>
#include <rtthread.h>
#include "fxls8974.h"

void sensor_task(void * para)
{
    struct rt_i2c_bus_device *i2c_bus;
    struct rt_i2c_msg msgs[2];
    rt_uint8_t reg;
    rt_uint8_t buffer[6];

    i2c_bus = rt_i2c_bus_device_find("i2c0");
    if(i2c_bus == RT_NULL)
    {
        rt_kprintf("i2c0 not find\n");
        return;
    }

    /* Init get who am i */
    reg = FXLS8974_WHO_AM_I;
    msgs[0].addr = FXLS8974_DEVICE_ADDRESS_SA0_0;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = &reg;
    msgs[0].len = 1;

    msgs[1].addr = FXLS8974_DEVICE_ADDRESS_SA0_0;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf = buffer;
    msgs[1].len = 1;


    if (rt_i2c_transfer(i2c_bus, msgs, 2) != 2)
    {
        rt_kprintf("i2c0 read who am i  failed\n");
        return;
    }

    if(buffer[0] == FXLS8974_WHOAMI_VALUE)
    {
        rt_kprintf("i2c0 read who am i  value(%x) ok\n",buffer[0]);
    }
    else
    {
        rt_kprintf("i2c0 read who am i  value(%x) ng\n",buffer[0]);
        return;
    }

    /*! Put the device into standby mode so that configuration can be applied.*/
    buffer[0] = FXLS8974_SENS_CONFIG1;
    buffer[1] = FXLS8974_SENS_CONFIG1_ACTIVE_STANDBY;
    msgs[0].addr = FXLS8974_DEVICE_ADDRESS_SA0_0;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = buffer;
    msgs[0].len = 2;
    rt_i2c_transfer(i2c_bus, &msgs[0], 1);

    buffer[0] = FXLS8974_SENS_CONFIG3;
    buffer[1] = FXLS8974_SENS_CONFIG3_WAKE_ODR_6_25HZ;
    /* Sensor config */
    msgs[0].addr = FXLS8974_DEVICE_ADDRESS_SA0_0;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = buffer;
    msgs[0].len = 2;
    rt_i2c_transfer(i2c_bus, &msgs[0], 1);

    /*! Put the device into active mode and ready for reading data.*/
    buffer[0] = FXLS8974_SENS_CONFIG1;
    buffer[1] = FXLS8974_SENS_CONFIG1_ACTIVE_ACTIVE;
    msgs[0].addr = FXLS8974_DEVICE_ADDRESS_SA0_0;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = buffer;
    msgs[0].len = 2;
    rt_i2c_transfer(i2c_bus, &msgs[0], 1);

    while(1)
    {
        reg = FXLS8974_INT_STATUS;
        msgs[0].addr = FXLS8974_DEVICE_ADDRESS_SA0_0;
        msgs[0].flags = RT_I2C_WR;
        msgs[0].buf = &reg;
        msgs[0].len = 1;

        msgs[1].addr = FXLS8974_DEVICE_ADDRESS_SA0_0;
        msgs[1].flags = RT_I2C_RD;
        msgs[1].buf = buffer;
        msgs[1].len = 1;

        rt_i2c_transfer(i2c_bus, msgs, 2);

        if(0 == (buffer[0] & FXLS8974_INT_STATUS_SRC_DRDY_MASK))
        {
            rt_thread_mdelay(2);
            continue;
        }

        reg = FXLS8974_OUT_X_LSB;
        msgs[0].addr = FXLS8974_DEVICE_ADDRESS_SA0_0;
        msgs[0].flags = RT_I2C_WR;
        msgs[0].buf = &reg;
        msgs[0].len = 1;

        msgs[1].addr = FXLS8974_DEVICE_ADDRESS_SA0_0;
        msgs[1].flags = RT_I2C_RD;
        msgs[1].buf = buffer;
        msgs[1].len = 6;

        rt_i2c_transfer(i2c_bus, msgs, 2);

        rt_kprintf("X = %5d Y = %5d Z = %5d \n",((int16_t)buffer[1] << 8) | buffer[0],((int16_t)buffer[3] << 8) | buffer[2],((int16_t)buffer[5] << 8) | buffer[4]);

    }
}


void fxl8974(char argc,char ** argv)
{
    rt_thread_t tid;
    tid = rt_thread_create("fxl8974", sensor_task, RT_NULL,
                           RT_MAIN_THREAD_STACK_SIZE, RT_MAIN_THREAD_PRIORITY, 20);
    RT_ASSERT(tid != RT_NULL);
    rt_thread_startup(tid);
}
MSH_CMD_EXPORT(fxl8974, test fxl8974);
