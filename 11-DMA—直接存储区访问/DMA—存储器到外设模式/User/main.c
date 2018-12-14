/**
  ******************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   USART—USART1接发例程
  ******************************************************************
  * @attention
  *
  * 实验平台:野火 STM32H743开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************
  */  
#include "stm32h7xx.h"
#include "main.h"
#include "./usart/bsp_usart_dma.h"
#include "./led/bsp_led.h"
#include "./delay/core_delay.h"   

extern DMA_HandleTypeDef DMA_Handle;
__attribute__ ((at(0x30000000))) uint8_t SendBuff[SENDBUFF_SIZE];                                   

int main(void)
{
  uint16_t i;
    
  /* 系统时钟初始化成400MHz */
  SystemClock_Config();
  /* 初始化USART */
  DEBUG_USART_Config(); 

  /* 配置使用DMA模式 */
  USART_DMA_Config();  

  /* 配置RGB彩色灯 */
  LED_GPIO_Config();
    
  printf("\r\n USART1 DMA TX 测试 \r\n");
  
  /*填充将要发送的数据*/
  for(i=0;i<SENDBUFF_SIZE;i++)
  {
    SendBuff[i]	 = 'A';    
  }
  

  /*为演示DMA持续运行而CPU还能处理其它事情，持续使用DMA发送数据，量非常大，
  *长时间运行可能会导致电脑端串口调试助手会卡死，鼠标乱飞的情况，
  *或把DMA配置中的循环模式改为单次模式*/
  
   HAL_UART_Transmit_DMA (&UartHandle,(uint8_t *)SendBuff,SENDBUFF_SIZE);
  /* 此时CPU是空闲的，可以干其他的事情 */  
  //例如同时控制LED
  while(1)
  {
    LED1_TOGGLE
    HAL_Delay(1000);
  }
}
/**
  * @brief  System Clock 配置
  *         system Clock 配置如下: 
	*            System Clock source  = PLL (HSE)
	*            SYSCLK(Hz)           = 400000000 (CPU Clock)
	*            HCLK(Hz)             = 200000000 (AXI and AHBs Clock)
	*            AHB Prescaler        = 2
	*            D1 APB3 Prescaler    = 2 (APB3 Clock  100MHz)
	*            D2 APB1 Prescaler    = 2 (APB1 Clock  100MHz)
	*            D2 APB2 Prescaler    = 2 (APB2 Clock  100MHz)
	*            D3 APB4 Prescaler    = 2 (APB4 Clock  100MHz)
	*            HSE Frequency(Hz)    = 25000000
	*            PLL_M                = 5
	*            PLL_N                = 160
	*            PLL_P                = 2
	*            PLL_Q                = 4
	*            PLL_R                = 2
	*            VDD(V)               = 3.3
	*            Flash Latency(WS)    = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;
  
  /*使能供电配置更新 */
  MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

  /* 当器件的时钟频率低于最大系统频率时，电压调节可以优化功耗，
		 关于系统频率的电压调节值的更新可以参考产品数据手册。  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
 
  /* 启用HSE振荡器并使用HSE作为源激活PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
 
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
	/* 选择PLL作为系统时钟源并配置总线时钟分频器 */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK  | \
																 RCC_CLOCKTYPE_HCLK    | \
																 RCC_CLOCKTYPE_D1PCLK1 | \
																 RCC_CLOCKTYPE_PCLK1   | \
                                 RCC_CLOCKTYPE_PCLK2   | \
																 RCC_CLOCKTYPE_D3PCLK1);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;  
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; 
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; 
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; 
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
}
/****************************END OF FILE***************************/
