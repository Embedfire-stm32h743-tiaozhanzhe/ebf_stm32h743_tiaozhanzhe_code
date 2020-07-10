#ifndef __FTL_H
#define __FTL_H

#include "stm32h7xx.h"
//升级说明
//V1.1 20160124
//修改FTL_CopyAndWriteToBlock和FTL_WriteSectors函数,提高非0XFF时的写入速度.  
//V1.2 20160520
//1,修改FTL_ReadSectors,增加ECC出错判断,检测坏块处理,并增加多块连读,提高速度
//2,新增FTL_BlockCompare和FTL_SearchBadBlock函数,用于搜寻坏块
//3,修改FTL_Format坏块检测方式,增加FTL_USE_BAD_BLOCK_SEARCH宏
//V1.3 20160530
//修改当1bit ECC错误出现时，读取2次，来确认1bit 错误，以防错误的修改数据
////////////////////////////////////////////////////////////////////////////////// 	

//坏块搜索控制
//如果设置为1,将在FTL_Format的时候,搜寻坏块,耗时久(512M,3分钟以上),且会导致RGB屏乱闪
#define FTL_USE_BAD_BLOCK_SEARCH		0		//定义是否使用坏块搜索



uint8_t FTL_Init(void); 
void FTL_BadBlockMark(uint32_t blocknum);
uint8_t FTL_CheckBadBlock(uint32_t blocknum); 
uint8_t FTL_UsedBlockMark(uint32_t blocknum);
uint32_t FTL_FindUnusedBlock(uint32_t sblock,uint8_t flag);
uint32_t FTL_FindSamePlaneUnusedBlock(uint32_t sblock);
uint8_t FTL_CopyAndWriteToBlock(uint32_t Source_PageNum,uint16_t ColNum,uint8_t *pBuffer,uint32_t NumByteToWrite);
uint16_t FTL_LBNToPBN(uint32_t LBNNum); 
uint8_t FTL_WriteSectors(uint8_t *pBuffer,uint32_t SectorNo,uint16_t SectorSize,uint32_t SectorCount);
uint8_t FTL_ReadSectors(uint8_t *pBuffer,uint32_t SectorNo,uint16_t SectorSize,uint32_t SectorCount);
uint8_t FTL_CreateLUT(uint8_t mode);
uint8_t FTL_BlockCompare(uint32_t blockx,uint32_t cmpval);
uint32_t FTL_SearchBadBlock(void);
uint8_t FTL_Format(void); 
#endif

