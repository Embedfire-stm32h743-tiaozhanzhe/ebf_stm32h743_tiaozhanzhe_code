/**
  ******************************************************************************
  * @file    bsp_sdio_sd.c
  * @author  fire
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   SDIO-SD驱动
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火  STM32 F767 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */ 
#include "./sdmmc/bsp_sdmmc_sd.h"
#include "sdmmc/sdmmc_test.h"

SD_HandleTypeDef uSdHandle;
extern uint8_t Wait_SDCARD_Ready(void);

/**
  * @brief  初始化SD卡设备
  * @retval SD卡状态
  */
uint8_t BSP_SD_Init(void)
{ 
  uint8_t sd_state = HAL_OK;
  
  /* 定义SDMMC句柄 */
  uSdHandle.Instance = SDMMC1;
  HAL_SD_DeInit(&uSdHandle);
	/* SDMMC内核时钟200Mhz, SDCard时钟25Mhz  */
  uSdHandle.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
  uSdHandle.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  uSdHandle.Init.BusWide             = SDMMC_BUS_WIDE_4B;
  uSdHandle.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  uSdHandle.Init.ClockDiv            = 4;
  
  /* 初始化SD底层驱动 */
  BSP_SD_MspInit(&uSdHandle, NULL);

  /* HAL SD 初始化 */
  if(HAL_SD_Init(&uSdHandle) != HAL_OK)
  {
    sd_state = MSD_ERROR;
  }
  return  sd_state;
}

/**
  * @brief  取消初始化SD卡设备
  * @retval SD状态
  */
uint8_t BSP_SD_DeInit(void)
{ 
  uint8_t sd_state = HAL_OK;
 
  uSdHandle.Instance = SDMMC1;
  
  /* 取消初始化SD卡设备 */
  if(HAL_SD_DeInit(&uSdHandle) != HAL_OK)
  {
    sd_state = MSD_ERROR;
  }

  /* 取消初始化SD底层驱动 */
  uSdHandle.Instance = SDMMC1;
  BSP_SD_MspDeInit(&uSdHandle, NULL);
  
  return  sd_state;
}

/**
  * @brief  在轮询模式下从SD卡中的指定地址读取块
  * @param  pData: 指向将包含要传输的数据的缓冲区的指针
  * @param  ReadAddr: 从哪里读取数据的地址  
  * @param  NumOfBlocks: 读取的扇区数 
  * @retval SD 状态
  */
uint8_t BSP_SD_ReadBlocks(uint8_t *pData, uint64_t ReadAddr, uint32_t NumOfBlocks)
{
  if(HAL_SD_ReadBlocks(&uSdHandle,pData, ReadAddr,NumOfBlocks,SD_TIMEOUT) != HAL_OK)
  {
    return MSD_ERROR;
  }
  else
  {
    return HAL_OK;
  }
}

/**
  * @brief  在轮询模式下从SD卡中的指定地址写入块 
  * @param  pData: 指向将包含要传输的数据的缓冲区的指针
  * @param  WriteAddr: 从哪里读取数据的地址  
  * @param  NumOfBlocks: 读取的扇区数 
  * @retval SD 状态
  */
uint8_t BSP_SD_WriteBlocks(uint8_t *pData, uint64_t WriteAddr,uint32_t NumOfBlocks)
{
  if(HAL_SD_WriteBlocks(&uSdHandle,pData, WriteAddr,NumOfBlocks,SD_TIMEOUT) != HAL_OK)
  {
    return MSD_ERROR;
  }
  else
  {
    return HAL_OK;
  }
}

/**
  * @brief  以DMA方式从SD卡中的指定地址读取块
  * @param  pData: 指向将包含要传输的数据的缓冲区的指针
  * @param  ReadAddr: 从哪里读取数据的地址  
  * @param  NumOfBlocks: 读取的扇区数 
  * @retval SD 状态
  */
uint8_t BSP_SD_ReadBlocks_DMA(uint8_t *pData, uint64_t ReadAddr, uint32_t NumOfBlocks)
{
  uint8_t sd_state = HAL_OK;
  
  /* 以DMA方式从SD卡中的指定地址读取块 */
  if(HAL_SD_ReadBlocks_DMA(&uSdHandle, pData, ReadAddr,NumOfBlocks) != HAL_OK)
  {
    sd_state = MSD_ERROR;
  }
  
  return sd_state;
}

/**
  * @brief  以DMA方式从SD卡中的指定地址写入块
  * @param  pData: 指向将包含要传输的数据的缓冲区的指针
  * @param  ReadAddr: 从哪里写入数据的地址  
  * @param  NumOfBlocks: 读取的扇区数 
  * @retval SD 状态
  */
uint8_t BSP_SD_WriteBlocks_DMA(uint8_t *pData, uint64_t WriteAddr, uint32_t NumOfBlocks)
{
  uint8_t sd_state = HAL_OK;
  
  /* 以DMA方式从SD卡中的指定地址写入块 */
  if(HAL_SD_WriteBlocks_DMA(&uSdHandle, pData, WriteAddr, NumOfBlocks) != HAL_OK)
  {
    sd_state = MSD_ERROR;
  }
  
  return sd_state;
}

/**
  * @brief  擦除给定SD卡的指定存储区域 
  * @param  StartAddr: 开始地址
  * @param  EndAddr: 结束地址
  * @retval SD 状态
  */
uint8_t BSP_SD_Erase(uint64_t StartAddr, uint64_t EndAddr)
{
  if(HAL_SD_Erase(&uSdHandle, StartAddr, EndAddr) != HAL_OK)
  {
    return MSD_ERROR;
  }
	if(Wait_SDCARD_Ready() != HAL_OK)
  {
		return MSD_ERROR;
  }
  return HAL_OK;
}

/**
  * @brief  初始化SD外设
  * @param  hsd: SD 句柄
  * @param  Params
  * @retval None
  */
void BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* 使能 SDMMC 时钟 */
  __HAL_RCC_SDMMC1_CLK_ENABLE();

  /* 使能 GPIOs 时钟 */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  
  /* 配置GPIO复用推挽、上拉、高速模式 */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF12_SDIO1;
  
  /* GPIOC 配置 */
  gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOC, &gpio_init_structure);

  /* GPIOD 配置 */
  gpio_init_structure.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOD, &gpio_init_structure);
	
  __HAL_RCC_SDMMC1_FORCE_RESET();
  __HAL_RCC_SDMMC1_RELEASE_RESET();
  /* SDIO 中断配置 */
  HAL_NVIC_SetPriority(SDMMC1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
    
}

/**
  * @brief  取消初始化SD底层驱动
  * @param  hsd: SD handle
  * @param  Params
  * @retval None
  */
void BSP_SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params)
{
  /* 禁用SDIC中断*/
  HAL_NVIC_DisableIRQ(SDMMC1_IRQn);

  /* 禁用SDMMC1时钟 */
  __HAL_RCC_SDMMC1_CLK_DISABLE();

}

/**
  * @brief  处理SD卡中断请求
  * @retval 无
  */
void BSP_SD_IRQHandler(void)
{
  HAL_SD_IRQHandler(&uSdHandle);
}

/**
  * @brief  获取有关特定SD卡的SD信息
  * @param  CardInfo: 指向HAL_SD_CardInfoTypedef结构体的指针
  * @retval 无 
  */
void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo)
{
  /* 获取SD信息 */
  HAL_SD_GetCardInfo(&uSdHandle, CardInfo);
}


