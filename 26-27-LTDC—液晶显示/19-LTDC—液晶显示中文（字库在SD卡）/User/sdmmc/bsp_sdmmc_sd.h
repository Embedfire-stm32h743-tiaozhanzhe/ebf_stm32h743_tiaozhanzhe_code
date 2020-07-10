#ifndef __BSP_SDIO_SD_H
#define __BSP_SDIO_SD_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx.h"

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup STM32746G_DISCOVERY
  * @{
  */
    
/** @addtogroup STM32746G_DISCOVERY_SD
  * @{
  */    

/** @defgroup STM32746G_DISCOVERY_SD_Exported_Types STM32746G_DISCOVERY_SD Exported Types
  * @{
  */

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

#define SD_DATATIMEOUT           ((uint32_t)100000000)
    

/**
  * @}
  */
  
/** @defgroup STM32746G_DISCOVERY_SD_Exported_Macro STM32746G_DISCOVERY_SD Exported Macro
  * @{
  */ 
/**
  * @}
  */
   
uint8_t BSP_SD_Init(void);
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
