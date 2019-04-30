/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2016-xx-xx
  * @brief   MPU6050 基本数据读
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火  STM32 F767 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32h7xx.h"
#include "./led/bsp_led.h"
#include "./usart/bsp_usart.h"
#include <stdlib.h>
#include "main.h"
#include "./i2c/i2c.h"
#include "./mpu6050/mpu6050.h"
//设置是否使用LCD进行显示，不需要的话把这个宏注释掉即可
//#define USE_LCD_DISPLAY

#ifdef USE_LCD_DISPLAY
 #include "./lcd/bsp_lcd.h"
#endif

/*简单任务管理*/
uint32_t Task_Delay[3]={0};
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
	static short Acel[3];
	static short Gyro[3];
	static float Temp;
	
    /* 系统时钟初始化成216 MHz */
    SystemClock_Config();
    /* LED 端口初始化 */
    LED_GPIO_Config();
	
#ifdef USE_LCD_DISPLAY		
    /* LCD 端口初始化 */ 
    LCD_Init();
    /* LCD 第一层初始化 */ 
    LCD_LayerInit(0, LCD_FB_START_ADDRESS,ARGB8888);
	/* LCD 第二层初始化 */ 
    LCD_LayerInit(1, LCD_FB_START_ADDRESS+(LCD_GetXSize()*LCD_GetYSize()*4),ARGB8888);
    /* 使能LCD，包括开背光 */ 
    LCD_DisplayOn(); 

    /* 选择LCD第一层 */
    LCD_SelectLayer(0);

    /* 第一层清屏，显示全黑 */ 
    LCD_Clear(LCD_COLOR_BLACK);  

    /* 选择LCD第二层 */
    LCD_SelectLayer(1);

    /* 第二层清屏，显示透明 */ 
    LCD_Clear(LCD_COLOR_TRANSPARENT);

    /* 配置第一和第二层的透明度,最小值为0，最大值为255*/
    LCD_SetTransparency(0, 255);
    LCD_SetTransparency(1, 0);
	
	/* 选择LCD第一层 */
    LCD_SelectLayer(0);
	/*设置字体颜色及字体的背景颜色*/
	LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_BLACK);	
#endif
  /*初始化USART1*/
	UARTx_Config(); 
		
	//初始化 I2C
	I2cMaster_Init(); 

	printf("\r\n 欢迎使用野火  STM32 H743 开发板。\r\n");		 

	printf("\r\n 这是一个I2C外设(MPU6050)读写测试例程 \r\n");

 	//MPU6050初始化
	MPU6050_Init();
	
	//检测MPU6050
	if (MPU6050ReadID() == 1)
	{	
		while(1)
		{
			//if(Task_Delay[0]==0)
//			{
//				LED2_TOGGLE;
//				Task_Delay[0]=1000;
//			}
			
			//if(Task_Delay[1]==0)
			{
				MPU6050ReadAcc(Acel);
				printf("加速度：%8d%8d%8d",Acel[0],Acel[1],Acel[2]);
				MPU6050ReadGyro(Gyro);
				printf("    陀螺仪%8d%8d%8d",Gyro[0],Gyro[1],Gyro[2]);
				MPU6050_ReturnTemp(&Temp);
				printf("    温度%8.2f\r\n",Temp);				
				
				
				#ifdef USE_LCD_DISPLAY	
					{
						char cStr [ 70 ];
						sprintf ( cStr, "Acceleration:%8d%8d%8d",Acel[0],Acel[1],Acel[2] );	//加速度原始数据


						LCD_DisplayStringLine(7,(uint8_t* )cStr);			

						sprintf ( cStr, "Gyro        :%8d%8d%8d",Gyro[0],Gyro[1],Gyro[2] );	//角原始数据

						LCD_DisplayStringLine(8,(uint8_t* )cStr);			

						sprintf ( cStr, "Temperture  :%8.2f",Temp );	//温度值
						LCD_DisplayStringLine(9,(uint8_t* )cStr);			

					}
				#endif
				HAL_Delay(500);
				//Task_Delay[1]=500; //更新一次数据，可根据自己的需求，提高采样频率，如100ms采样一次
				
			}

			//*************************************	下面是增加任务的格式************************************//
	//		if(Task_Delay[i]==0)
	//		{
	//			Task(i);
	//			Task_Delay[i]=;
	//		}

		}

	}
	else
	{
			printf("\r\n没有检测到MPU6050传感器！\r\n");
			LED_RED; 
			#ifdef USE_LCD_DISPLAY			
				/*设置字体颜色及字体的背景颜色*/
				LCD_SetColors(LCD_COLOR_BLUE,LCD_COLOR_BLACK);	

				LCD_DisplayStringLine(4,(uint8_t* )"No MPU6050 detected! ");			//野火自带的17*24显示
				LCD_DisplayStringLine(5,(uint8_t* )"Please check the hardware connection! ");//野火自带的17*24显示

			#endif
		while(1);	
	}
}

/**
  * @brief  System Clock 配置
  *         system Clock 配置如下 : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 216000000
  *            HCLK(Hz)                       = 216000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  无
  * @retval 无
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

/*********************************************END OF FILE**********************/

