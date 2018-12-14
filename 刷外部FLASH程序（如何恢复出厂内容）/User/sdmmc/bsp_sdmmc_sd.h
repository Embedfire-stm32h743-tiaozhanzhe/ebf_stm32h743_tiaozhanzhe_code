#ifndef __BSP_SDIO_SD_H
#define __BSP_SDIO_SD_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx.h"

extern SD_HandleTypeDef uSdHandle;
/** 
  * @brief SD Card information structure 
  */
#define SD_CardInfo HAL_SD_CardInfoTypedef
/**
  * @}
  */
   
/** 
  * @brief  SD status structure definition  
  */     
#define MSD_OK                        ((uint8_t)0x00)
#define MSD_ERROR                     ((uint8_t)0x01)

/**
  * @brief  SD transfer state definition
  */
#define   SD_TRANSFER_OK                ((uint8_t)0x00)
#define   SD_TRANSFER_BUSY              ((uint8_t)0x01)

#define SD_DATATIMEOUT           ((uint32_t)100000000)
    
/* DMA definitions for SD DMA transfer */
#define __DMAx_TxRx_CLK_ENABLE            __DMA2_CLK_ENABLE
#define SD_DMAx_Tx_CHANNEL                DMA_CHANNEL_4
#define SD_DMAx_Rx_CHANNEL                DMA_CHANNEL_4
#define SD_DMAx_Tx_STREAM                 DMA2_Stream6  
#define SD_DMAx_Rx_STREAM                 DMA2_Stream3  
#define SD_DMAx_Tx_IRQn                   DMA2_Stream6_IRQn
#define SD_DMAx_Rx_IRQn                   DMA2_Stream3_IRQn
#define SD_DMAx_Tx_IRQHandler             DMA2_Stream6_IRQHandler   
#define SD_DMAx_Rx_IRQHandler             DMA2_Stream3_IRQHandler 
/**
  * @}
  */
  
/** @defgroup STM32746G_DISCOVERY_SD_Exported_Macro STM32746G_DISCOVERY_SD Exported Macro
  * @{
  */ 
/**
  * @}
  */
   
/** @addtogroup STM32746G_DISCOVERY_SD_Exported_Functions
  * @{
  */   
uint8_t BSP_SD_Init(void);
uint8_t BSP_SD_DeInit(void);
uint8_t BSP_SD_ITConfig(void);
void    BSP_SD_DetectIT(void);
void    BSP_SD_DetectCallback(void);
uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint64_t ReadAddr,uint32_t NumOfBlocks);
uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint64_t WriteAddr, uint32_t NumOfBlocks);
uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint64_t ReadAddr, uint32_t NumOfBlocks);
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pData, uint64_t WriteAddr, uint32_t NumOfBlocks);
uint8_t BSP_SD_Erase(uint64_t StartAddr, uint64_t EndAddr);
void    BSP_SD_IRQHandler(void);
void    BSP_SD_DMA_Tx_IRQHandler(void);
void    BSP_SD_DMA_Rx_IRQHandler(void);
void    BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo);
uint8_t BSP_SD_IsDetected(void);
uint8_t BSP_SD_GetStatus(void);
/* These functions can be modified in case the current settings (e.g. DMA stream)
   need to be changed for specific application needs */
void    BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params);
void    BSP_SD_Detect_MspInit(SD_HandleTypeDef *hsd, void *Params);
void    BSP_SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params);
extern void MPU_Config(void);
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif /* __STM32746G_DISCOVERY_SD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
