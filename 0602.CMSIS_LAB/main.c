#include "device_driver.h"
#include <stdio.h>

static void Sys_Init(int baud) 
{
  SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
}

void Main(void)
{
	volatile int i;

	Sys_Init(115200);
	printf("CMSIS Based Register Define\n");

	// LED Pin을 출력으로 설정
	GPIOA->MODER = 0x1 <<10;
	GPIOA->OTYPER = 0;

	for(;;)
	{
		// LED ON
		GPIOA->ODR =0x1<<5;
		for(i=0; i<0x40000; i++);

		// LED OFF
		GPIOA->ODR =0;
		for(i=0; i<0x40000; i++);
	}
}
