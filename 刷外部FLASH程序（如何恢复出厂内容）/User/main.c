/**
  ******************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   SDMMC—SD卡读写测试
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
#include "./led/bsp_led.h"
#include "./usart/bsp_debug_usart.h"
#include "./sdmmc/bsp_sdmmc_sd.h"
#include "./key/bsp_key.h"
/* FatFs includes component */
#include "ff.h"
#include "ff_gen_drv.h"
#include "./fatfs/drivers/fatfs_flash_qspi.h"
#include "aux_data.h"
/**
  ******************************************************************************
  *                              定义变量
  ******************************************************************************
  */
char SDPath[4]; /* SD逻辑驱动器路径 */
extern FATFS sd_fs;	
FRESULT res_sd;                /* 文件操作结果 */
uint8_t SDworkBuffer[_MAX_SS];
static void SystemClock_Config(void);
extern FATFS flash_fs;
extern Diskio_drvTypeDef  SD_Driver;
//要复制的文件路径，到aux_data.c修改
extern char src_dir[];
extern char dst_dir[];

/**
	**************************************************************
	* Description : 初始化WiFi模块使能引脚，并禁用WiFi模块
	* Argument(s) : none.
	* Return(s)   : none.
	**************************************************************
	*/
static void WIFI_PDN_INIT(void)
{
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStruct;
	/*使能引脚时钟*/	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	/*选择要控制的GPIO引脚*/															   
	GPIO_InitStruct.Pin = GPIO_PIN_13;	
	/*设置引脚的输出类型为推挽输出*/
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;      
	/*设置引脚为上拉模式*/
	GPIO_InitStruct.Pull  = GPIO_PULLUP;
	/*设置引脚速率为高速 */   
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; 
	/*调用库函数，使用上面配置的GPIO_InitStructure初始化GPIO*/
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	/*禁用WiFi模块*/
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);  
}
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
	MPU_Config();
	/* 使能指令缓存 */
//	SCB_EnableICache();
//    /* 使能数据缓存 */
//    SCB_EnableDCache();
	/* 系统时钟初始化成400MHz */
	SystemClock_Config();
	/*禁用WiFi模块*/
	WIFI_PDN_INIT();	
	/* LED 端口初始化 */
	LED_GPIO_Config();	
	LED_BLUE;
	
	/* 配置串口1为：115200 8-N-1 */
	DEBUG_USART_Config();
	/* 初始化独立按键 */
	Key_GPIO_Config();
	printf("****** 这是一个SD卡文件系统实验 ******\r\n");

	//在外部SPI Flash挂载文件系统，文件系统挂载时会对SPI设备初始化
	TM_FATFS_FLASH_SPI_disk_initialize(NULL);

	//链接驱动器，创建盘符
	FATFS_LinkDriver(&SD_Driver, SDPath);
	//f_mkfs(SDPath, FM_ANY, 0, SDworkBuffer, sizeof(SDworkBuffer));		
	res_sd = f_mount(&sd_fs,(TCHAR const*)SDPath,1);
  //如果文件系统挂载失败就退出
  if(res_sd != FR_OK)
  {
    BURN_ERROR("f_mount ERROR!请给开发板插入SD卡然后重新复位开发板!");
    LED_RED;
    while(1);
  }    
    
  printf("\r\n 按一次KEY1开始烧写字库并复制文件到FLASH。 \r\n"); 
  printf("\r\n 注意该操作会把FLASH的原内容会被删除！！ \r\n"); 

  while(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)==0){};
  while(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)==1){}; 

    
  //烧录数据到flash的非文件系统区域    
  res_sd = burn_file_sd2flash(burn_data,AUX_MAX_NUM); 

  if(res_sd == FR_OK)
  {
    printf("\r\n\r\n\r\n"); 

    //复制文件到FLASH的文件系统区域
    copy_file_sd2flash(src_dir,dst_dir);
      
    if(res_sd == FR_OK)
    {
      printf("\r\n 所有数据已成功复制到FLASH！！！ \r\n");  
      LED_GREEN;
    }
    else
    {
      printf("\r\n 复制文件到FLASH失败(文件系统部分)，请复位重试！！ \r\n"); 
    }
  }
  else
  {
    printf("\r\n 拷贝数据到FLASH失败(非文件系统部分)，请复位重试！！ \r\n"); 
  }
  
  while(1)
	{
			
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
