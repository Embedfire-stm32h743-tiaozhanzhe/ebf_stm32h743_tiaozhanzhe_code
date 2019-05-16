/**
  ******************************************************************************
  * @file    bsp_i2c.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   I2C 配置文件
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 STM32 H743 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */

#include "./i2c/bsp_i2c.h"

I2C_HandleTypeDef  I2C_Handle; 

/**
  * @brief I2C MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - DMA configuration for transmission request by peripheral 
  *           - NVIC configuration for DMA interrupt request enable
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{  
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  I2Cx_SCL_GPIO_CLK_ENABLE();
  I2Cx_SDA_GPIO_CLK_ENABLE();
  /* Enable I2C1 clock */
  I2Cx_CLK_ENABLE(); 
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* I2C TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = I2Cx_SCL_AF;
  
  HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);
    
  /* I2C RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = I2Cx_SDA_PIN;
  GPIO_InitStruct.Alternate = I2Cx_SDA_AF;
    
  HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);
  
  	/* Force the I2C peripheral clock reset */  
	I2Cx_FORCE_RESET() ; 

	/* Release the I2C peripheral clock reset */  
	I2Cx_RELEASE_RESET(); 
}


/**
  * @brief  I2C 工作模式配置
  * @param  无
  * @retval 无
  */
void I2C_Mode_Config(void)
{
  I2Cx_CLK_ENABLE();
  I2C_Handle.Instance             = I2Cx;
  
  I2C_Handle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  I2C_Handle.Init.Timing           = 0x40604E73;//100KHz
  I2C_Handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  I2C_Handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  I2C_Handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  I2C_Handle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
  I2C_Handle.Init.OwnAddress1     = 0 ;
  I2C_Handle.Init.OwnAddress2     = 0; 
    /* Init the I2C */
  HAL_I2C_Init(&I2C_Handle);	

  HAL_I2CEx_AnalogFilter_Config(&I2C_Handle, I2C_ANALOGFILTER_ENABLE);    
}

/*********************************************END OF FILE**********************/
