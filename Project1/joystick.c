#include "device_driver.h"

// HW-504 SW => PC8 (Active Low, Pull-Up)
#define JOY_SW_PIN		8

void Joystick_Init(void)
{
	ADC1_Init();

	Macro_Set_Bit(RCC->AHB1ENR, 2); 					// GPIOC Clock Enable
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, JOY_SW_PIN*2);  // Input
	Macro_Write_Block(GPIOC->PUPDR, 0x3, 0x1, JOY_SW_PIN*2);  // Pull-Up
}

int Joystick_Get_X(void)
{
	return ADC1_Read(6);
}

int Joystick_Get_Y(void)
{
	return ADC1_Read(7);
}

int Joystick_Get_SW(void)
{
	return Macro_Check_Bit_Clear(GPIOC->IDR, JOY_SW_PIN);
}
