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
	printf("ADC Test\n");
		
	volatile int i;

	ADC1_IN6_Init();

	for(;;)
	{
		ADC1_Start();
		while(!ADC1_Get_Status());
		printf("0x%.4X\n", ADC1_Get_Data());
		for(i=0; i<0x400000; i++);
	}
}
