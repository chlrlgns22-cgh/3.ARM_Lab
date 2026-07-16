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

/* Key 인식 */

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("KEY Input Test #1\n");

	/* 아래 코드 수정 금지 : Port-C Clock Enable */
	Macro_Set_Bit(RCC->AHB1ENR, 2); 

	// KEY(PC13)을 GPIO 입력으로 선언
	Macro_Write_Block(GPIOC->MODER,0x3,0x0,26);

	
	for(;;)
	{
		// KEY가 눌렸으면 LED(PA5) ON, 안 눌렸으면 OFF
		Macro_Write_Block(GPIOA->ODR,0x1,Macro_Extract_Area(~GPIOC->IDR,0x1,13),5);
		// if(Macro_Check_Bit_Set(GPIOC->IDR,13))
		// {
		// 	LED_On();
		// }
		// else LED_Off();
	
	}
}

#endif

/* Key에 의한 LED Toggling */

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("KEY Input Toggling #1\n");
	int prev=0;
	int current=0;

	Macro_Set_Bit(RCC->AHB1ENR, 2); 
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 26);
	// int waiting;
	for(;;)
	{
		// KEY(PC13)이 눌릴때마다 LED(PA5)가 Toggling하도록 코드 작성
		current = Macro_Check_Bit_Set(GPIOC->IDR,13);
		if(current ==1 && prev ==0)
		{
			Macro_Invert_Bit(GPIOA->ODR,5);
		}
		prev = current;
		// if(Macro_Check_Bit_Set(GPIOC->IDR,13))
		// {
		// 	if(Macro_Check_Bit_Set(GPIOA->ODR,5) & waiting==0) 
		// 	{
		// 		LED_Off();
		// 		waiting = 1;
		// 	}
		// 	else if(Macro_Check_Bit_Clear(GPIOA->ODR,5) & waiting==0) 
		// 	{
		// 		LED_On();
		// 		waiting = 1;
		// 	}
		// 	if(Macro_Check_Bit_Clear(GPIOC->IDR,13) && waiting == 1)
		// 	{
		// 		waiting =0;
		// 	}
		}		
	}
#endif

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("KEY Input Test #1\n");

	/* 아래 코드 수정 금지 : Port-C Clock Enable */
	Macro_Set_Bit(RCC->AHB1ENR, 2); 

	// KEY(PC7)을 GPIO 입력으로 선언
	Macro_Write_Block(GPIOC->MODER,0x3,0x0,14);
	Macro_Write_Block(GPIOC->PUPDR,0x3,0x1,14);
	
	for(;;)
	{
		// KEY가 눌렸으면 LED(PA5) ON, 안 눌렸으면 OFF
		if(!Macro_Check_Bit_Set(GPIOC->IDR,7))
		{
			LED_On();
		}
		else LED_Off();
	}
}

#endif

#if 1

void Main(void)
{
	Sys_Init(115200);
	printf("KEY Input Test #1\n");

	Macro_Set_Bit(RCC->AHB1ENR, 2); 

	Macro_Write_Block(GPIOC->MODER,0x3,0x0,14);
	Macro_Write_Block(GPIOC->PUPDR,0x3,0x1,14);
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 10);
	Macro_Set_Bit(GPIOA->OTYPER, 5);
	Macro_Set_Bit(GPIOA->ODR, 5);
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 12);
	Macro_Set_Bit(GPIOA->OTYPER, 6);
	Macro_Set_Bit(GPIOA->ODR, 6);
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 14);
	Macro_Set_Bit(GPIOA->OTYPER, 7);
	Macro_Set_Bit(GPIOA->ODR, 7);

	volatile int i=0;
	int cnt = 0;
	int led5 = 1, led6 = 1, led7 = 1;
	int prev = 1;
	int timer = 0;

	for(;;)
	{
		int cur = Macro_Check_Bit_Set(GPIOC->IDR,7);

		if(!cur)  // 누르는 동안
		{
			timer++;
			if(timer >= 0x80000)
			{
				timer = 0;
				if(cnt>=4) cnt=0;

				led5 = (cnt==0) ? 0 : 1;
				led6 = (cnt==1||cnt==3) ? 0 : 1;
				led7 = (cnt==2) ? 0 : 1;

				if(led5) Macro_Set_Bit(GPIOA->ODR,5);
				else     Macro_Clear_Bit(GPIOA->ODR,5);
				if(led6) Macro_Set_Bit(GPIOA->ODR,6);
				else     Macro_Clear_Bit(GPIOA->ODR,6);
				if(led7) Macro_Set_Bit(GPIOA->ODR,7);
				else     Macro_Clear_Bit(GPIOA->ODR,7);

				cnt++;
			}
		}
		else if(cur && !prev)  // 뗀 순간 1회
		{
			timer = 0;
			if(led5 == 0)
			{
				for(i=0;i<0x800000;i++);
				Macro_Set_Bit(GPIOA->ODR,5);
				Macro_Set_Bit(GPIOA->ODR,6);
				Macro_Set_Bit(GPIOA->ODR,7);
				cnt = 0;
			}
			if(led6 == 0)
			{
				for(i=0;i<0x800000;i++);
				Macro_Clear_Bit(GPIOA->ODR,5);
				Macro_Clear_Bit(GPIOA->ODR,6);
				Macro_Clear_Bit(GPIOA->ODR,7);
				cnt = 0;
			}
			if(led7 == 0)
			{
				for(i=0;i<0x800000;i++);
				Macro_Set_Bit(GPIOA->ODR,5);
				Macro_Set_Bit(GPIOA->ODR,6);
				Macro_Set_Bit(GPIOA->ODR,7);
				cnt = 0;
			}
		}

		prev = cur;
	}
}

#endif
