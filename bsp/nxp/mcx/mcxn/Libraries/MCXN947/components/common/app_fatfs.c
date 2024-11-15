/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    FatFs/FatFs_RAMDisk/FatFs/App/app_fatfs.c
  * @author  MCD Application Team
  * @brief   FatFs_uSD_Standalone application file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "ff.h"
#include "ff_gen_drv.h"
#include "sram_diskio.h" /* defines SRAMDISK_Driver as external */
#include <stdio.h>
#include "littleshell.h"

#define APP_OK                     0
#define APP_ERROR                  -1
#define APP_INIT                   1

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
  APPLICATION_IDLE = 0,
  APPLICATION_INIT,
  APPLICATION_RUNNING,
}FS_FileOperationsTypeDef;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FATFS_MKFS_ALLOWED 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
FATFS SRAMDISKFatFS;    /* File system object for SRAMDISK logical drive */
FIL SRAMDISKFile;       /* File object for SRAMDISK */
char SRAMDISKPath[4];   /* SRAMDISK logical drive path */
/* USER CODE BEGIN PV */
FS_FileOperationsTypeDef Appli_state = APPLICATION_IDLE;
uint8_t workBuffer[_MAX_SS];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static int32_t FS_FileOperations(void);
/* USER CODE END PFP */

/**
  * @brief  FatFs initialization
  * @param  None
  * @retval Initialization result
  */
int32_t MX_FATFS_Init(void)
{
  /*## FatFS: Link the disk I/O driver(s)  ###########################*/
   SRAMDISKFatFS.fs_type = FS_FAT16;
if (FATFS_LinkDriver(&SRAMDISK_Driver, SRAMDISKPath) != 0)
  /* USER CODE BEGIN FATFS_Init */
  {
    return APP_ERROR;
  }
  else
  {
    Appli_state = APPLICATION_INIT;
    return APP_OK;
  }
  /* USER CODE END FATFS_Init */
}

/**
  * @brief  FatFs application main process
  * @param  None
  * @retval Process result
  */
int32_t MX_FATFS_Process(void)
{
  /* USER CODE BEGIN FATFS_Process */
  int32_t process_res = APP_OK;
  /* Mass Storage Application State Machine */
  switch(Appli_state)
  {
  case APPLICATION_INIT:
#if FATFS_MKFS_ALLOWED
      if (f_mkfs(SRAMDISKPath, FM_ANY, 0, workBuffer, sizeof(workBuffer))!= FR_OK)
      {
        process_res = APP_ERROR;
      }
      else
      {
        process_res = APP_INIT;
      }
#else
      process_res = APP_INIT;
#endif
      Appli_state = APPLICATION_RUNNING;
    break;
  case APPLICATION_RUNNING:
      process_res = FS_FileOperations();
      Appli_state = APPLICATION_IDLE;
    break;

  case APPLICATION_IDLE:
  default:
    break;
  }
  return process_res;
  /* USER CODE END FATFS_Process */
}

/**
  * @brief  Gets Time from RTC (generated when FS_NORTC==0; see ff.c)
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return 0;
  /* USER CODE END get_fattime */
}

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN Application */
/**
  * @brief File system : file operation
  * @retval File operation result
  */
