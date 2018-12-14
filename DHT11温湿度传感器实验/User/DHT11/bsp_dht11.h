#ifndef __DHT11_H_
#define __DHT11_H_

#include  "stm32h7xx.h"

#define DHT11_HIGH  1
#define DHT11_LOW   0


/*---------------------------------------*/
#define DHT11_CLK     RCC_AHB1Periph_GPIOE
#define DHT11_PIN     GPIO_PIN_2               
#define DHT11_PORT    GPIOE


#define DHT11_DATA_OUT(a)	if (a)	\
                                   HAL_GPIO_WritePin(DHT11_PORT,DHT11_PIN, GPIO_PIN_SET);\
                                   else		\
                                   HAL_GPIO_WritePin(DHT11_PORT,DHT11_PIN, GPIO_PIN_RESET)

#define  DHT11_DATA_IN()	  HAL_GPIO_ReadPin(DHT11_PORT,DHT11_PIN)

typedef struct
{
	uint8_t  humi_int;		//湿度的整数部分
	uint8_t  humi_deci;	 	//湿度的小数部分
	uint8_t  temp_int;	 	//温度的整数部分
	uint8_t  temp_deci;	 	//温度的小数部分
	uint8_t  check_sum;	 	//校验和
}DHT11_Data_TypeDef;

void DHT11_GPIO_Config(void);

uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data);

#endif //__DHT11_H_
