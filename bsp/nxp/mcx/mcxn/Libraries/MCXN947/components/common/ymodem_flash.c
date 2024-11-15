#include <stdint.h>
#include <string.h>
#include <stm32h5xx_hal.h>

#define YMODE_FLASH_PG_SIZE          16U

static uint8_t pg_array[YMODE_FLASH_PG_SIZE] = {0xff};
static uint8_t index = 0;
static uint32_t last_addr;

static uint32_t ymode_flash_write_byte(uint32_t addr,uint8_t data)
{
     pg_array[index++] = data;
     
    if(index == YMODE_FLASH_PG_SIZE)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, addr - YMODE_FLASH_PG_SIZE + 1, ((uint32_t)pg_array));

        /* reset index & cache buffer */
        index = 0;
        memset((void *)pg_array,0xff,YMODE_FLASH_PG_SIZE);
        last_addr = addr - YMODE_FLASH_PG_SIZE;
    }

    return data;
}


uint32_t ymode_flash_write(uint32_t addr, uint8_t *data, uint32_t datalen)
{
    for(int i = 0;i < datalen;i++)
    {
         ymode_flash_write_byte(addr,data[i]);
         addr++;
    }
    
    return 0;
}


uint32_t ymode_flash_start(void)
{
     FLASH_EraseInitTypeDef EraseInitStruct;

     uint32_t sectorerr;

     HAL_ICACHE_Disable();
     
     /* Unlock the Flash to enable the flash control register access *************/
     HAL_FLASH_Unlock();
    
    if (READ_BIT(FLASH->OPTSR_CUR, FLASH_OPTSR_SWAP_BANK) == 0)
    {
         EraseInitStruct.Banks = FLASH_BANK_2;
    }
    else
    {
          EraseInitStruct.Banks = FLASH_BANK_1;
    }
    
    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector        = 0;
    EraseInitStruct.NbSectors     = 32;
    
    return HAL_FLASHEx_Erase(&EraseInitStruct, &sectorerr); 
}

extern CRC_HandleTypeDef hcrc;

uint32_t ymode_flash_end(void)
{
  
    uint32_t crc = 0;

    if(index != 0)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, last_addr + YMODE_FLASH_PG_SIZE, ((uint32_t)pg_array));
    }
    
    /* check image crc value */
    HAL_CRC_DeInit(&hcrc);
    hcrc.Instance = CRC;
    hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
    hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
    hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_WORD;
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
    HAL_CRC_Init(&hcrc);
    
    crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)0x08040000,0x3fffc/4);
    
    if(crc^0xffffffff == *((uint32_t *)0x0807fffc))
    {  
        HAL_FLASH_OB_Unlock();
        
        uint32_t bank_swap = READ_REG(FLASH->OPTSR_PRG);
        
        if(READ_BIT(FLASH->OPTSR_CUR, FLASH_OPTSR_SWAP_BANK) == 0)
        {
            bank_swap |= FLASH_OPTSR_SWAP_BANK_Msk;
        }
        else
        {
            bank_swap &= ~FLASH_OPTSR_SWAP_BANK_Msk;
        }
        
        WRITE_REG(FLASH->OPTSR_PRG,bank_swap);
        
        MODIFY_REG(FLASH->OPTCR,FLASH_OPTCR_OPTSTART_Msk,FLASH_OPTCR_OPTSTART_Msk);
        
        HAL_FLASH_OB_Lock();
    }
    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();
    
    HAL_ICACHE_Enable();    
    
    return 0;  
}