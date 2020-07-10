/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   SDIO sd卡测试驱动（不含文件系统）
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 H743 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
#include "sdmmc/sdmmc_test.h"
#include "led/bsp_led.h"
#include "sdmmc/bsp_sdmmc_sd.h"
#include "./usart/bsp_debug_usart.h"

/******** SD发送缓冲区定义 *******/
__attribute__((section (".RAM_D1"))) uint8_t aTxBuffer[BUFFER_WORD_SIZE*4];

/******** SD接收缓冲区定义 *******/
__attribute__((section (".RAM_D1"))) uint8_t aRxBuffer[BUFFER_WORD_SIZE*4];

__IO uint8_t RxCplt, TxCplt;

__IO uint8_t step = 0;

/**
  * @brief  发生错误时执行该函数
  * @param  无
  * @retval 无
  */
static void Error_Handler(void)
{
  printf(" - Error \n");
	LED_RGBOFF;
  while(1)
  {
    /* 有错误红色的闪烁 */
    LED1_TOGGLE;
		HAL_Delay(300);
  }
}
/**
  * @brief  等待SD卡进入就绪状态
  * @param  无
  * @retval 无
  */
uint8_t Wait_SDCARD_Ready(void)
{
  uint32_t loop = SD_TIMEOUT;
  
  /* 等待擦除过程完成 */
  /* 确认SD卡擦除后已准备好使用 */
  while(loop > 0)
  {
    loop--;
    if(HAL_SD_GetCardState(&uSdHandle) == HAL_SD_CARD_TRANSFER)
    {
        return HAL_OK;
    }
  }
  return HAL_ERROR;
}

/**
  * @brief 接收传输已完成回调函数
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  if(Wait_SDCARD_Ready() != HAL_OK)
  {
    Error_Handler();
  }
  SCB_InvalidateDCache_by_Addr((uint32_t*)aRxBuffer, BUFFER_WORD_SIZE*4);
  RxCplt=1;
}

/**
* @brief 发送传输已完成回调函数
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
  if(Wait_SDCARD_Ready() != HAL_OK)
  {
    Error_Handler();
  }
  TxCplt=1;
}

/**
  * @brief SD错误回调函数
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
  Error_Handler();
}
/**
  * @brief  配置MPU属性
  * @note   如果这个存储器接口是AXI，那么基地址是0x30040000
  *         配置区域大小为32KB，因为与D2SRAM3大小相同
  * @note   SDIO内部DMA可访问的SRAM1，基地址为0x24000000。
            配置区域大小为512KB
  * @param  无
  * @retval 无
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* 禁用MPU */
  HAL_MPU_Disable();

  /* 将MPU属性配置为SRAM1的WT */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* 使能MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
  
}

void SD_Test(void)
{
	uint32_t index = 0;
	static uint32_t start_time = 0;
	static uint32_t stop_time = 0;
	
	MPU_Config();
	/*------------------------------ SD 初始化 ------------------------------ */
	/* SD卡使用SDIO中断及DMA中断接收数据，中断服务程序位于bsp_sdio_sd.c文件尾*/
	if(BSP_SD_Init() != HAL_OK)
	{    
		LED_RED;
		printf("SD卡初始化失败，请确保SD卡已正确接入开发板，或换一张SD卡测试！\n");
	}
	else
	{
		if(BSP_SD_Erase(ADDRESS, ADDRESS+BUFFERSIZE) != HAL_OK)
		{
			Error_Handler();
		}			
		while(1)
		{
			switch(step)
			{
				case 0:
				{
					/*初始化发送缓冲区*/
					for (index = 0; index < BUFFERSIZE; index++)
					{
						aTxBuffer[index] = DATA_PATTERN + index;
					}
					/*清除Dcache的数据*/
					SCB_CleanDCache_by_Addr((uint32_t*)aTxBuffer, BUFFER_WORD_SIZE*4);
					printf(" ****************** 开始写入测试 ******************* \n");
					printf(" - 写入缓冲区大小： %lu MB   \n", (long)(DATA_SIZE>>20));
					index = 0;
					start_time = HAL_GetTick();
					step++;
				}
				break;
				case 1:
				{
					TxCplt = 0;
					/*开始发送数据，DMA方式*/
					if(BSP_SD_WriteBlocks_DMA(aTxBuffer, ADDRESS, NB_BLOCK_BUFFER) != HAL_OK)
					{
						Error_Handler();
					}
					step++;
				}
				break;
				case 2:
				{
					if(TxCplt != 0)
					{
						/* 传输完成蓝灯闪烁一次 */
						LED3_TOGGLE;
						
						/* 缓冲区传输完成 */
						index++;
						if(index<NB_BUFFER)
						{
							/* 还有数据没传输完成 */
							step--;
						}
						else
						{
							stop_time = HAL_GetTick();
							printf(" - 写入数据时间(ms): %lu  -  写入速度: %02.2f MB/s  \n",(long)(stop_time - start_time),\
																							(float)((float)(DATA_SIZE>>10)/(float)(stop_time - start_time)));
							/* 所有数据都被传输 */
							step++;
						}
					}
				}
				break;
				case 3:
				{
					/*初始化接收缓冲区*/
					for (index = 0; index < BUFFERSIZE; index++)
					{
						aRxBuffer[index] = 0;
					}
					/*清除Dcache的数据*/
					SCB_CleanDCache_by_Addr((uint32_t*)aRxBuffer, BUFFER_WORD_SIZE*4);
					printf(" ******************* 开始读取测试 ******************* \n");
					printf(" - 读取缓冲区大小: %lu MB   \n", (long)(DATA_SIZE>>20));
					start_time = HAL_GetTick();
					index = 0;
					step++;
				}
				break;
				case 4:
				{
					/*开始接收数据，DMA方式*/
					RxCplt = 0;
					if(BSP_SD_ReadBlocks_DMA(aRxBuffer, ADDRESS, NB_BLOCK_BUFFER) != HAL_OK)
					{
						Error_Handler();
					}
					step++;
				}
				break;
				case 5:
				{
					if(RxCplt != 0)
					{
						/* 传输完成蓝灯闪烁一次 */
						LED3_TOGGLE;
						/* 缓冲区传输完成 */
						index++;
						if(index<NB_BUFFER)
						{
							 /* 还有数据没传输完成 */
							step--;
						}
						else
						{
							stop_time = HAL_GetTick();
							printf(" 读取数据时间(ms): %lu  -  读取速度: %02.2f MB/s  \n", (long)(stop_time - start_time),\
																						(float)((float)(DATA_SIZE>>10)/(float)(stop_time - start_time)));
							/* 所有数据都被传输 */
							step++;
						}
					}
				}
				break;
				case 6:
				{
					/*检查接收缓冲区数据*/
					index=0;
					printf(" ********************* 校验数据 ********************** \n");
					while((index<BUFFERSIZE) && (aRxBuffer[index] == aTxBuffer[index]))
					{
						index++;
					}
					
					if(index != BUFFERSIZE)
					{
						printf(" 校验数据出错 !!!!   \n");
						Error_Handler();
					}
					printf(" 校验数据正确  \n");
					step++;
				}
				break;			
				default :
					Error_Handler();
			}
			/*读写测试完成*/
			if(step == 7)
			{
				LED_GREEN;
				printf(" SD读写测试完成 \n");
				break;
			}
		}
	}

}

/*********************************************END OF FILE**********************/
