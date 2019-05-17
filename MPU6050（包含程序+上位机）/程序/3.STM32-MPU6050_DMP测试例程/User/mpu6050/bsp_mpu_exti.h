#ifndef __MPU_EXTI_H
#define	__MPU_EXTI_H

#include "stm32f7xx.h"


#define MPU_INT_GPIO_PORT                GPIOI
#define MPU_INT_GPIO_CLK_ENABLE()        __GPIOI_CLK_ENABLE()
#define MPU_INT_GPIO_PIN                 GPIO_PIN_1
#define MPU_INT_EXTI_IRQ                 EXTI1_IRQn

#define MPU_IRQHandler                   EXTI1_IRQHandler

void EXTI_MPU_Config(void);

#endif /* __EXTI_H */
