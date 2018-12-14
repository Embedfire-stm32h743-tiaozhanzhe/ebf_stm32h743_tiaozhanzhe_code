/**
  ******************************************************************
  * @file    lan8720a.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   lan8720a应用函数接口
  ******************************************************************
  * @attention
  *
  * 实验平台:野火 STM32H743开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************
  */  
  
#include "LAN8720a.h" 
/**
  * @brief  初始化ETH引脚.
  * @param  无
  * @retval 无
  */    
static void ETH_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /* 使能端口时钟 */
    ETH_MDIO_GPIO_CLK_ENABLE();
    ETH_MDC_GPIO_CLK_ENABLE();
    ETH_RMII_REF_CLK_GPIO_CLK_ENABLE();
    ETH_RMII_CRS_DV_GPIO_CLK_ENABLE();
    ETH_RMII_RXD0_GPIO_CLK_ENABLE();
    ETH_RMII_RXD1_GPIO_CLK_ENABLE();
    ETH_RMII_TX_EN_GPIO_CLK_ENABLE();
    ETH_RMII_TXD0_GPIO_CLK_ENABLE();    
    ETH_RMII_TXD1_GPIO_CLK_ENABLE();
	
    /* 配置ETH_MDIO引脚 */
    GPIO_InitStructure.Pin = ETH_MDIO_PIN;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Alternate = ETH_MDIO_AF;
    HAL_GPIO_Init(ETH_MDIO_PORT, &GPIO_InitStructure);

    /* 配置ETH_MDC引脚 */
    GPIO_InitStructure.Pin = ETH_MDC_PIN;
    GPIO_InitStructure.Alternate = ETH_MDC_AF;
    HAL_GPIO_Init(ETH_MDC_PORT, &GPIO_InitStructure);

    /* 配置ETH_RMII_REF_CLK引脚 */
    GPIO_InitStructure.Pin = ETH_RMII_REF_CLK_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_REF_CLK_AF;
    HAL_GPIO_Init(ETH_RMII_REF_CLK_PORT, &GPIO_InitStructure);

    /* 配置ETH_RMII_CRS_DV引脚 */
    GPIO_InitStructure.Pin = ETH_RMII_CRS_DV_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_CRS_DV_AF;
    HAL_GPIO_Init(ETH_RMII_CRS_DV_PORT, &GPIO_InitStructure);

    /* 配置ETH_RMII_RXD0引脚 */
    GPIO_InitStructure.Pin = ETH_RMII_RXD0_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_RXD0_AF;
    HAL_GPIO_Init(ETH_RMII_RXD0_PORT, &GPIO_InitStructure);

    /* 配置ETH_RMII_RXD1引脚 */
    GPIO_InitStructure.Pin = ETH_RMII_RXD1_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_RXD1_AF;
    HAL_GPIO_Init(ETH_RMII_RXD1_PORT, &GPIO_InitStructure);

    /* 配置ETH_RMII_TX_EN引脚 */
    GPIO_InitStructure.Pin = ETH_RMII_TX_EN_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_TX_EN_AF;
    HAL_GPIO_Init(ETH_RMII_TX_EN_PORT, &GPIO_InitStructure);

    /* 配置ETH_RMII_TXD0引脚 */
    GPIO_InitStructure.Pin = ETH_RMII_TXD0_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_TXD0_AF;
    HAL_GPIO_Init(ETH_RMII_TXD0_PORT, &GPIO_InitStructure);

    /* 配置ETH_RMII_TXD1引脚 */
    GPIO_InitStructure.Pin = ETH_RMII_TXD1_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_TXD1_AF;
    HAL_GPIO_Init(ETH_RMII_TXD1_PORT, &GPIO_InitStructure);      
}  
/**
  * @brief  初始化ETH外设时钟，引脚.
  * @param  heth: ETH handle
  * @retval None
  */  
void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
    ETH_GPIO_Config();
    /* 使能以太网时钟  */
    __HAL_RCC_ETH1MAC_CLK_ENABLE();
    __HAL_RCC_ETH1TX_CLK_ENABLE();
    __HAL_RCC_ETH1RX_CLK_ENABLE();    
}  

/**
  * @brief  初始化LAN8720A.
  * @param  heth: ETH handle
  * @retval HAL_StatusTypeDef：状态值
  */  
HAL_StatusTypeDef LAN8720_Init(ETH_HandleTypeDef *heth)
{
    uint32_t phyreg = 0;
    uint32_t TIME_Out = 0;
    //软件复位LAN8720A
    if(HAL_ETH_WritePHYRegister(heth, LAN8720A_PHY_ADDRESS, PHY_BCR, PHY_RESET) != HAL_OK)
    {
        return HAL_ERROR;
    }
    //等待LAN8720A复位完成
    HAL_Delay(PHY_RESET_DELAY);
    
    if((HAL_ETH_WritePHYRegister(heth, LAN8720A_PHY_ADDRESS, PHY_BCR, PHY_AUTONEGOTIATION)) != HAL_OK)
    {
      return HAL_ERROR;   
    }     
    //等待LAN8720A写入完成
    HAL_Delay(0xFFF);
    do
    {     
      HAL_ETH_ReadPHYRegister(heth, LAN8720A_PHY_ADDRESS, PHY_BSR, &phyreg);
      TIME_Out++;
      if(TIME_Out > PHY_READ_TO) 
        return HAL_TIMEOUT;
    } while (((phyreg & PHY_AUTONEGO_COMPLETE) != PHY_AUTONEGO_COMPLETE));
    
    return HAL_OK;    
}

/**
  * @brief  获取LAN8720的工作状态
  * @param  heth: ETH handle
  * @retval phyreg：LAN8720的SR寄存器值
  */
uint32_t LAN8720_GetLinkState(ETH_HandleTypeDef *heth)
{
    uint32_t phyreg = 0;   
  
    if(HAL_ETH_ReadPHYRegister(heth, LAN8720A_PHY_ADDRESS, PHY_SR, &phyreg) == HAL_OK)
        return phyreg;
    return 0;

}
/*********************************************END OF FILE**********************/
