#include "device_driver.h"

// 74HC595 : PB1(SER) / PB2(SRCLK) / PB3(RCLK)
#define SHIFT_SER_PIN		1
#define SHIFT_SRCLK_PIN		2
#define SHIFT_RCLK_PIN		3

void Shift595_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 1); // GPIOB Clock Enable

	Macro_Write_Block(GPIOB->MODER, 0x3, 0x1, SHIFT_SER_PIN*2);
	Macro_Write_Block(GPIOB->MODER, 0x3, 0x1, SHIFT_SRCLK_PIN*2);
	Macro_Write_Block(GPIOB->MODER, 0x3, 0x1, SHIFT_RCLK_PIN*2);

	Macro_Clear_Bit(GPIOB->ODR, SHIFT_SER_PIN);
	Macro_Clear_Bit(GPIOB->ODR, SHIFT_SRCLK_PIN);
	Macro_Clear_Bit(GPIOB->ODR, SHIFT_RCLK_PIN);
}

void Shift595_Send_Byte(unsigned char data)
{
	int i;

	for(i = 7; i >= 0; i--)
	{
		if(Macro_Check_Bit_Set(data, i))
			Macro_Set_Bit(GPIOB->ODR, SHIFT_SER_PIN);
		else
			Macro_Clear_Bit(GPIOB->ODR, SHIFT_SER_PIN);

		Macro_Set_Bit(GPIOB->ODR, SHIFT_SRCLK_PIN);
		Macro_Clear_Bit(GPIOB->ODR, SHIFT_SRCLK_PIN);
	}

	Macro_Set_Bit(GPIOB->ODR, SHIFT_RCLK_PIN);
	Macro_Clear_Bit(GPIOB->ODR, SHIFT_RCLK_PIN);
}
