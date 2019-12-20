/**
  ******************************************************************************
  * @file    bsp_wwdg.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   窗口看门狗驱动
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 STM32H743 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "./wwdg/bsp_wwdg.h"   
#include "./led/bsp_led.h"

WWDG_HandleTypeDef WWDG_Handle;

// WWDG 中断优先级初始化
static void WWDG_NVIC_Config(void)
{
	HAL_NVIC_SetPriority(WWDG_IRQn,0,0);
	HAL_NVIC_EnableIRQ(WWDG_IRQn);
}

/* WWDG 配置函数
 * tr ：递减计时器的值， 取值范围为：0x7f~0x40，超出范围会直接复位
 * wr ：窗口值，取值范围为：0x7f~0x40
 * prv：预分频器值，取值可以是
 * @arg WWDG_PRESCALER_1: WWDG counter clock = (PCLK1(54MHz)/4096)/1   
	 约13184 76us
 * @arg WWDG_PRESCALER_2: WWDG counter clock = (PCLK1(54MHz)/4096)/2	 
	 约6592Hz 152us
 * @arg WWDG_PRESCALER_4: WWDG counter clock = (PCLK1(54MHz)/4096)/4	 
	 约3296Hz 304us
 * @arg WWDG_PRESCALER_8: WWDG counter clock = (PCLK1(54MHz)/4096)/8   
   约1648Hz	608us
 *      
 * 例：tr = 127(0x7f，tr的最大值)  
 *     wr = 80（0x50, 0x40为最小wr最小值）  
 *	  prv = WWDG_PRESCALER_8
 * 窗口时间为608 * (127-80) = 28.6ms < 刷新窗口 < ~608 * 64 = 38.9ms
 * 也就是说调用WWDG_Config进行这样的配置，若在之后的28.6ms前喂狗，
 * 系统会复位，在38.9ms后没有喂狗，系统也会复位。
 * 需要在刷新窗口的时间内喂狗，系统才不会复位。	
*/
void WWDG_Config(uint8_t tr, uint8_t wr, uint32_t prv)
{		
	// 开启 WWDG 时钟
	__HAL_RCC_WWDG1_CLK_ENABLE();
	// 配置WWDG中断优先级
	WWDG_NVIC_Config();
	// 配置WWDG句柄即寄存器基地址
	WWDG_Handle.Instance = WWDG1;
	// 设置预分频器值
	WWDG_Handle.Init.Prescaler = prv;
	// 设置上窗口值
	WWDG_Handle.Init.Window = wr;	
	// 设置计数器的值
	WWDG_Handle.Init.Counter = tr;
	// 使能提前唤醒中断
	WWDG_Handle.Init.EWIMode = WWDG_EWI_ENABLE;
	// 初始化WWDG
	HAL_WWDG_Init(&WWDG_Handle);	
}

// 喂狗
void WWDG_Feed(void)
{
	// 喂狗，刷新递减计数器的值，设置成最大WDG_CNT=0X7F
	HAL_WWDG_Refresh(&WWDG_Handle);
}

void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef* hwwdg)
{
	//黄灯亮，点亮LED只是示意性的操作，
	//真正使用的时候，这里应该是做最重要的事情
	LED_YELLOW; 
}
/*********************************************END OF FILE**********************/
