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
	printf("\nI2C BUS Interface Test\n");

	volatile int i, j, data;
		
	I2C1_SC16IS752_Init(400000);	// 5KHz ~ 400KHz
	I2C1_SC16IS752_Config_GPIO(0xFF);

	for(;;)
	{
		for(i = 0; i < 8; i++)
		{
			data = ~(1u << i);
			I2C1_SC16IS752_Write_GPIO(data);
			printf("LED[%d] ON, Value = 0x%.2X\n", i, (unsigned char)data);
			for(j = 0; j < 1000000; j++);
		}
	}
}
