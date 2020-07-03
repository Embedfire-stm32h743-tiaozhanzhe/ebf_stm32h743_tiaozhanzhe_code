/**
  ******************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   FMC-SDRAM
  ******************************************************************
  * @attention
  *
  * 实验平台:野火 STM32H50 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************
  */  
#include "stm32h7xx.h"
#include "main.h"
#include "./led/bsp_led.h" 
#include "./usart/bsp_usart.h"
#include "./sdram/bsp_sdram.h" 
#include "./delay/core_delay.h" 
#include "./malloc/malloc.h"
#include "./nand/ftl.h"
#include "./nand/bsp_nand.h"
#include "./key/bsp_key.h" 
#include "./nand/nandtest.h"

void SDRAM_Check(void);
uint32_t RadomBuffer[10000];

uint32_t ReadBuffer[10000];

uint32_t *pSDRAM;

long long count=0,sdram_count=0;
RNG_HandleTypeDef hrng;

/**
  * @brief  延迟一段时间
  * @param  延迟的时间长度
  * @retval None
  */
static void Delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for(index = (100000 * nCount); index != 0; index--)
  {
  }
}
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
uint8_t CNT = 0;
int main(void)
{   
	uint8_t *buf;
	uint8_t *backbuf;	
	
//	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
	
	/* 系统时钟初始化成400MHz */
	SystemClock_Config();
Key_GPIO_Config();
	LED_GPIO_Config();
	/* 配置串口1为：115200 8-N-1 */
	UARTx_Config();	
	
	printf("\r\n 欢迎使用野火  STM32 H743 开发板。\r\n");		 

		
	/*初始化SDRAM模块*/
	SDRAM_Init();
	/*初始化外部内存池*/
	my_mem_init(SRAMEX);		
	/*蓝灯亮，表示正在读写SDRAM测试*/
	LED_BLUE;

	 	while(FTL_Init())			    //检测NAND FLASH,并初始化FTL
	{
		printf("NAND Error!");
		Delay(500);				 
		printf("Please Check");
		Delay(500);				 
		LED1_TOGGLE;//红灯闪烁
	}
	backbuf=mymalloc(SRAMEX,NAND_ECC_SECTOR_SIZE);	//申请一个扇区的缓存
	buf=mymalloc(SRAMIN,NAND_ECC_SECTOR_SIZE);		//申请一个扇区的缓存
	sprintf((char*)buf,"NAND Size:%dMB",(nand_dev.block_totalnum/1024)*(nand_dev.page_mainsize/1024)*nand_dev.block_pagenum);
	printf((char*)buf);	//显示NAND容量  
	FTL_ReadSectors(backbuf,2,NAND_ECC_SECTOR_SIZE,1);//预先读取扇区0到备份区域,防止乱写导致文件系统损坏.
	

CNT = 55;

//test_writepage(9,0,256,CNT);
//test_readpage(9,0,256);

	while(1)
	{
    CNT++;
    if(CNT >= 255)
      CNT = 1;
    if(Key_Scan(KEY1_GPIO_PORT, KEY1_PIN) == KEY_ON)
    {
      /* 写之前需要擦除 */
      NAND_EraseBlock(1);
      test_writepage(1*64,0,215,CNT);
      printf("write: %d\n", CNT);
    }
    
    if(Key_Scan(KEY2_GPIO_PORT, KEY2_PIN) == KEY_ON)
    {
      test_readpage(1*64,0,215);                                  //0x40
    }
	}		
}

void SDRAM_Check(void)
{
    pSDRAM=(uint32_t*)SDRAM_BANK_ADDR;
	count=0;
	printf("开始写入SDRAM\r\n");
	for(sdram_count=0;sdram_count<SDRAM_SIZE/4;sdram_count++)
	{
		*pSDRAM=RadomBuffer[count];
		count++;
		pSDRAM++;
		if(count>=10000)

		{
			count=0;
		}
	}
	printf("写入总字节数:%d\r\n",(uint32_t)pSDRAM-SDRAM_BANK_ADDR);

	count=0;
	pSDRAM=(uint32_t*)SDRAM_BANK_ADDR;
	printf("开始读取SDRAM并与原随机数比较\r\n");
	sdram_count=0;
	for(;sdram_count<SDRAM_SIZE/4;sdram_count++)
	{
		if(*pSDRAM != RadomBuffer[count])
		{
			printf("数据比较错误——退出~\r\n");
			break;
		}
		count++;
		pSDRAM++;
		if(count>=10000)
		{
			count=0;
		}
	}

	printf("比较通过总字节数:%d\r\n",(uint32_t)pSDRAM-SDRAM_BANK_ADDR);

	if(sdram_count == SDRAM_SIZE/4)
	{
		LED_GREEN;
		printf("SDRAM测试成功\r\n");
	}
	else
	{
		LED_RED;
		printf("SDRAM测试失败\r\n");
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
  RCC_OscInitStruct.PLL.PLLQ = 2;
 
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
