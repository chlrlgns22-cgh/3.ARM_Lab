#include "device_driver.h"
#include <stdio.h>
#include <string.h>

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}
#if 0
void Main(void)
{
	Sys_Init(115200);
	printf("\nUART Echo-Back Test\n");

	Uart1_Init(115200);

	char x, y;

	for(x = 'A'; x <= 'Z'; x++)
	{
		// 송신 버퍼가 비면 x의 글자를 출력
		while(!(Macro_Check_Bit_Set(USART1->SR,7)));
		Macro_Write_Block(USART1->DR,0x7f,x,0);
		// 수신 버퍼에 글자가 입력되면 y에 글자를 수신
		while(!(Macro_Check_Bit_Set(USART1->SR,5)));
		y= Macro_Extract_Area(USART1->DR,0x7f,0);

		printf("%c ", y);
	}
}
#endif

#if 1
extern void Uart1_Printf(char * fmt, ...);
void Main(void)
{

	char buffer[100] = "led on 1";
	char a[10];
	char b[10];
	int num;
	Sys_Init(1152000);
	printf("\nTART Echo-Back Test\n");

	Uart1_Init(1152000);
	sscanf(buffer, "%s %s %d",a,b,&num);
	printf("Receive : %s %s %d\n",a ,b, num);
	printf("cmp = %d\n",strcmp(a,"led"));
}
#endif