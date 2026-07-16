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
	printf("SPI BUS Interface Test\n");

	volatile int i, j, data;

	/* PCLK2(96MHz)의 분주비 전달 : 2,4,6,8,16,32,64,128,256 */
	/* SC16IS752의 SPI 최대 속도 4MHz => 32(3MHz) */

	SPI1_SC16IS752_Init(32); 			
	SPI1_SC16IS752_Config_GPIO(0xFF);

	for(;;)
	{
		for(i = 0; i < 8; i++)
		{
			data = ~(1u << i);
			SPI1_SC16IS752_Write_GPIO(data);
			printf("LED[%d] ON, Value = 0x%.2X\n", i, (unsigned char)data);
			for(j = 0; j < 1000000; j++);
		}
	}
}
