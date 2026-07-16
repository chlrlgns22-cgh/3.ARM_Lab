#include "device_driver.h"
#include <stdio.h>

static void Sys_Init(int baud) 
{
    SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
}

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("LED ON\n");

	GPIOA->MODER = 0x1 << 10;
	GPIOA->OTYPER = 0x0 << 5;
	GPIOA->ODR = 0x1 << 5; 
}

#endif

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("LED ON : Bit Operation - 1\n");

	/* 비트 연산을 이용하여 LED를 ON하는 코드를 설계하시오 */
	GPIOA->MODER |= 0x1 << 10;
	GPIOA->OTYPER |= 0x0 << 5;
	GPIOA->ODR |= 0x1 << 5;

}

#endif

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("LED Toggling : Macro\n");

	volatile int i;

	/* 매크로를 이용하여 초기에 LED를 출력으로 설정하고 OFF */
	Macro_Write_Block(GPIOA->MODER,0x3,0x1,10);
	Macro_Clear_Bit(GPIOA->OTYPER,5);
	Macro_Clear_Bit(GPIOA->ODR,5);

	
	for(;;)
	{
		/* LED 반전 및 Delay, Delay는 0x80000으로 설정 */
		Macro_Invert_Bit(GPIOA->ODR,5);
		for(i=0;i<0x80000;i++);
	}
}

#endif


#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("LED Toggling : Macro\n");
	

	volatile int i;
	int j=0;
	/* 매크로를 이용하여 초기에 LED를 출력으로 설정하고 OFF */
	Macro_Write_Block(GPIOA->MODER,0x3,0x1,10);
	Macro_Write_Block(GPIOA->MODER,0x3,0x1,12);
	Macro_Write_Block(GPIOA->MODER,0x3,0x1,14);
	Macro_Set_Bit(GPIOA->OTYPER,5);
	Macro_Set_Bit(GPIOA->OTYPER,6);
	Macro_Set_Bit(GPIOA->OTYPER,7);
	Macro_Set_Bit(GPIOA->ODR,5);
	Macro_Set_Bit(GPIOA->ODR,6);
	Macro_Set_Bit(GPIOA->ODR,7);
	
	
	for(;;)
	{
		/* LED 반전 및 Delay, Delay는 0x80000으로 설정 */
		if(j==8) j=0;
		if((j==1||j==3||j==5||j==7)) {Macro_Clear_Bit(GPIOA->ODR,5);}
		else {Macro_Set_Bit(GPIOA->ODR,5);}
		if((j==2||j==3||j==6||j==7)) {Macro_Clear_Bit(GPIOA->ODR,6);}
		else {Macro_Set_Bit(GPIOA->ODR,6);}
		if((j==4||j==5||j==6||j==7)) {Macro_Clear_Bit(GPIOA->ODR,7);}
		else {Macro_Set_Bit(GPIOA->ODR,7);}
		for(i=0;i<0x80000;i++);
		j++;
	}
}

#endif

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("LED Toggling : Macro\n");

	volatile int i;
	int j=0;
	/* 매크로를 이용하여 초기에 LED를 출력으로 설정하고 OFF */
	Macro_Write_Block(GPIOA->MODER,0x3,0x1,10);
	Macro_Write_Block(GPIOA->MODER,0x3,0x1,12);
	Macro_Write_Block(GPIOA->MODER,0x3,0x1,14);
	Macro_Set_Bit(GPIOA->OTYPER,5);
	Macro_Set_Bit(GPIOA->OTYPER,6);
	Macro_Set_Bit(GPIOA->OTYPER,7);
	Macro_Set_Bit(GPIOA->ODR,5);
	Macro_Set_Bit(GPIOA->ODR,6);
	Macro_Set_Bit(GPIOA->ODR,7);

	
	for(;;)
	{
		/* LED 반전 및 Delay, Delay는 0x80000으로 설정 */
		if(j==4) j=0;
		if(j == 0) {Macro_Clear_Bit(GPIOA->ODR,5);}
		else {Macro_Set_Bit(GPIOA->ODR,5);}
		if(j==1||j==3) {Macro_Clear_Bit(GPIOA->ODR,6);}
		else {Macro_Set_Bit(GPIOA->ODR,6);}
		if(j==2) 	{Macro_Clear_Bit(GPIOA->ODR,7);}
		else {Macro_Set_Bit(GPIOA->ODR,7);}
		for(i=0;i<0x80000;i++);
		j++;
	}
}

#endif
