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

#include "bsp_sdmmc_sd.h"

static SD_HandleTypeDef uSdHandle;
static void BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params);
/**
  * @brief  初始化SD卡设备
  * @retval SD卡状态
  */
uint8_t BSP_SD_Init(void)
{
  uint8_t sd_state = MSD_OK;
  
  uSdHandle.Instance = SDMMC1;
  
    
  /* SDMMC IP clock 200Mhz, SDCard clock 100Mhz  */
  uSdHandle.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
  uSdHandle.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  uSdHandle.Init.BusWide             = SDMMC_BUS_WIDE_1B;
  uSdHandle.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  uSdHandle.Init.ClockDiv            = 1;  

  /* 初始化SD底层驱动 */
  BSP_SD_MspInit(&uSdHandle, NULL);  
  sd_state = HAL_SD_Init(&uSdHandle); 
  return  sd_state;
}

/**
  * @brief  初始化SD外设
  * @param  hsd: SD 句柄
  * @param  Params
  * @retval None
  */
static void BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
  static DMA_HandleTypeDef dma_rx_handle;
  static DMA_HandleTypeDef dma_tx_handle;
  GPIO_InitTypeDef gpio_init_structure;

  /* 使能 SDMMC 时钟 */
  __HAL_RCC_SDMMC1_CLK_ENABLE();
  

  /* 使能 GPIOs 时钟 */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  
  /* 配置GPIO复用推挽、上拉、高速模式 */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_HIGH;
  gpio_init_structure.Alternate = GPIO_AF12_SDIO1;
  
  /* GPIOC 配置 */
  gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOC, &gpio_init_structure);

  /* GPIOD 配置 */
  gpio_init_structure.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOD, &gpio_init_structure);

  /* SDIO 中断配置 */
  HAL_NVIC_SetPriority(SDMMC1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
    

}