static int32_t FS_FileOperations(void)
{
  FRESULT res; /* FatFs function common result code */
  uint32_t byteswritten, bytesread; /* File write/read counts */
  uint8_t wtext[] = "This is STM32 working with FatFs and SRAM diskio driver"; /* File write buffer */
  uint8_t rtext[100]; /* File read buffer */

  /* Register the file system object to the FatFs module */
  if(f_mount(&SRAMDISKFatFS, (TCHAR const*)SRAMDISKPath, 0) == FR_OK)
  {
    /* Create and Open a new text file object with write access */
    if(f_open(&SRAMDISKFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
    {
      /* Write data to the text file */
      res = f_write(&SRAMDISKFile, wtext, sizeof(wtext), (void *)&byteswritten);

      if((byteswritten > 0) && (res == FR_OK))
      {
        /* Close the open text file */
        f_close(&SRAMDISKFile);

        /* Open the text file object with read access */
        if(f_open(&SRAMDISKFile, "STM32.TXT", FA_READ) == FR_OK)
        {
          /* Read data from the text file */
          res = f_read(&SRAMDISKFile, rtext, sizeof(rtext), (void *)&bytesread);

          if((bytesread > 0) && (res == FR_OK))
          {
            /* Close the open text file */
            f_close(&SRAMDISKFile);

            /* Compare read data with the expected data */
            if((bytesread == byteswritten))
            {
              /* Success of the demo: no error occurrence */
              return 0;
            }
          }
        }
      }
    }
  }
  /* Error */
  return -1;
}



unsigned int fatfs_test(char argc,char ** argv)
{
   int32_t ProcessStatus = 0;

  MX_FATFS_Init();

  while (1)
  {
    ProcessStatus = MX_FATFS_Process();
    /* Call middleware background task */
    if (ProcessStatus == APP_ERROR)
    {
      printf("fatfs error.\r\n");
    }
    else if (ProcessStatus == APP_OK)
    {
      printf("fatfs sucess.\r\n");
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
}
LTSH_FUNCTION_EXPORT(fatfs_test,"fatfs test");


FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\r\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

unsigned int ls(char argc,char ** argv)
{
    char buff[20];

    strcpy(buff, "/");
    scan_files(buff);

    return 0;
}

LTSH_FUNCTION_EXPORT(ls,"fatfs ls dir.");

unsigned int fatfs(char argc,char ** argv)
{
  int cmd = 0;
  FRESULT res; /* FatFs function common result code */
  uint32_t byteswritten, bytesread; /* File write/read counts */
  uint8_t wtext[] = "This is STM32 working with FatFs and SRAM diskio driver"; /* File write buffer */
  uint8_t rtext[100]; /* File read buffer */
  cmd = atoi(argv[1]);

  switch(cmd)
  {
  case 0:
    /*## FatFS: Link the disk I/O driver(s)  ###########################*/
    SRAMDISKFatFS.fs_type = FS_FAT16;
    if (FATFS_LinkDriver(&SRAMDISK_Driver, SRAMDISKPath) != 0)
    {
      printf("fatfs link driver error \r\n");
    }
    else
    {
      printf("fatfs link driver path = %s\r\n",SRAMDISKPath);
    }
    break;
  case 1:
    if (f_mkfs(SRAMDISKPath, FM_ANY, 0, workBuffer, sizeof(workBuffer))!= FR_OK)
    {
      printf("Create an FAT/exFAT volume failed.\r\n");
    }
    else
    {
      printf("Create an FAT/exFAT volume ok.\r\n");
    }
    break;
  case 2:/* mount fs */
    if(f_mount(&SRAMDISKFatFS, (TCHAR const*)SRAMDISKPath, 0) == FR_OK)
    {
      printf("mount FAT/exFAT ok.\r\n");
    }
    else
    {
      printf("mount FAT/exFAT ok.\r\n");
    }
    break;
  case 3:
        /* Create and Open a new text file object with write access */
    if(f_open(&SRAMDISKFile, argv[2], FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
    {
      /* Write data to the text file */
      res = f_write(&SRAMDISKFile, wtext, sizeof(wtext), (void *)&byteswritten);

      if((byteswritten > 0) && (res == FR_OK))
      {
        /* Close the open text file */
        f_close(&SRAMDISKFile);
      }
    }
    else
    {
      printf("open file failed\r\n");
    }
    break;
  case 4:
    /* Open the text file object with read access */
    if(f_open(&SRAMDISKFile, argv[2], FA_READ) == FR_OK)
    {
        /* Read data from the text file */
        res = f_read(&SRAMDISKFile, rtext, sizeof(rtext), (void *)&bytesread);

        if((bytesread > 0) && (res == FR_OK))
        {
            /* Close the open text file */
            f_close(&SRAMDISKFile);

            /* Compare read data with the expected data */
            printf("%s\r\n",rtext);
        }
        else
        {
          printf("read failed\r\n");
        }
      }
    break;
  default:
    break;
  }
  return 0;
}
LTSH_FUNCTION_EXPORT(fatfs,"fatfs api test");


unsigned int mkdir(char argc,char ** argv)
{
  FRESULT res;
  res = f_mkdir(argv[1]);
  if (res)
  {
    printf("mkdir failed.\r\n");
  }

   return 0;
}

LTSH_FUNCTION_EXPORT(mkdir,"make a dir.");

/* USER CODE END Application */
