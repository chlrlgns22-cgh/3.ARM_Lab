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

#if 1

void Main(void)
{
	Sys_Init(115200);
	printf("TIM2 stopwatch test\n");

	int i;

	for(i = 1; i <= 10; i++)
	{
		i % 2 ? LED_On() : LED_Off();

		TIM2_Stopwatch_Start();
		SysTick_Run(100 * i);
		while(!SysTick_Check_Timeout());
		SysTick_Stop();

		unsigned int r = TIM2_Stopwatch_Stop();

		printf("[%d] Elapsed Time = %f msec\n", i, r/1000.);
	}
}

#endif

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("TIM2 delay test\n");

	for(;;)
	{
		LED_On();
		TIM2_Delay(1000);
		printf(".");

		LED_Off();
		TIM2_Delay(1000);
		printf(".");
	}
}

#endif

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("TIM2 long delay test\n");

	for(;;)
	{
		LED_On();
		TIM2_Delay(5000);
		printf(".");

		LED_Off();
		TIM2_Delay(5000);
		printf(".");
	}
}

#endif

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("TIM4 repeat timeout test\n");

	int i = 1, j = 1;

	TIM4_Repeat(500);
	SysTick_Run(500);

	for(;;)
	{
		if(TIM4_Check_Timeout())
		{
			printf("Timer4 [%d]\n", i++);
			if(i > 20) TIM4_Stop();
		}

	    if(SysTick_Check_Timeout())
	    {
			printf("SysTick [%d]\n", j++);
			if(j > 20) SysTick_Stop();
	    }
	}
}

#endif

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("TIM4 variable interval test\n");

	int i = 1;
	TIM4_Repeat(50);

	for(;;)
	{
		if(TIM4_Check_Timeout())
		{
			printf("[%d]\n", i++);
			if(i > 20) i = 1;
			TIM4_Change_Value(50 * i);
		}
	}
}

#endif
