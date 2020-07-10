#ifndef __SDIO_TEST_H
#define __SDIO_TEST_H

#include "stm32h7xx.h"

extern __IO uint8_t step;
	
/* 数据大小10Mb */
#define DATA_SIZE              ((uint32_t)0x00A00000U) 
/* 缓冲区大小：256Kb */
#define BUFFER_SIZE            ((uint32_t)0x00040000U)
/* 缓冲区数量 */
#define NB_BUFFER              DATA_SIZE / BUFFER_SIZE
/* 每个缓冲区的扇区数量 */
#define NB_BLOCK_BUFFER        BUFFER_SIZE / BLOCKSIZE 
/* 缓冲区大小（双字节） */
#define BUFFER_WORD_SIZE       (BUFFER_SIZE>>2)        

#define SD_TIMEOUT             ((uint32_t)0x00100000U)
/* SD读写的地址 */
#define ADDRESS                ((uint32_t)0x00000400U)
/* 输入的数据 */
#define DATA_PATTERN           ((uint32_t)0xB5F3A5F3U) 

#define COUNTOF(__BUFFER__)    (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
#define BUFFERSIZE             (COUNTOF(aTxBuffer) - 1)

void SD_Test(void);

#endif
/*****************************END OF FILE**************************/
