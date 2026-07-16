#include "device_driver.h"
#include <stdio.h>

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}

volatile int Key_Pressed = 0;
volatile int Uart_Data_In = 0;
volatile unsigned char Uart_Data = 0;

void Main(void)
{
	Sys_Init(115200);
	printf("\nUART2 RX Interrupt Test\n");

	Key_ISR_Enable(1);

	Uart2_RX_Interrupt_Enable(1);

	for(;;)
	{
		if(Key_Pressed)
		{
			printf("KEY Pressed!!!\n");
			Key_Pressed = 0;
		}

		if(Uart_Data_In)
		{
			printf("RX Data = %c\n", Uart_Data);
			Uart_Data_In = 0;
		}		

		LED_On();
		TIM2_Delay(500);
		LED_Off();
		TIM2_Delay(500);
	}
}
