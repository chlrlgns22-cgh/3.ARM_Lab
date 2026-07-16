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

void Main(void)
{
	Sys_Init(115200);
	printf("\nCompiler Test\n");
	printf("\nTerminal에서 입력한 Key가 표시되면 동작 정상\n");

	for(;;)
	{
		unsigned char x;

		while(!Macro_Check_Bit_Set(USART2->SR, 5));
		x = USART2->DR;

		while(!Macro_Check_Bit_Set(USART2->SR, 7));
		USART2->DR = x;
	}
}
