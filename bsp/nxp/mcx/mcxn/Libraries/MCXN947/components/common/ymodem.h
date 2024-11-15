/**********************************************************
 *
 * File :
 *		YModem.h
 *
 * Description:
 *		YModem protocal (use statemachine)
 *
 */


#ifndef __YMODEM_H__
#define __YMODEM_H__

#include <stdint.h>
/*
 * Receive Frame return values
 */

typedef int32_t                     YmodemRetVal;

#define YMODEM_OK                   0       /* OK */
#define YMODEM_ERROR                -1      /* ERROR */
#define YMODEM_TIMEOUT              -2      /* Receive timeout */
#define YMODEM_PARAM_ERR            -3      /* Parameters error */
#define YMODEM_ABORT                -4      /* Remote abort */

#define YMODE_FLASH_BASE_START       ((uint32_t)0x08040000u) /* Base BANK2 of SECTOR 1, 8 Kbytes */
uint32_t ymode_flash_start(void);
uint32_t ymode_flash_end(void);
uint32_t ymode_flash_write(uint32_t addr, uint8_t *data, uint32_t datalen);

YmodemRetVal YmodemReceive(uint32_t addrSaveData, uint32_t * savedSize);

#endif //__YMODEM_H__

