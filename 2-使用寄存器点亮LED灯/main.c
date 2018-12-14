
/*
	使用寄存器的方法点亮LED灯
  */
#include  "stm32h743xx.h" 


/**
  *   主函数
  */
int main(void)
{	
	/*开启 GPIOH 时钟，使用外设时都要先开启它的时钟*/
	RCC_AHB4ENR |= (1<<7);	
	
	/* LED 端口初始化 */
	
	/*GPIOH MODER10清空*/
	GPIOH_MODER  &= ~( 0x03<< (2*10));	
	/*PH10 MODER10 = 01b 输出模式*/
	GPIOH_MODER |= (1<<2*10);
	
	/*GPIOH OTYPER10清空*/
	GPIOH_OTYPER &= ~(1<<1*10);
	/*PH10 OTYPER10 = 0b 推挽模式*/
	GPIOH_OTYPER |= (0<<1*10);
	
	/*GPIOH OSPEEDR10清空*/
	GPIOH_OSPEEDR &= ~(0x03<<2*10);
	/*PH10 OSPEEDR10 = 0b 速率2MHz*/
	GPIOH_OSPEEDR |= (0<<2*10);
	
	/*GPIOH PUPDR10清空*/
	GPIOH_PUPDR &= ~(0x03<<2*10);
	/*PH10 PUPDR10 = 01b 上拉模式*/
	GPIOH_PUPDR |= (1<<2*10);
	
	/*PH10 BSRR寄存器的 BR10置1，使引脚输出低电平*/
	GPIOH_BSRRH |= (1<<10);
	
	/*PH10 BSRR寄存器的 BS10置1，使引脚输出高电平*/
	//GPIOH_BSRRL |= (1<<10);

	while(1);

}

// 函数为空，目的是为了骗过编译器不报错
void SystemInit(void)
{	
}






/*********************************************END OF FILE**********************/

