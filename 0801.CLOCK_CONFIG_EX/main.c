#include "device_driver.h"
#include <stdio.h>

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}

#if 0

/* 반드시 Option.h의 조건부 컴파일 부분을 변경한 후 실험 */

void Main(void)
{
	Sys_Init(115200);
	printf("SysClock = 8MHz(HSI)\n");

	for(;;)
	{
		volatile int i;

		LED_On();
		for(i=0; i<0x100000; i++);
		LED_Off();
		for(i=0; i<0x100000; i++);
		printf(".");
	}
}

#else

/* 반드시 Option.h의 조건부 컴파일 부분을 변경한 후 실험 */

void Main(void)
{
	Clock_Init();

	Sys_Init(115200);
	printf("SysClock = 100MHz(HSI)\n");

	for(;;)
	{
		volatile int i;

		LED_On();
		for(i=0; i<0x100000; i++);
		LED_Off();
		for(i=0; i<0x100000; i++);
		printf(".");
	}
}

#endif
