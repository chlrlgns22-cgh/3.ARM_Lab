#include "device_driver.h"
#include <stdio.h>

void _Invalid_ISR(void)
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for(;;);
}

extern volatile int Key_Pressed;

void EXTI15_10_IRQHandler(void)
{
	Key_Pressed = 1;
	
	EXTI->PR = 0x1 << 13;
	NVIC_ClearPendingIRQ(40);
}

extern volatile int Uart_Data_In;
extern volatile unsigned char Uart_Data;

void USART2_IRQHandler(void)
{
	// 수신된 데이터는 Uart_Data에 저장
	Uart_Data = USART2->DR;
	// Uart_Data_In Flag Setting
	Uart_Data_In=1;
	// NVIC Pending Clear
	NVIC_ClearPendingIRQ(38);
}
