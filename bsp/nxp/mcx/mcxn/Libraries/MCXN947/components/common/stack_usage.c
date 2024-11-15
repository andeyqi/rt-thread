#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "perf_counter.h"
#include "littleshell.h"
#include "debug.h"

#if ((configUSE_STACK_MAX_USAGE == 0 )&& (configUSE_PERF_CNT == 1))
    #error perf is depend on configUSE_STACK_MAX_USAGE = 1
#endif


#if ( configUSE_STACK_MAX_USAGE == 1 )

struct task_stack_info
{
    char task_name[configMAX_TASK_NAME_LEN];
    uint32_t stack_deep;
    uint32_t*  stack_tail;
};

#define OS_NUM_TASKS 3

static struct task_stack_info stack_all[OS_NUM_TASKS+2]; //add timer & idle


void task_create_hook(char * name ,unsigned int deep,unsigned int * stack_tail)
{
    static uint8_t i_loop = 0;
    uint8_t len,i;

    if(i_loop < OS_NUM_TASKS+2)
    {
        strcpy(stack_all[i_loop].task_name,name);
        if((len = strlen(stack_all[i_loop].task_name)) != (configMAX_TASK_NAME_LEN-1))
        {
            for(i = len;i < configMAX_TASK_NAME_LEN-1;i++)
                stack_all[i_loop].task_name[i] = '\0';
        }
        stack_all[i_loop].task_name[configMAX_TASK_NAME_LEN-1] = '\0';
        stack_all[i_loop].stack_deep = deep*4;
        stack_all[i_loop].stack_tail = stack_tail;
        /* if use pref to set perf default value */
#if ( configUSE_PERF_CNT == 1 )
        memset((void*)stack_all[i_loop].stack_tail,0,sizeof(task_cycle_info_t)+8);
#endif
        i_loop++;
    }
    else
    {
        printf("array is overflow \r\n");
    }
}

void get_cur_thread_stack_info(uint32_t sp, uint32_t *start_addr, size_t *size)
{
    uint8_t i_loop = 0;
    extern __no_init volatile char switchintaskname[configMAX_TASK_NAME_LEN];

    for(i_loop = 0;i_loop< OS_NUM_TASKS+2;i_loop++)
    {
        if(strcmp(stack_all[i_loop].task_name,(const char *)switchintaskname) == 0)
            break;
        if((sp > (uint32_t)stack_all[i_loop].stack_tail) && ( sp < ((uint32_t)stack_all[i_loop].stack_tail)+stack_all[i_loop].stack_deep))
            break;
    }
    /* find stack info update */
    if(i_loop != OS_NUM_TASKS+2)
    {
        *start_addr = (uint32_t)stack_all[i_loop].stack_tail;
        *size = stack_all[i_loop].stack_deep;
    }
	else
	{
		 *size = 0;
	}
}

static int buff_continuous_numbers(uint8_t * buff,uint8_t data)
{
    int l = 0;

    if(NULL == buff)
        return 0;

    while(data == buff[l++]);

    return  --l;
}

extern size_t xPortGetFreeHeapSize( void );

unsigned int stack(char argc,char ** argv)
{
    uint8_t i,j;
    int len,name_len;
    char task_name[configMAX_TASK_NAME_LEN];

    if(argc == 1)
    {
        //logctrl_instance()->is_sync = 1;
        printf("taskname\tdeep\tused\taddress\t\t\tusage\r\n");
        for(i = 0;i < OS_NUM_TASKS+2;i++)
        {
#if ( configUSE_PERF_CNT != 1 )
            len = buff_continuous_numbers((uint8_t *)stack_all[i].stack_tail,0xa5);
#else
            len = buff_continuous_numbers((uint8_t *)&stack_all[i].stack_tail[WORD_OF_CYCEL_INFO],0xa5);
            if(len)
                len += BYTE_OF_CYCEL_INFO;
#endif
        strcpy(task_name,stack_all[i].task_name);
        task_name[configMAX_TASK_NAME_LEN - 1] = '\0';
        if((name_len = strlen(stack_all[i].task_name)) != (configMAX_TASK_NAME_LEN-1))
        {
            for(j = name_len;j < configMAX_TASK_NAME_LEN-1;j++)
                task_name[j] = ' ';
        }
            printf("%s\t%d\t%d\t0x%p~0x%p\t%d%%\r\n",task_name,stack_all[i].stack_deep,stack_all[i].stack_deep-len,stack_all[i].stack_tail,stack_all[i].stack_tail+(stack_all[i].stack_deep/4),(100-(len*100)/stack_all[i].stack_deep));
        }
        printf("\r\nHeap Total:%d\tFree:%d\r\n",configTOTAL_HEAP_SIZE,xPortGetFreeHeapSize());
        //logctrl_instance()->is_sync = 0;
    }
    if(argc == 2)
    {
        uint8_t index = atoi(argv[1]);
        if(index >=  OS_NUM_TASKS+2)
            goto out;
        printf("task name [%s] stack deep 0x%x\r\n",stack_all[index].task_name,stack_all[index].stack_deep);
        //logctrl_instance()->is_sync = 1;
        trace_byte_stream((uint8_t *)stack_all[index].stack_tail,stack_all[index].stack_deep,0x00);
        //logctrl_instance()->is_sync = 0;
    }
out:
    return 0;
}

LTSH_FUNCTION_EXPORT(stack,"show task statck info");

#if ( configUSE_PERF_CNT == 1 )

unsigned int perf(char argc,char ** argv)
{
    uint8_t i,j,len;
    int64_t count = get_system_ticks();
    char task_name[configMAX_TASK_NAME_LEN];

    //logctrl_instance()->is_sync = 1;
    if(argc == 1)
    {
        printf("taskname\trecent\t\tactive\tusedmax\t\tusedmin\t\tusedavg\t\tcpuusage\r\n");
        for(i = 0;i < OS_NUM_TASKS+2;i++)
        {
            task_cycle_info_t * pcycle = get_rtos_task_cycle_info_by_stack(stack_all[i].stack_tail);
            task_name[configMAX_TASK_NAME_LEN - 1] = '\0';
            strcpy(task_name,stack_all[i].task_name);
            if((len = strlen(stack_all[i].task_name)) != (configMAX_TASK_NAME_LEN-1))
            {
                for(j = len;j < configMAX_TASK_NAME_LEN-1;j++)
                    task_name[j] = ' ';
            }
            printf("%s\t%010d\t%d\t%010d\t%010d\t%010lld\t%.2f\r\n",
                   task_name,pcycle->nUsedRecent,pcycle->wActiveCount,
                   pcycle->nUsedMax,pcycle->nUsedMin,pcycle->lUsedTotal/pcycle->wActiveCount,
                   ((double)(pcycle->lUsedTotal))/((double)count)*((double)100.0f));
        }
    }
    //logctrl_instance()->is_sync = 0;
    return 0;
}

LTSH_FUNCTION_EXPORT(perf,"show perf info");
#endif

#endif /* end of configUSE_STACK_MAX_USAGE */

