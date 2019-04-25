/*
******************************************************************************
* @file    bsp_led.c
* @author  fire
* @version V1.0
* @date    2015-xx-xx
* @brief   wm8978驱动
******************************************************************************
* @attention
*
* 实验平台:秉火  STM32 F767 开发板  
* 论坛    :http://www.chuxue123.com
* 淘宝    :http://firestm32.taobao.com
*
******************************************************************************
*/
#include "./wm8978/bsp_wm8978.h"  
#include "./usart/bsp_debug_usart.h"
#include "main.h"
#include "./i2c/i2c.h"
#include "./Recorder/Recorder.h"
#include "./delay/core_delay.h" 
extern I2C_HandleTypeDef I2C_Handle;
uint32_t AudioTotalSize ;         /* 音频文件的总大小 */
uint32_t AudioRemSize;            /* 将剩余的数据保存在音频文件中 */
uint16_t *CurrentPos;             /* 音频数据指针的当前位置 */

I2S_HandleTypeDef I2S_InitStructure;
I2S_HandleTypeDef I2Sext_InitStructure;
DMA_HandleTypeDef hdma_spi2_tx;
DMA_HandleTypeDef hdma_spi2_rx;
/**
	*******************************************************************************************************
	*	                     I2C控制WM8978配置部分 
	*******************************************************************************************************
	*/
static uint8_t WM8978_I2C_WriteRegister(uint8_t RegisterAddr, uint16_t RegisterValue);
static uint16_t wm8978_ReadReg(uint8_t _ucRegAddr);
static uint8_t wm8978_WriteReg(uint8_t _ucRegAddr, uint16_t _usValue);
/*
	wm8978寄存器缓存
	由于WM8978的I2C两线接口不支持读取操作，因此寄存器值缓存在内存中，当写寄存器时同步更新缓存，读寄存器时
	直接返回缓存中的值。
	寄存器MAP 在WM8978(V4.5_2011).pdf 的第89页，寄存器地址是7bit， 寄存器数据是9bit
*/
static uint16_t wm8978_RegCash[] = {
	0x000, 0x000, 0x000, 0x000, 0x050, 0x000, 0x140, 0x000,
	0x000, 0x000, 0x000, 0x0FF, 0x0FF, 0x000, 0x100, 0x0FF,
	0x0FF, 0x000, 0x12C, 0x02C, 0x02C, 0x02C, 0x02C, 0x000,
	0x032, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x038, 0x00B, 0x032, 0x000, 0x008, 0x00C, 0x093, 0x0E9,
	0x000, 0x000, 0x000, 0x000, 0x003, 0x010, 0x010, 0x100,
	0x100, 0x002, 0x001, 0x001, 0x039, 0x039, 0x039, 0x039,
	0x001, 0x001
};
/**
  * @brief  通过I2C将给定寄存器的字节写入音频编解码器
  * @param  RegisterAddr: 待写入寄存器的地址
  * @param  RegisterValue: 要写入目标寄存器的字节值
  * @retval 通信成功返回1，失败返回0
  */
static uint8_t WM8978_I2C_WriteRegister(uint8_t RegisterAddr, uint16_t RegisterValue)
{	
//	uint16_t tmp;

//	tmp  = (RegisterValue&0xff) << 8;
//	tmp |= ((RegisterAddr << 1) & 0xFE) | ((RegisterValue >> 8) & 0x1);
  
  uint8_t tmp[2];

	tmp[1]  = (RegisterValue&0xff);
	tmp[0] = ((RegisterAddr << 1) & 0xFE) | ((RegisterValue >> 8) & 0x1);
  
	if(HAL_I2C_Master_Transmit(&I2C_Handle,WM8978_SLAVE_ADDRESS,(uint8_t *)&tmp,2,WM8978_I2C_FLAG_TIMEOUT)==HAL_OK)
	{
		return 1; 
	}
	else 		
	return 0;  
}

/**
	* @brief  从cash中读回读回wm8978寄存器
	* @param  _ucRegAddr ： 寄存器地址
	* @retval 寄存器值
	*/
static uint16_t wm8978_ReadReg(uint8_t _ucRegAddr)
{
	return wm8978_RegCash[_ucRegAddr];
}

/**
	* @brief  写wm8978寄存器
	* @param  _ucRegAddr： 寄存器地址
	* @param  _usValue： 寄存器值
	* @retval 0：写入失败
    *         1：写入成功
	*/
static uint8_t wm8978_WriteReg(uint8_t _ucRegAddr, uint16_t _usValue)
{
	uint8_t res;
	res=WM8978_I2C_WriteRegister(_ucRegAddr,_usValue);
	wm8978_RegCash[_ucRegAddr] = _usValue;
	return res;
}

/**
	* @brief  配置I2C GPIO，并检查I2C总线上的WM8978是否正常
	* @param  无
	* @retval 1,初始化成功;
	*         0,初始化失败。
	*/
uint8_t wm8978_Init(void)
{
	uint8_t res;
	
	I2cMaster_Init();		/* 初始化I2C接口 */
	res=wm8978_Reset();		/* 硬件复位WM8978所有寄存器到缺省状态 */
	wm8978_CtrlGPIO1(1);	/* 控制WM8978的一个GPIO接口控制其为放音状态 */
	return res;
}

/**
	* @brief  修改输出通道1音量
	* @param  _ucVolume ：音量值, 0-63
	* @retval 无
	*/
void wm8978_SetOUT1Volume(uint8_t _ucVolume)
{
	uint16_t regL;
	uint16_t regR;

	if (_ucVolume > 0x3F)
	{
		_ucVolume = 0x3F;
	}

	regL = _ucVolume;
	regR = _ucVolume;

	/*
		R52	LOUT1 Volume control
		R53	ROUT1 Volume control
	*/
	/* 先更新左声道缓存值 */
	wm8978_WriteReg(52, regL | 0x00);

	/* 再同步更新左右声道的音量 */
	wm8978_WriteReg(53, regR | 0x100);	/* 0x180表示 在音量为0时再更新，避免调节音量出现的“嘎哒”声 */
}

/**
	* @brief  修改输出通道2音量
	* @param  _ucVolume ：音量值, 0-63
	* @retval 无
	*/
void wm8978_SetOUT2Volume(uint8_t _ucVolume)
{
	uint16_t regL;
	uint16_t regR;

	if (_ucVolume > 0x3F)
	{
		_ucVolume = 0x3F;
	}

	regL = _ucVolume;
	regR = _ucVolume;

	/*
		R54	LOUT2 (SPK) Volume control
		R55	ROUT2 (SPK) Volume control
	*/
	/* 先更新左声道缓存值 */
	wm8978_WriteReg(54, regL | 0x00);

	/* 再同步更新左右声道的音量 */
	wm8978_WriteReg(55, regR | 0x100);	/* 在音量为0时再更新，避免调节音量出现的“嘎哒”声 */
}

