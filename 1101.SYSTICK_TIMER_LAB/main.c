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
	printf("SysTick 시간 측정 test\n");

	volatile int i, j;
	unsigned int cnt;

	for(i = 1; i <= 4; i++)
	{
		SysTick_Run(1000);
		cnt = 0x40000 * i;
		for(j = 0; j < cnt; j++);
		SysTick_Stop();
		cnt = (int)((SysTick_Get_Load_Time() - SysTick_Get_Time()) * (8.*1000./HCLK));
		printf("Delay Time = %u msec\n", cnt);
	}

	printf("SysTick TICK 발생 test\n");

	i = 0;
	SysTick_Run(1000);

	for(;;)
	{
		if(SysTick_Check_Timeout())
		{
			(i ^= 0x1) ? LED_On() : LED_Off();
			printf(".");
		}
	}
}
