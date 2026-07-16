#include "device_driver.h"
#include <stdio.h>

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Uart2_Init(baud);	
	setvbuf(stdout, NULL, _IONBF, 0);
}

#if 0

#define GPIOA_MODER		(*(unsigned long *)0x40020000)
#define GPIOA_OTYPER	(*(unsigned long *)0x40020004)
#define GPIOA_ODR		(*(unsigned long *)0x40020014)

void Main(void)
{
	Sys_Init(115200);
	printf("LED Toggling Test #1\n");

	GPIOA_MODER = 0x1 << 10;
	GPIOA_OTYPER = 0x0 << 5;

	int i;

	for(;;)
	{
		GPIOA_ODR = 0x1 << 5; 
		for(i=0; i<0x10000; i++);

		GPIOA_ODR = 0x0 << 5; 
		for(i=0; i<0x10000; i++);
	}
}

#endif

#if 0

#define TIMER   (*(unsigned long *)0xE000E018)

void Main(void)
{
	int i;
	unsigned long a[10];

	Sys_Init(115200);
	printf("Timer Access #1\n");

	SysTick_Run();

	for(i=0; i<10; i++)
	{
		a[i] = TIMER;
		printf("%d => %#.8X\n", i, a[i]);
	}
}

#endif

#if 1

#define TIMER   (*(unsigned long *)0xE000E018)

void Main(void)
{
	int i;
	unsigned long a[10];

	Sys_Init(115200);
	printf("Timer Access #2\n");

	SysTick_Run();

	for(i=0; i<10; i++)
	{
		a[i] = TIMER;
	}

	for(i=0; i<10; i++)
	{
		printf("%d => %#.8X\n", i, a[i]);
	}
}

#endif

#if 0

#define TIMER   (*(volatile unsigned long *)0xE000E018)

void Main(void)
{
	int i;
	unsigned long a[10];

	Sys_Init(115200);
	printf("Timer Access #3\n");

	SysTick_Run();

	for(i=0; i<10; i++)
	{
		a[i] = TIMER;
	}

	for(i=0; i<10; i++)
	{
		printf("%d => %#.8X\n", i, a[i]);
	}
}

#endif

#if 0

#define GPIOA_MODER		(*(volatile unsigned long *)0x40020000)
#define GPIOA_OTYPER	(*(volatile unsigned long *)0x40020004)
#define GPIOA_ODR		(*(volatile unsigned long *)0x40020014)

void Main(void)
{
	Sys_Init(115200);
	printf("LED Toggling Test #2\n");

	GPIOA_MODER = 0x1 << 10;
	GPIOA_OTYPER = 0x0 << 5;

	volatile int i;

	for(;;)
	{
		GPIOA_ODR = 0x1 << 5; 
		for(i=0; i<0x40000; i++);
		
		GPIOA_ODR = 0x0 << 5; 
		for(i=0; i<0x40000; i++);
	}
}

#endif