/**
	* @brief  读取输出通道1音量
	* @param  无
	* @retval 当前音量值
	*/
uint8_t wm8978_ReadOUT1Volume(void)
{
	return (uint8_t)(wm8978_ReadReg(52) & 0x3F );
}

/**
	* @brief  读取输出通道2音量
	* @param  无
	* @retval 当前音量值
	*/
uint8_t wm8978_ReadOUT2Volume(void)
{
	return (uint8_t)(wm8978_ReadReg(54) & 0x3F );
}

/**
	* @brief  输出静音.
	* @param  _ucMute：模式选择
	*         @arg 1：静音
	*         @arg 0：取消静音
	* @retval 无
	*/
void wm8978_OutMute(uint8_t _ucMute)
{
	uint16_t usRegValue;

	if (_ucMute == 1) /* 静音 */
	{
		usRegValue = wm8978_ReadReg(52); /* Left Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(52, usRegValue);

		usRegValue = wm8978_ReadReg(53); /* Left Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(53, usRegValue);

		usRegValue = wm8978_ReadReg(54); /* Right Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(54, usRegValue);

		usRegValue = wm8978_ReadReg(55); /* Right Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(55, usRegValue);
	}
	else	/* 取消静音 */
	{
		usRegValue = wm8978_ReadReg(52);
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(52, usRegValue);

		usRegValue = wm8978_ReadReg(53); /* Left Mixer Control */
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(53, usRegValue);

		usRegValue = wm8978_ReadReg(54);
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(54, usRegValue);

		usRegValue = wm8978_ReadReg(55); /* Left Mixer Control */
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(55, usRegValue);
	}
}

/**
	* @brief  设置增益
	* @param  _ucGain ：增益值, 0-63
	* @retval 无
	*/
void wm8978_SetMicGain(uint8_t _ucGain)
{
	if (_ucGain > GAIN_MAX)
	{
		_ucGain = GAIN_MAX;
	}

	/* PGA 音量控制  R45， R46   WM8978(V4.5_2011).pdf 25页
		Bit8	INPPGAUPDATE
		Bit7	INPPGAZCL		过零再更改
		Bit6	INPPGAMUTEL		PGA静音
		Bit5:0	增益值，010000是0dB
	*/
	wm8978_WriteReg(45, _ucGain);
	wm8978_WriteReg(46, _ucGain | (1 << 8));
}

/**
	* @brief  设置Line输入通道的增益
	* @param  _ucGain ：音量值, 0-7. 7最大，0最小。 可衰减可放大。
	* @retval 无
	*/
void wm8978_SetLineGain(uint8_t _ucGain)
{
	uint16_t usRegValue;

	if (_ucGain > 7)
	{
		_ucGain = 7;
	}

	/*
		Mic 输入信道的增益由 PGABOOSTL 和 PGABOOSTR 控制
		Aux 输入信道的输入增益由 AUXL2BOOSTVO[2:0] 和 AUXR2BOOSTVO[2:0] 控制
		Line 输入信道的增益由 LIP2BOOSTVOL[2:0] 和 RIP2BOOSTVOL[2:0] 控制
	*/
	/*	WM8978(V4.5_2011).pdf 29页，R47（左声道），R48（右声道）, MIC 增益控制寄存器
		R47 (R48定义与此相同)
		B8		PGABOOSTL	= 1,   0表示MIC信号直通无增益，1表示MIC信号+20dB增益（通过自举电路）
		B7		= 0， 保留
		B6:4	L2_2BOOSTVOL = x， 0表示禁止，1-7表示增益-12dB ~ +6dB  （可以衰减也可以放大）
		B3		= 0， 保留
		B2:0`	AUXL2BOOSTVOL = x，0表示禁止，1-7表示增益-12dB ~ +6dB  （可以衰减也可以放大）
	*/

	usRegValue = wm8978_ReadReg(47);
	usRegValue &= 0x8F;/* 将Bit6:4清0   1000 1111*/
	usRegValue |= (_ucGain << 4);
	wm8978_WriteReg(47, usRegValue);	/* 写左声道输入增益控制寄存器 */

	usRegValue = wm8978_ReadReg(48);
	usRegValue &= 0x8F;/* 将Bit6:4清0   1000 1111*/
	usRegValue |= (_ucGain << 4);
	wm8978_WriteReg(48, usRegValue);	/* 写右声道输入增益控制寄存器 */
}

/**
	* @brief  关闭wm8978，进入低功耗模式
	* @param  无
	* @retval 无
	*/
void wm8978_PowerDown(void)
{
	wm8978_Reset();			/* 硬件复位WM8978所有寄存器到缺省状态 */
}

/**
	* @brief  配置WM8978的音频接口(I2S)
	* @param  _usStandard : 接口标准，I2S_Standard_Phillips, I2S_Standard_MSB 或 I2S_Standard_LSB
	* @param  _ucWordLen : 字长，16、24、32  （丢弃不常用的20bit格式）
	* @retval 无
	*/
void wm8978_CfgAudioIF(uint16_t _usStandard, uint8_t _ucWordLen)
{
	uint16_t usReg;

	/* WM8978(V4.5_2011).pdf 73页，寄存器列表 */

	/*	REG R4, 音频接口控制寄存器
		B8		BCP	 = X, BCLK极性，0表示正常，1表示反相
		B7		LRCP = x, LRC时钟极性，0表示正常，1表示反相
		B6:5	WL = x， 字长，00=16bit，01=20bit，10=24bit，11=32bit （右对齐模式只能操作在最大24bit)
		B4:3	FMT = x，音频数据格式，00=右对齐，01=左对齐，10=I2S格式，11=PCM
		B2		DACLRSWAP = x, 控制DAC数据出现在LRC时钟的左边还是右边
		B1 		ADCLRSWAP = x，控制ADC数据出现在LRC时钟的左边还是右边
		B0		MONO	= 0，0表示立体声，1表示单声道，仅左声道有效
	*/
	usReg = 0;
	if (_usStandard == I2S_STANDARD_PHILIPS)	/* I2S飞利浦标准 */
	{
		usReg |= (2 << 3);
	}
	else if (_usStandard == I2S_STANDARD_MSB)	/* MSB对齐标准(左对齐) */
	{
		usReg |= (1 << 3);
	}
	else if (_usStandard == I2S_STANDARD_LSB)	/* LSB对齐标准(右对齐) */
	{
		usReg |= (0 << 3);
	}
	else	/* PCM标准(16位通道帧上带长或短帧同步或者16位数据帧扩展为32位通道帧) */
	{
		usReg |= (3 << 3);;
	}

	if (_ucWordLen == 24)
	{
		usReg |= (2 << 5);
	}
	else if (_ucWordLen == 32)
	{
		usReg |= (3 << 5);
	}
	else
	{
		usReg |= (0 << 5);		/* 16bit */
	}
	wm8978_WriteReg(4, usReg);

	/*
		R6，时钟产生控制寄存器
		MS = 0,  WM8978被动时钟，由MCU提供MCLK时钟
	*/
	wm8978_WriteReg(6, 0x000);
}


/**
	* @brief  配置wm8978音频通道
	* @param  _InPath : 音频输入通道配置
	* @param  _OutPath : 音频输出通道配置
	* @retval 无
	*/
void wm8978_CfgAudioPath(uint16_t _InPath, uint16_t _OutPath)
{
	uint16_t usReg;

	/* 查看WM8978数据手册的 REGISTER MAP 章节， 第89页 */

	if ((_InPath == IN_PATH_OFF) && (_OutPath == OUT_PATH_OFF))
	{
		wm8978_PowerDown();
		return;
	}

	/* --------------------------- 第1步：根据输入通道参数配置寄存器 -----------------------*/
	/*
		R1 寄存器 Power manage 1
		Bit8    BUFDCOPEN,  Output stage 1.5xAVDD/2 driver enable
 		Bit7    OUT4MIXEN, OUT4 mixer enable
		Bit6    OUT3MIXEN, OUT3 mixer enable
		Bit5    PLLEN	.不用
		Bit4    MICBEN	,Microphone Bias Enable (MIC偏置电路使能)
		Bit3    BIASEN	,Analogue amplifier bias control 必须设置为1模拟放大器才工作
		Bit2    BUFIOEN , Unused input/output tie off buffer enable
		Bit1:0  VMIDSEL, 必须设置为非00值模拟放大器才工作
	*/
	usReg = (1 << 3) | (3 << 0);
	if (_OutPath & OUT3_4_ON) 	/* OUT3和OUT4使能输出到GSM模块 */
	{
		usReg |= ((1 << 7) | (1 << 6));
	}
	if ((_InPath & MIC_LEFT_ON) || (_InPath & MIC_RIGHT_ON))
	{
		usReg |= (1 << 4);
	}
	wm8978_WriteReg(1, usReg);	/* 写寄存器 */

	/*
		R2 寄存器 Power manage 2
		Bit8	ROUT1EN,	ROUT1 output enable 耳机右声道输出使能
		Bit7	LOUT1EN,	LOUT1 output enable 耳机左声道输出使能
		Bit6	SLEEP, 		0 = Normal device operation   1 = Residual current reduced in device standby mode
		Bit5	BOOSTENR,	Right channel Input BOOST enable 输入通道自举电路使能. 用到PGA放大功能时必须使能
		Bit4	BOOSTENL,	Left channel Input BOOST enable
		Bit3	INPGAENR,	Right channel input PGA enable 右声道输入PGA使能
		Bit2	INPGAENL,	Left channel input PGA enable
		Bit1	ADCENR,		Enable ADC right channel
		Bit0	ADCENL,		Enable ADC left channel
	*/
	usReg = 0;
	if (_OutPath & EAR_LEFT_ON)
	{
		usReg |= (1 << 7);
	}
	if (_OutPath & EAR_RIGHT_ON)
	{
		usReg |= (1 << 8);
	}
	if (_InPath & MIC_LEFT_ON)
	{
		usReg |= ((1 << 4) | (1 << 2));
	}
	if (_InPath & MIC_RIGHT_ON)
	{
		usReg |= ((1 << 5) | (1 << 3));
	}
	if (_InPath & LINE_ON)
	{
		usReg |= ((1 << 4) | (1 << 5));
	}
	if (_InPath & MIC_RIGHT_ON)
	{
		usReg |= ((1 << 5) | (1 << 3));
	}
	if (_InPath & ADC_ON)
	{
		usReg |= ((1 << 1) | (1 << 0));
	}
	wm8978_WriteReg(2, usReg);	/* 写寄存器 */

	/*
		R3 寄存器 Power manage 3
		Bit8	OUT4EN,		OUT4 enable
		Bit7	OUT3EN,		OUT3 enable
		Bit6	LOUT2EN,	LOUT2 output enable
		Bit5	ROUT2EN,	ROUT2 output enable
		Bit4	0,
		Bit3	RMIXEN,		Right mixer enable
		Bit2	LMIXEN,		Left mixer enable
		Bit1	DACENR,		Right channel DAC enable
		Bit0	DACENL,		Left channel DAC enable
	*/
	usReg = 0;
	if (_OutPath & OUT3_4_ON)
	{
		usReg |= ((1 << 8) | (1 << 7));
	}
	if (_OutPath & SPK_ON)
	{
		usReg |= ((1 << 6) | (1 << 5));
	}
	if (_OutPath != OUT_PATH_OFF)
	{
		usReg |= ((1 << 3) | (1 << 2));
	}
	if (_InPath & DAC_ON)
	{
		usReg |= ((1 << 1) | (1 << 0));
	}
	wm8978_WriteReg(3, usReg);	/* 写寄存器 */

	/*
		R44 寄存器 Input ctrl

		Bit8	MBVSEL, 		Microphone Bias Voltage Control   0 = 0.9 * AVDD   1 = 0.6 * AVDD
		Bit7	0
		Bit6	R2_2INPPGA,		Connect R2 pin to right channel input PGA positive terminal
		Bit5	RIN2INPPGA,		Connect RIN pin to right channel input PGA negative terminal
		Bit4	RIP2INPPGA,		Connect RIP pin to right channel input PGA amplifier positive terminal
		Bit3	0
		Bit2	L2_2INPPGA,		Connect L2 pin to left channel input PGA positive terminal
		Bit1	LIN2INPPGA,		Connect LIN pin to left channel input PGA negative terminal
		Bit0	LIP2INPPGA,		Connect LIP pin to left channel input PGA amplifier positive terminal
	*/
	usReg = 0 << 8;
	if (_InPath & LINE_ON)
	{
		usReg |= ((1 << 6) | (1 << 2));
	}
	if (_InPath & MIC_RIGHT_ON)
	{
		usReg |= ((1 << 5) | (1 << 4));
	}
	if (_InPath & MIC_LEFT_ON)
	{
		usReg |= ((1 << 1) | (1 << 0));
	}
	wm8978_WriteReg(44, usReg);	/* 写寄存器 */


	/*
		R14 寄存器 ADC Control
		设置高通滤波器（可选的） WM8978(V4.5_2011).pdf 31 32页,
		Bit8 	HPFEN,	High Pass Filter Enable高通滤波器使能，0表示禁止，1表示使能
		BIt7 	HPFAPP,	Select audio mode or application mode 选择音频模式或应用模式，0表示音频模式，
		Bit6:4	HPFCUT，Application mode cut-off frequency  000-111选择应用模式的截止频率
		Bit3 	ADCOSR,	ADC oversample rate select: 0=64x (lower power) 1=128x (best performance)
		Bit2   	0
		Bit1 	ADC right channel polarity adjust:  0=normal  1=inverted
		Bit0 	ADC left channel polarity adjust:  0=normal 1=inverted
	*/
	if (_InPath & ADC_ON)
	{
		usReg = (1 << 3) | (0 << 8) | (4 << 0);		/* 禁止ADC高通滤波器, 设置截至频率 */
	}
	else
	{
		usReg = 0;
	}
	wm8978_WriteReg(14, usReg);	/* 写寄存器 */

	/* 设置陷波滤波器（notch filter），主要用于抑制话筒声波正反馈，避免啸叫.  暂时关闭
		R27，R28，R29，R30 用于控制限波滤波器，WM8978(V4.5_2011).pdf 33页
		R7的 Bit7 NFEN = 0 表示禁止，1表示使能
	*/
	if (_InPath & ADC_ON)
	{
		usReg = (0 << 7);
		wm8978_WriteReg(27, usReg);	/* 写寄存器 */
		usReg = 0;
		wm8978_WriteReg(28, usReg);	/* 写寄存器,填0，因为已经禁止，所以也可不做 */
		wm8978_WriteReg(29, usReg);	/* 写寄存器,填0，因为已经禁止，所以也可不做 */
		wm8978_WriteReg(30, usReg);	/* 写寄存器,填0，因为已经禁止，所以也可不做 */
	}

	/* 自动增益控制 ALC, R32  - 34  WM8978(V4.5_2011).pdf 36页 */
	{
		usReg = 0;		/* 禁止自动增益控制 */
		wm8978_WriteReg(32, usReg);
		wm8978_WriteReg(33, usReg);
		wm8978_WriteReg(34, usReg);
	}

	/*  R35  ALC Noise Gate Control
		Bit3	NGATEN, Noise gate function enable
		Bit2:0	Noise gate threshold:
	*/
	usReg = (3 << 1) | (7 << 0);		/* 禁止自动增益控制 */
	wm8978_WriteReg(35, usReg);

	/*
		Mic 输入信道的增益由 PGABOOSTL 和 PGABOOSTR 控制
		Aux 输入信道的输入增益由 AUXL2BOOSTVO[2:0] 和 AUXR2BOOSTVO[2:0] 控制
		Line 输入信道的增益由 LIP2BOOSTVOL[2:0] 和 RIP2BOOSTVOL[2:0] 控制
	*/
	/*	WM8978(V4.5_2011).pdf 29页，R47（左声道），R48（右声道）, MIC 增益控制寄存器
		R47 (R48定义与此相同)
		B8		PGABOOSTL	= 1,   0表示MIC信号直通无增益，1表示MIC信号+20dB增益（通过自举电路）
		B7		= 0， 保留
		B6:4	L2_2BOOSTVOL = x， 0表示禁止，1-7表示增益-12dB ~ +6dB  （可以衰减也可以放大）
		B3		= 0， 保留
		B2:0`	AUXL2BOOSTVOL = x，0表示禁止，1-7表示增益-12dB ~ +6dB  （可以衰减也可以放大）
	*/
	usReg = 0;
	if ((_InPath & MIC_LEFT_ON) || (_InPath & MIC_RIGHT_ON))
	{
		usReg |= (1 << 8);	/* MIC增益取+20dB */
	}
	if (_InPath & AUX_ON)
	{
		usReg |= (3 << 0);	/* Aux增益固定取3，用户可以自行调整 */
	}
	if (_InPath & LINE_ON)
	{
		usReg |= (3 << 4);	/* Line增益固定取3，用户可以自行调整 */
	}
	wm8978_WriteReg(47, usReg);	/* 写左声道输入增益控制寄存器 */
	wm8978_WriteReg(48, usReg);	/* 写右声道输入增益控制寄存器 */

	/* 数字ADC音量控制，pdf 35页
		R15 控制左声道ADC音量，R16控制右声道ADC音量
		Bit8 	ADCVU  = 1 时才更新，用于同步更新左右声道的ADC音量
		Bit7:0 	增益选择； 0000 0000 = 静音
						   0000 0001 = -127dB
						   0000 0010 = -12.5dB  （0.5dB 步长）
						   1111 1111 = 0dB  （不衰减）
	*/
	usReg = 0xFF;
	wm8978_WriteReg(15, usReg);	/* 选择0dB，先缓存左声道 */
	usReg = 0x1FF;
	wm8978_WriteReg(16, usReg);	/* 同步更新左右声道 */

	/* 通过 wm8978_SetMicGain 函数设置mic PGA增益 */

	/*	R43 寄存器  AUXR – ROUT2 BEEP Mixer Function
		B8:6 = 0

		B5	 MUTERPGA2INV,	Mute input to INVROUT2 mixer
		B4	 INVROUT2,  Invert ROUT2 output 用于扬声器推挽输出
		B3:1 BEEPVOL = 7;	AUXR input to ROUT2 inverter gain
		B0	 BEEPEN = 1;	Enable AUXR beep input

	*/
	usReg = 0;
	if (_OutPath & SPK_ON)
	{
		usReg |= (1 << 4);	/* ROUT2 反相, 用于驱动扬声器 */
	}
	if (_InPath & AUX_ON)
	{
		usReg |= ((7 << 1) | (1 << 0));
	}
	wm8978_WriteReg(43, usReg);

	/* R49  Output ctrl
		B8:7	0
		B6		DACL2RMIX,	Left DAC output to right output mixer
		B5		DACR2LMIX,	Right DAC output to left output
		B4		OUT4BOOST,	0 = OUT4 output gain = -1; DC = AVDD / 2；1 = OUT4 output gain = +1.5；DC = 1.5 x AVDD / 2
		B3		OUT3BOOST,	0 = OUT3 output gain = -1; DC = AVDD / 2；1 = OUT3 output gain = +1.5；DC = 1.5 x AVDD / 2
		B2		SPKBOOST,	0 = Speaker gain = -1;  DC = AVDD / 2 ; 1 = Speaker gain = +1.5; DC = 1.5 x AVDD / 2
		B1		TSDEN,   Thermal Shutdown Enable  扬声器热保护使能（缺省1）
		B0		VROI,	Disabled Outputs to VREF Resistance
	*/
	usReg = 0;
	if (_InPath & DAC_ON)
	{
		usReg |= ((1 << 6) | (1 << 5));
	}
	if (_OutPath & SPK_ON)
	{
		usReg |=  ((1 << 2) | (1 << 1));	/* SPK 1.5x增益,  热保护使能 */
	}
	if (_OutPath & OUT3_4_ON)
	{
		usReg |=  ((1 << 4) | (1 << 3));	/* BOOT3  BOOT4  1.5x增益 */
	}
	wm8978_WriteReg(49, usReg);

	/*	REG 50    (50是左声道，51是右声道，配置寄存器功能一致) WM8978(V4.5_2011).pdf 56页
		B8:6	AUXLMIXVOL = 111	AUX用于FM收音机信号输入
		B5		AUXL2LMIX = 1		Left Auxilliary input to left channel
		B4:2	BYPLMIXVOL			音量
		B1		BYPL2LMIX = 0;		Left bypass path (from the left channel input boost output) to left output mixer
		B0		DACL2LMIX = 1;		Left DAC output to left output mixer
	*/
	usReg = 0;
	if (_InPath & AUX_ON)
	{
		usReg |= ((7 << 6) | (1 << 5));
	}
	if ((_InPath & LINE_ON) || (_InPath & MIC_LEFT_ON) || (_InPath & MIC_RIGHT_ON))
	{
		usReg |= ((7 << 2) | (1 << 1));
	}
	if (_InPath & DAC_ON)
	{
		usReg |= (1 << 0);
	}
	wm8978_WriteReg(50, usReg);
	wm8978_WriteReg(51, usReg);

	/*	R56 寄存器   OUT3 mixer ctrl
		B8:7	0
		B6		OUT3MUTE,  	0 = Output stage outputs OUT3 mixer;  1 = Output stage muted – drives out VMID.
		B5:4	0
		B3		BYPL2OUT3,	OUT4 mixer output to OUT3  (反相)
		B4		0
		B2		LMIX2OUT3,	Left ADC input to OUT3
		B1		LDAC2OUT3,	Left DAC mixer to OUT3
		B0		LDAC2OUT3,	Left DAC output to OUT3
	*/
	usReg = 0;
	if (_OutPath & OUT3_4_ON)
	{
		usReg |= (1 << 3);
	}
	wm8978_WriteReg(56, usReg);

	/* R57 寄存器		OUT4 (MONO) mixer ctrl
		B8:7	0
		B6		OUT4MUTE,	0 = Output stage outputs OUT4 mixer  1 = Output stage muted – drives outVMID.
		B5		HALFSIG,	0 = OUT4 normal output	1 = OUT4 attenuated by 6dB
		B4		LMIX2OUT4,	Left DAC mixer to OUT4
		B3		LDAC2UT4,	Left DAC to OUT4
		B2		BYPR2OUT4,	Right ADC input to OUT4
		B1		RMIX2OUT4,	Right DAC mixer to OUT4
		B0		RDAC2OUT4,	Right DAC output to OUT4
	*/
	usReg = 0;
	if (_OutPath & OUT3_4_ON)
	{
		usReg |= ((1 << 4) |  (1 << 1));
	}
	wm8978_WriteReg(57, usReg);


	/* R11, 12 寄存器 DAC数字音量
		R11		Left DAC Digital Volume
		R12		Right DAC Digital Volume
	*/
	if (_InPath & DAC_ON)
	{
		wm8978_WriteReg(11, 255);
		wm8978_WriteReg(12, 255 | 0x100);
	}
	else
	{
		wm8978_WriteReg(11, 0);
		wm8978_WriteReg(12, 0 | 0x100);
	}

	/*	R10 寄存器 DAC Control
		B8	0
		B7	0
		B6	SOFTMUTE,	Softmute enable:
		B5	0
		B4	0
		B3	DACOSR128,	DAC oversampling rate: 0=64x (lowest power) 1=128x (best performance)
		B2	AMUTE,		Automute enable
		B1	DACPOLR,	Right DAC output polarity
		B0	DACPOLL,	Left DAC output polarity:
	*/
	if (_InPath & DAC_ON)
	{
		wm8978_WriteReg(10, 0);
	}
	;
}

/**
	* @brief  设置陷波滤波器（notch filter），主要用于抑制话筒声波正反馈，避免啸叫
	* @param  NFA0[13:0] and NFA1[13:0]
	* @retval 无
	*/
void wm8978_NotchFilter(uint16_t _NFA0, uint16_t _NFA1)
{
	uint16_t usReg;

	/*  page 26
		A programmable notch filter is provided. This filter has a variable centre frequency and bandwidth,
		programmable via two coefficients, a0 and a1. a0 and a1 are represented by the register bits
		NFA0[13:0] and NFA1[13:0]. Because these coefficient values require four register writes to setup
		there is an NFU (Notch Filter Update) flag which should be set only when all four registers are setup.
	*/
	usReg = (1 << 7) | (_NFA0 & 0x3F);
	wm8978_WriteReg(27, usReg);	/* 写寄存器 */

	usReg = ((_NFA0 >> 7) & 0x3F);
	wm8978_WriteReg(28, usReg);	/* 写寄存器 */

	usReg = (_NFA1 & 0x3F);
	wm8978_WriteReg(29, usReg);	/* 写寄存器 */

	usReg = (1 << 8) | ((_NFA1 >> 7) & 0x3F);
	wm8978_WriteReg(30, usReg);	/* 写寄存器 */
}


/**
	* @brief  控制WM8978的GPIO1引脚输出0或1，
	*		  控制模拟开关来切换录音放音
	*		  1：放音
	*		  0：录音
	* @param  _ucValue ：GPIO1输出值，0或1
	* @retval 无
	*/
void wm8978_CtrlGPIO1(uint8_t _ucValue)
{
	uint16_t usRegValue;

	/* R8， pdf 62页 */
	if (_ucValue == 0) /* 输出0 */
	{
		usRegValue = 6; /* B2:0 = 110 */
	}
	else
	{
		usRegValue = 7; /* B2:0 = 111 */
	}
	wm8978_WriteReg(8, usRegValue);
}

/**
	* @brief  复位wm8978，所有的寄存器值恢复到缺省值
	* @param  无
	* @retval 1: 复位成功
	* 				0：复位失败
	*/
uint8_t wm8978_Reset(void)
{
	/* wm8978寄存器缺省值 */
	const uint16_t reg_default[] = {
	0x000, 0x000, 0x000, 0x000, 0x050, 0x000, 0x140, 0x000,
	0x000, 0x000, 0x000, 0x0FF, 0x0FF, 0x000, 0x100, 0x0FF,
	0x0FF, 0x000, 0x12C, 0x02C, 0x02C, 0x02C, 0x02C, 0x000,
	0x032, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x038, 0x00B, 0x032, 0x000, 0x008, 0x00C, 0x093, 0x0E9,
	0x000, 0x000, 0x000, 0x000, 0x003, 0x010, 0x010, 0x100,
	0x100, 0x002, 0x001, 0x001, 0x039, 0x039, 0x039, 0x039,
	0x001, 0x001
	};
	uint8_t res;
	uint8_t i;

	res=wm8978_WriteReg(0x00, 0);

	for (i = 0; i < sizeof(reg_default) / 2; i++)
	{
		wm8978_RegCash[i] = reg_default[i];
	}
	return res;
}

/**
	*******************************************************************************************************
	*	                     下面的代码是和STM32 I2S硬件相关的
	*******************************************************************************************************
	*/

/*  I2S DMA回调函数指针  */
void (*I2S_DMA_TX_Callback)(void);	//I2S DMA 回调函数
void (*I2S_DMA_RX_Callback)(void);	//I2S DMA RX回调函数

/**
	* @brief  配置GPIO引脚用于codec应用
	* @param  无
	* @retval 无
	*/
void I2S_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

/**
	* I2S总线传输音频数据口线
	* WM8978_LRC    -> PB12/I2S2_WS
	* WM8978_BCLK   -> PD3/I2S2_CK
	* WM8978_ADCDAT -> PC2/I2S2_SD
	* WM8978_DACDAT -> PI3/I2S2_SD
	* WM8978_MCLK   -> PC6/I2S2_MCK
	*/	
	/* Enable GPIO clock */
	WM8978_LRC_GPIO_CLK();
	WM8978_BCLK_GPIO_CLK();                         
	WM8978_ADCDAT_GPIO_CLK();
	WM8978_DACDAT_GPIO_CLK();
	WM8978_MCLK_GPIO_CLK();

	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;

	GPIO_InitStructure.Pin = WM8978_LRC_PIN;
	GPIO_InitStructure.Alternate = WM8978_LRC_AF;
	HAL_GPIO_Init(WM8978_LRC_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = WM8978_BCLK_PIN;
	GPIO_InitStructure.Alternate = WM8978_BCLK_AF;
	HAL_GPIO_Init(WM8978_BCLK_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = WM8978_DACDAT_PIN;
	GPIO_InitStructure.Alternate = WM8978_DACDAT_AF;
	HAL_GPIO_Init(WM8978_DACDAT_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = WM8978_MCLK_PIN;
	GPIO_InitStructure.Alternate = WM8978_MCLK_AF;
	HAL_GPIO_Init(WM8978_MCLK_PORT, &GPIO_InitStructure);
	
	//GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin = WM8978_ADCDAT_PIN;
  
	HAL_GPIO_Init(WM8978_ADCDAT_PORT, &GPIO_InitStructure);
}


/**
	* @brief  停止I2S工作
	* @param  无
	* @retval 无
	*/
void I2S_Stop(void)
{
	/* 禁能 SPI2/I2S2 外设 */
	__HAL_I2S_DISABLE(&I2S_InitStructure);
	__HAL_I2S_DISABLE(&I2Sext_InitStructure);
	/* 关闭 I2S2 APB1 时钟 */
	WM8978_CLK_DISABLE();
}

/**
  * @brief  配置时钟
  * @param  hi2s: I2S句柄
  * @param  AudioFreq: 用于播放音频流的音频频率  
  *       
  * @retval None
  */
void BSP_AUDIO_OUT_ClockConfig(I2S_HandleTypeDef *hi2s, uint32_t AudioFreq, void *Params)
{
  RCC_PeriphCLKInitTypeDef RCC_ExCLKInitStruct;

//  HAL_RCCEx_GetPeriphCLKConfig(&RCC_ExCLKInitStruct);
//  
  /* 根据音频频率设置PLL配置 */
  if((AudioFreq == I2S_AUDIOFREQ_11K) || (AudioFreq == I2S_AUDIOFREQ_22K) || (AudioFreq == I2S_AUDIOFREQ_44K))
  {
    RCC_ExCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI2;
    RCC_ExCLKInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    RCC_ExCLKInitStruct.PLL2.PLL2M = 25;
    RCC_ExCLKInitStruct.PLL2.PLL2N = 400;
    RCC_ExCLKInitStruct.PLL2.PLL2P = 10;
    RCC_ExCLKInitStruct.PLL2.PLL2R = 2;
    RCC_ExCLKInitStruct.PLL2.PLL2Q = 2;
    RCC_ExCLKInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
    RCC_ExCLKInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    RCC_ExCLKInitStruct.PLL2.PLL2FRACN = 0;
    if (HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct) != HAL_OK)
    {
      while(1);
    }
  }
//  else /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_48K), AUDIO_FREQUENCY_96K */
//  {
//    /* 配置分配系数
//    PLLI2S_VCO: 344MHz
//    SAI_CLK(first level) = PLLI2S_VCO/PLLSAIQ = 344/7 = 49.142 Mhz
//    SAI_CLK_x = SAI_CLK(first level)/PLLI2SDivQ = 49.142/1 = 49.142 Mhz */
//    RCC_ExCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
//    RCC_ExCLKInitStruct.Sai2ClockSelection = RCC_I2SCLKSOURCE_PLLI2S;
//    RCC_ExCLKInitStruct.PLLI2S.PLLI2SP = 0;
//    RCC_ExCLKInitStruct.PLLI2S.PLLI2SN = 344;
//    RCC_ExCLKInitStruct.PLLI2S.PLLI2SR = 7;
//    RCC_ExCLKInitStruct.PLLI2S.PLLI2SQ = 1;
//    RCC_ExCLKInitStruct.PLLI2SDivQ = 1;
//    HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct);
//  }
}


/*--------------------------   音频播放部分   --------------------------------*/
/**
	* @brief  配置STM32的I2S外设工作模式
	* @param  _usStandard : 接口标准，I2S_STANDARD_PHILIPS, I2S_STANDARD_MSB 或 I2S_STANDARD_LSB
  * @param  _usWordlen : 数据格式，16bit 或者24bit
	* @param  _usAudioFreq : 采样频率，I2S_AUDIOFREQ_8K、I2S_AUDIOFREQ_16K、I2S_AUDIOFREQ_22K、
  *					I2S_AUDIOFREQ_44K、I2S_AUDIOFREQ_48K
	* @retval 无
	*/
void I2Sx_Mode_Config(const uint16_t _usStandard,const uint16_t _usWordLen,const uint32_t _usAudioFreq)
{
	
	/* PLL时钟根据AudioFreq设置 (44.1khz vs 48khz groups) */
  BSP_AUDIO_OUT_ClockConfig(&I2S_InitStructure,_usAudioFreq, NULL); /* Clock config is shared between AUDIO IN and OUT */

	/* 打开 I2S2 APB1 时钟 */
	WM8978_CLK_ENABLE();

	/* 复位 SPI2 外设到缺省状态 */
	HAL_I2S_DeInit(&I2S_InitStructure);

	/* I2S2 外设配置 */
	I2S_InitStructure.Instance = WM8978_I2Sx_SPI;
	I2S_InitStructure.Init.Mode = I2S_MODE_MASTER_TX;			/* 配置I2S工作模式 */
	I2S_InitStructure.Init.Standard = _usStandard;				/* 接口标准 */
	I2S_InitStructure.Init.DataFormat = _usWordLen;				/* 数据格式，16bit */
	I2S_InitStructure.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;	/* 主时钟模式 */
	I2S_InitStructure.Init.AudioFreq = _usAudioFreq;			/* 音频采样频率 */
	I2S_InitStructure.Init.CPOL = I2S_CPOL_LOW;
	if(HAL_I2S_Init(&I2S_InitStructure) != HAL_OK)
  {
      printf("I2S初始化失败\r\n");
  }
}

/**
	* @brief  I2Sx TX DMA配置,设置为双缓冲模式,并开启DMA传输完成中断
	* @param  buf0:M0AR地址.
	* @param  buf1:M1AR地址.
	* @param  num:每次传输数据量(以两个字节算的一个传输数据量，因为数据长度为HalfWord)
	* @retval 无
	*/
void I2Sx_TX_DMA_Init(uint32_t buffer0,uint32_t buffer1,const uint32_t num)
{  
	DMA_HandleTypeDef  DMA_InitStructure;
	I2Sx_DMA_CLK_ENABLE();//DMA1时钟使能 

	//清空DMA1_Stream4上所有中断标志
	__HAL_DMA_CLEAR_FLAG(&DMA_InitStructure,DMA_FLAG_FEIF0_4 | DMA_FLAG_DMEIF0_4 |\
						  DMA_FLAG_TEIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_TCIF0_4);
	/* 配置 DMA Stream */
	hdma_spi2_tx.Instance =I2Sx_TX_DMA_STREAM;
  hdma_spi2_tx.Init.Request = DMA_REQUEST_SPI2_TX;
	hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;//存储器到外设模式
	hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;//外设非增量模式
	hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;//存储器增量模式
	hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;//外设数据长度:16位
	hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;//存储器数据长度：16位 
	hdma_spi2_tx.Init.Mode = DMA_CIRCULAR;// 使用循环模式 
	hdma_spi2_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;//高优先级
	hdma_spi2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE; //不使用FIFO模式        
	HAL_DMA_Init(&hdma_spi2_tx);//初始化DMA Stream
  HAL_DMAEx_MultiBufferStart_IT(&hdma_spi2_tx,(uint32_t)buffer0,(uint32_t)&(WM8978_I2Sx_SPI->TXDR),(uint32_t)buffer1,num);
  
	__HAL_LINKDMA(&I2S_InitStructure,hdmatx,hdma_spi2_tx);
  
  
	HAL_NVIC_SetPriority(I2Sx_TX_DMA_STREAM_IRQn,0,3);
	HAL_NVIC_EnableIRQ(I2Sx_TX_DMA_STREAM_IRQn);
}


/**
	* @brief  SPIx_TX_DMA_STREAM中断服务函数
	* @param  无
	* @retval 无
	*/
void I2Sx_TX_DMA_STREAM_IRQFUN(void)
{  
	//执行回调函数,读取数据等操作在这里面处理	
	hdma_spi2_tx.XferCpltCallback = I2S_DMAConvCplt;
	hdma_spi2_tx.XferM1CpltCallback = I2S_DMAConvCplt;
	HAL_DMA_IRQHandler(&hdma_spi2_tx);   	
	
} 

/**
* @brief  DMA conversion complete callback. 
* @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
*                the configuration information for the specified DMA module.
* @retval None
*/
void I2S_DMAConvCplt(DMA_HandleTypeDef *hdma)
{
    MusicPlayer_I2S_DMA_TX_Callback();
}
/**
	* @brief  I2S开始播放
	* @param  无
	* @retval 无
	*/
void I2S_Play_Start(void)
{   	  
  if(HAL_IS_BIT_CLR(I2S_InitStructure.Instance->CFG1, SPI_CFG1_TXDMAEN))
  {
    if ((I2S_InitStructure.Instance->CR1 & SPI_CR1_SPE) == SPI_CR1_SPE)
    {
      /*开启DMA请求*/
      SET_BIT(I2S_InitStructure.Instance->CFG1, SPI_CFG1_TXDMAEN);
    }
    else
    {
      __HAL_I2S_DISABLE(&I2S_InitStructure);
  
      /*开启DMA请求*/
      SET_BIT(I2S_InitStructure.Instance->CFG1, SPI_CFG1_TXDMAEN);
  
      /* 使能I2S */
      __HAL_I2S_ENABLE(&I2S_InitStructure);            
    }
  }      
  /* 开始传输 */
  SET_BIT(I2S_InitStructure.Instance->CR1, SPI_CR1_CSTART);
}

/**
	* @brief  关闭I2S播放
	* @param  无
	* @retval 无
	*/
void I2S_Play_Stop(void)
{   	 
	//关闭DMA TX传输,结束播放 
	HAL_I2S_DMAStop(&I2S_InitStructure);
}

/*--------------------------   录音部分   --------------------------------*/
/**
	* @brief  配置STM32的I2S外设工作模式
	* @param  _usStandard : 接口标准，I2S_Standard_Phillips, I2S_Standard_MSB 或 I2S_Standard_LSB
  * @param  _usWordlen : 数据格式，16bit 或者24bit
	* @param  _usAudioFreq : 采样频率，I2S_AudioFreq_8K、I2S_AudioFreq_16K、I2S_AudioFreq_22K、
  *					I2S_AudioFreq_44K、I2S_AudioFreq_48
	* @retval 无
	*/
void I2Sxext_Mode_Config(const uint16_t _usStandard, const uint16_t _usWordLen,const uint32_t _usAudioFreq)
{
	/* PLL时钟根据AudioFreq设置 (44.1khz vs 48khz groups) */
  BSP_AUDIO_OUT_ClockConfig(&I2Sext_InitStructure,_usAudioFreq, NULL); /* Clock config is shared between AUDIO IN and OUT */

	/* 打开 I2S2 APB1 时钟 */
	WM8978_CLK_ENABLE();

	/* 复位 SPI2 外设到缺省状态 */
	HAL_I2S_DeInit(&I2Sext_InitStructure);

	/* I2S2 外设配置 */
	I2Sext_InitStructure.Instance = WM8978_I2Sx_SPI;
	I2Sext_InitStructure.Init.Mode = I2S_MODE_MASTER_RX;			/* 配置I2S工作模式 */
	I2Sext_InitStructure.Init.Standard = _usStandard;				/* 接口标准 */
	I2Sext_InitStructure.Init.DataFormat = _usWordLen;				/* 数据格式，16bit */
	I2Sext_InitStructure.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;	/* 主时钟模式 */
	I2Sext_InitStructure.Init.AudioFreq = _usAudioFreq;			/* 音频采样频率 */
	I2Sext_InitStructure.Init.CPOL = I2S_CPOL_LOW;  
	if(HAL_I2S_Init(&I2Sext_InitStructure) != HAL_OK)
  {
      printf("I2S初始化失败\r\n");
  }
}

/**
	* @brief  I2Sxext RX DMA配置,设置为双缓冲模式,并开启DMA传输完成中断
	* @param  buf0:M0AR地址.
	* @param  buf1:M1AR地址.
	* @param  num:每次传输数据量
	* @retval 无
	*/
void I2Sxext_RX_DMA_Init(uint32_t buffer0,uint32_t buffer1,const uint32_t num)
{
 	DMA_HandleTypeDef  DMA_InitStructure;
	I2Sx_DMA_CLK_ENABLE();//DMA1时钟使能 

	//清空DMA1_Stream4上所有中断标志
	__HAL_DMA_CLEAR_FLAG(&DMA_InitStructure,DMA_FLAG_FEIF0_4 | DMA_FLAG_DMEIF0_4 |\
						  DMA_FLAG_TEIF0_4 | DMA_FLAG_HTIF0_4 | DMA_FLAG_TCIF0_4);
	/* 配置 DMA Stream */
	hdma_spi2_rx.Instance =I2Sx_RX_DMA_STREAM;
  hdma_spi2_rx.Init.Request = DMA_REQUEST_SPI2_RX;
	hdma_spi2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;//存储器到外设模式
	hdma_spi2_rx.Init.PeriphInc = DMA_PINC_DISABLE;//外设非增量模式
	hdma_spi2_rx.Init.MemInc = DMA_MINC_ENABLE;//存储器增量模式
	hdma_spi2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;//外设数据长度:16位
	hdma_spi2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;//存储器数据长度：16位 
	hdma_spi2_rx.Init.Mode = DMA_CIRCULAR;// 使用循环模式 
	hdma_spi2_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;//高优先级
	hdma_spi2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE; //不使用FIFO模式        
	HAL_DMA_Init(&hdma_spi2_rx);//初始化DMA Stream

  HAL_DMAEx_MultiBufferStart_IT(&hdma_spi2_rx,(uint32_t)&(WM8978_I2Sx_SPI->RXDR),(uint32_t)buffer0,(uint32_t)buffer1,num);
  
	__HAL_LINKDMA(&I2Sext_InitStructure,hdmarx,hdma_spi2_rx);
	/* NVIC configuration for I2S interrupts */

	HAL_NVIC_SetPriority(I2Sx_RX_DMA_STREAM_IRQn,0,0);
	HAL_NVIC_EnableIRQ(I2Sx_RX_DMA_STREAM_IRQn);       
}

/**
	* @brief  SPIx_TX_DMA_STREAM中断服务函数
	* @param  无
	* @retval 无
	*/
void I2Sx_RX_DMA_STREAM_IRQFUN(void)
{  
	//执行回调函数,读取数据等操作在这里面处理	
	hdma_spi2_rx.XferCpltCallback = I2Sxext_DMAConvCplt;
	hdma_spi2_rx.XferM1CpltCallback = I2Sxext_DMAConvCplt;
	HAL_DMA_IRQHandler(&hdma_spi2_rx);   	
	
} 
/**
	* @brief  I2S开始录音
	* @param  无
	* @retval 无
	*/
void I2Sxext_Recorde_Start(void)
{   	  
    /* Check if the I2S Tx request is already enabled */ 
  if(HAL_IS_BIT_CLR(I2Sext_InitStructure.Instance->CFG1, SPI_CFG1_RXDMAEN))
  {
    /* Check if the SPI2S is disabled to edit CFG1 register */
    if ((I2Sext_InitStructure.Instance->CR1 & SPI_CR1_SPE) == SPI_CR1_SPE)
    {
      /* Enable Tx DMA Request */
      SET_BIT(I2Sext_InitStructure.Instance->CFG1, SPI_CFG1_RXDMAEN);
    }
    else
    {
      /* Disable SPI peripheral */
      __HAL_I2S_DISABLE(&I2Sext_InitStructure);
  
      /* Enable Tx DMA Request */
      SET_BIT(I2Sext_InitStructure.Instance->CFG1, SPI_CFG1_RXDMAEN);
      /* Enable SPI peripheral */
      __HAL_I2S_ENABLE(&I2Sext_InitStructure);        
    } 
    /* Master transfer start */
    SET_BIT(I2Sext_InitStructure.Instance->CR1, SPI_CR1_CSTART);
  }
}
/**
	* @brief  关闭I2S录音
	* @param  无
	* @retval 无
	*/
void I2Sxext_Recorde_Stop(void)
{   	 
	HAL_I2S_DMAStop(&I2Sext_InitStructure);
}

void I2Sxext_DMAConvCplt(DMA_HandleTypeDef *hdma)
{
	Recorder_I2S_DMA_RX_Callback();
}
/***************************** (END OF FILE) *********************************/
