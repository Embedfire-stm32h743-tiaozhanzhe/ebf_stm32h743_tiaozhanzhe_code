#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32h7xx.h"

void SysTick_Init(void);
void Delay_us(__IO uint32_t nTime);
#define Delay_ms(x) Delay_us(1000*x)	 //µ¥Î»ms

#endif /* __SYSTICK_H */
