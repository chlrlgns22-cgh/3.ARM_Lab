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
	printf("KEY Driver Test\n");

	Key_Poll_Init();
	
	for(;;)
	{
		volatile int i, j;

		printf("Key_Get_Pressed() 함수 동작 시험\n");
		printf("Key를 누르면 Key Pressed! 인쇄, 안 눌리면 계속 # 인쇄 => 10회 동작\n");

		for(i = 0; i<10; i++)
		{
			for(;;)
			{
				if(Key_Get_Pressed())
				{
					printf("Key Pressed! [%d]\n", i);
					Key_Wait_Key_Released();
					break;
				}

				else
				{
					printf("#");
					for(j=0; j<0x80000; j++);
				}
			}
		}

		printf("\nKey_Wait_Key_Pressed(), Key_Wait_Key_Released() 함수 동작 시험\n");
		printf("Key를 누르면 ON 떼면 OFF => 10회 동작\n");

		for(i = 0; i<10; i++)
		{
			Key_Wait_Key_Pressed();
			printf("Key Pressed! [%d]\n", i);
			LED_On();
			Key_Wait_Key_Released();
			LED_Off();
		}
	}
}
