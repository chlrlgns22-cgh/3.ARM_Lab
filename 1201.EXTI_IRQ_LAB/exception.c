#include "device_driver.h"
#include <stdio.h>

void _Invalid_ISR(void)
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for(;;);
}

void EXTI15_10_IRQHandler(void)
{
	// KEY Pressed 메시지 인쇄
	printf("Key Pressed\n");
	// KEY(EXTI) Pending Clear
	Macro_Set_Bit(EXTI->PR,13);
	// NVIC Pending Clear
	NVIC_ClearPendingIRQ((IRQn_Type) 40);
}
