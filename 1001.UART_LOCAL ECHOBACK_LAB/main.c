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

#if 0
extern void Uart1_Printf(char * fmt, ...);
void Main(void)
{

	char buffer[100] = "led on 1";
	char a[10];
	char b[10];
	int num;
	Sys_Init(115200);
	printf("\nUART Echo-Back Test\n");

	Uart1_Init(115200);
	sscanf(buffer, "%s %s %d",a,b,&num);
	printf("Receive : %s %s %d\n",a ,b, num);
	printf("cmp = %d\n",strcmp(a,"led"));
}
#endif


#if 0
void Main(void)
{
	Sys_Init(115200);

	// LED2, LED3 : PA6, PA7 => Output, Open-Drain
	Macro_Set_Bit(RCC->AHB1ENR, 0);
	
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

	printf("\n[Ping-Pong LED Test]\n");
	printf("u:speed up  d:slow down  s:pause/resume\n");

	int idx = 0;         // 0,1,2 -> LED1,2,3
	int dir = 1;         // +1 or -1
	int paused = 0;
	int period = 200;    // 한 스텝 이동 주기 (ms)
	int elapsed = 0;      // 마지막 스텝 이후 누적 시간 (ms)
	const int quantum = 10;
	const int period_min = 50, period_max = 1000, period_step = 25;

	LED_On(); // LED1 켜고 시작

	for(;;)
	{
		// ---- UART(USART2, printf 나가는 포트) non-blocking 1바이트 수신 ----
		if(Macro_Check_Bit_Set(USART2->SR, 5))
		{
			char c = (char)USART2->DR;

			if(c == 'u')
			{
				if(period > period_min) period -= period_step;
				printf("Speed Up! period = %d ms\n", period);
			}
			else if(c == 'd')
			{
				if(period < period_max) period += period_step;
				printf("Slow Down! period = %d ms\n", period);
			}
			else if(c == 's')
			{
				paused ^= 1;
				printf(paused ? "Paused\n" : "Resumed\n");
			}
		}

		// ---- LED 이동 스테이트머신 ----
		if(!paused)
		{
			elapsed += quantum;

			if(elapsed >= period)
			{
				elapsed = 0;

				// idx 번째 LED Off
				if(idx == 0)      Macro_Set_Bit(GPIOA->ODR, 5);
				else if(idx == 1) Macro_Set_Bit(GPIOA->ODR, 6);
				else if(idx == 2) Macro_Set_Bit(GPIOA->ODR, 7);

				idx += dir;

				if(idx >= 2)
				{
					idx = 2;
					dir = -1;
					printf("[PONG]\n");
				}
				else if(idx <= 0)
				{
					idx = 0;
					dir = 1;
					printf("[PING]\n");
				}

				// idx 번째 LED On
				if(idx == 0)      Macro_Clear_Bit(GPIOA->ODR, 5);
				else if(idx == 1) Macro_Clear_Bit(GPIOA->ODR, 6);
				else if(idx == 2) Macro_Clear_Bit(GPIOA->ODR, 7);
			}
		}

		// ---- SysTick busy-wait quantum(ms) ----
		SysTick->CTRL = 0;
		SysTick->LOAD = (unsigned int)((HCLK / 8000.0) * quantum + 0.5) - 1;
		SysTick->VAL = 0;
		Macro_Set_Bit(SysTick->CTRL, 0);
		while(!(Macro_Check_Bit_Set(SysTick->CTRL, 16)));
		SysTick->CTRL = 0;
	}
}
#endif

#if 0
void Main(void)
{
	Sys_Init(115200);

	// LED2, LED3 : PA6, PA7 => Output, Open-Drain
	Macro_Set_Bit(RCC->AHB1ENR, 0);
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 10);
	Macro_Set_Bit(GPIOA->OTYPER, 5);
	Macro_Set_Bit(GPIOA->ODR, 5);
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 12);
	Macro_Set_Bit(GPIOA->OTYPER, 6);
	Macro_Set_Bit(GPIOA->ODR, 6);
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 14);
	Macro_Set_Bit(GPIOA->OTYPER, 7);
	Macro_Set_Bit(GPIOA->ODR, 7);

	// Key2 : PC7 => Input, Pull-Up  (0901.KEY_IN_LAB 참고)
	Macro_Set_Bit(RCC->AHB1ENR, 2);
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 14);
	Macro_Write_Block(GPIOC->PUPDR, 0x3, 0x1, 14);

	printf("\n[Reaction Time Test]\n");

	unsigned int seed = 0xACE1u;

	for(;;)
	{
		printf("\nPress Key 1 to Start!\n");

		int blink = 0;

		while(!Key_Get_Pressed())
		{
			blink ^= 1;

			if(blink)
			{
				Macro_Clear_Bit(GPIOA->ODR, 5);
				Macro_Clear_Bit(GPIOA->ODR, 6);
				Macro_Clear_Bit(GPIOA->ODR, 7);
			}
			else
			{
				Macro_Set_Bit(GPIOA->ODR, 5);
				Macro_Set_Bit(GPIOA->ODR, 6);
				Macro_Set_Bit(GPIOA->ODR, 7);
			}

			// 300ms busy-wait
			SysTick->CTRL = 0;
			SysTick->LOAD = (unsigned int)((HCLK / 8000.0) * 300 + 0.5) - 1;
			SysTick->VAL = 0;
			Macro_Set_Bit(SysTick->CTRL, 0);
			while(!(Macro_Check_Bit_Set(SysTick->CTRL, 16)));
			SysTick->CTRL = 0;

			seed = seed * 1103515245u + 12345u + SysTick->VAL; // 대기 시간 편차를 섞은 의사 난수
		}

		while(Key_Get_Pressed()); // 키 릴리즈 대기 (디바운스)

		Macro_Set_Bit(GPIOA->ODR, 5);
		Macro_Set_Bit(GPIOA->ODR, 6);
		Macro_Set_Bit(GPIOA->ODR, 7);

		int wait_ms = 1500 + (int)(seed % 2500u); // 1.5 ~ 4.0초 랜덤 대기
		int foul = 0;
		int t;

		for(t = 0; t < wait_ms; t += 10)
		{
			if(Macro_Check_Bit_Clear(GPIOC->IDR, 7))
			{
				foul = 1;
				break;
			}

			// 10ms busy-wait
			SysTick->CTRL = 0;
			SysTick->LOAD = (unsigned int)((HCLK / 8000.0) * 10 + 0.5) - 1;
			SysTick->VAL = 0;
			Macro_Set_Bit(SysTick->CTRL, 0);
			while(!(Macro_Check_Bit_Set(SysTick->CTRL, 16)));
			SysTick->CTRL = 0;
		}

		if(foul)
		{
			printf("Foul Play! (반칙)\n");
			while(Macro_Check_Bit_Clear(GPIOC->IDR, 7));
			continue;
		}

		printf("PRESS KEY NOW!\n");
		Macro_Clear_Bit(GPIOA->ODR, 5);
		Macro_Clear_Bit(GPIOA->ODR, 6);
		Macro_Clear_Bit(GPIOA->ODR, 7);

		SysTick->CTRL = 0;
		SysTick->LOAD = 0x00FFFFFF;   // 최대 약 1.4초까지 측정 (96MHz/8 기준)
		SysTick->VAL = 0;
		Macro_Set_Bit(SysTick->CTRL, 0);

		while(!Macro_Check_Bit_Clear(GPIOC->IDR, 7));

		unsigned int ticks = 0x00FFFFFFu - SysTick->VAL;
		SysTick->CTRL = 0;

		unsigned int reaction_ms = (unsigned int)(ticks / (HCLK / 8000.0));
		printf("Your reaction time: %u ms!\n", reaction_ms);

		while(Macro_Check_Bit_Clear(GPIOC->IDR, 7));

		Macro_Set_Bit(GPIOA->ODR, 5);
		Macro_Set_Bit(GPIOA->ODR, 6);
		Macro_Set_Bit(GPIOA->ODR, 7);
	}
}
#endif

#if 1
void Main(void)
{
	Sys_Init(115200);

	// LED2, LED3 : PA6, PA7 => Output, Open-Drain
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

	// Key2 : PC7 => Input, Pull-Up
	Macro_Set_Bit(RCC->AHB1ENR, 2);

	printf("\n[UART CLI Controller]\n");
	printf("Type 'help' for command list.\n> ");

	char line[64];
	int len = 0;
	int led_state[3] = {0, 0, 0};

	int timer_remain_ms = 0;
	int blink_remain = 0;
	int blink_elapsed = 0;
	int blink_on = 0;
	const int blink_half_period = 300;
	const int quantum = 10;

	for(;;)
	{
		// ---- UART(USART2) non-blocking 1바이트 수신 ----
		if(Macro_Check_Bit_Set(USART2->SR, 5))
		{
			char c = (char)USART2->DR;

			if(c == '\r' || c == '\n')
			{
				line[len] = 0;
				printf("\n");

				if(len == 0)
				{
					/* 빈 입력 무시 */
				}
				else if(strcmp(line, "help") == 0)
				{
					printf("Commands:\n");
					printf(" help\n");
					printf(" led on [1-3]\n");
					printf(" led off [1-3]\n");
					printf(" status\n");
					printf(" timer [sec]\n");
				}
				else if(strcmp(line, "status") == 0)
				{
					printf("LED1=%s LED2=%s LED3=%s\n",
						led_state[0] ? "ON" : "OFF",
						led_state[1] ? "ON" : "OFF",
						led_state[2] ? "ON" : "OFF");

					printf("KEY1=%s KEY2=%s\n",
						Key_Get_Pressed() ? "Pressed" : "Released",
						Macro_Check_Bit_Clear(GPIOC->IDR, 7) ? "Pressed" : "Released");
				}
				else
				{
					char cmd1[16], cmd2[16];
					int num;
					int n = sscanf(line, "%s %s %d", cmd1, cmd2, &num);

					if(n == 3 && strcmp(cmd1, "led") == 0 && strcmp(cmd2, "on") == 0 && num >= 1 && num <= 3)
					{
						if(num == 1)      Macro_Clear_Bit(GPIOA->ODR, 5);
						else if(num == 2) Macro_Clear_Bit(GPIOA->ODR, 6);
						else if(num == 3) Macro_Clear_Bit(GPIOA->ODR, 7);

						led_state[num - 1] = 1;
						printf("LED%d ON\n", num);
					}
					else if(n == 3 && strcmp(cmd1, "led") == 0 && strcmp(cmd2, "off") == 0 && num >= 1 && num <= 3)
					{
						if(num == 1)      Macro_Set_Bit(GPIOA->ODR, 5);
						else if(num == 2) Macro_Set_Bit(GPIOA->ODR, 6);
						else if(num == 3) Macro_Set_Bit(GPIOA->ODR, 7);

						led_state[num - 1] = 0;
						printf("LED%d OFF\n", num);
					}
					else if(sscanf(line, "timer %d", &num) == 1 && num > 0)
					{
						timer_remain_ms = num * 1000;
						blink_remain = 0;
						printf("Timer set: %d sec\n", num);
					}
					else
					{
						printf("Unknown command: %s (type 'help')\n", line);
					}
				}

				len = 0;
				printf("> ");
			}
			else if(c == 0x08 || c == 0x7f) // Backspace
			{
				if(len > 0)
				{
					len--;
					printf("\b \b");
				}
			}
			else if(len < (int)sizeof(line) - 1)
			{
				line[len++] = c;
				printf("%c", c); // echo
			}
		}

		// ---- timer 명령 카운트다운 (non-blocking) ----
		if(timer_remain_ms > 0)
		{
			timer_remain_ms -= quantum;

			if(timer_remain_ms <= 0)
			{
				timer_remain_ms = 0;
				printf("\nTimer Expired! Blinking...\n> ");
				blink_remain = 6; // On/Off 3회 = 6 토글
				blink_elapsed = 0;
				blink_on = 0;
			}
		}

		// ---- 만료 후 3회 점멸 (non-blocking) ----
		if(blink_remain > 0)
		{
			blink_elapsed += quantum;

			if(blink_elapsed >= blink_half_period)
			{
				blink_elapsed = 0;
				blink_on ^= 1;

				if(blink_on)
				{
					Macro_Clear_Bit(GPIOA->ODR, 5);
					Macro_Clear_Bit(GPIOA->ODR, 6);
					Macro_Clear_Bit(GPIOA->ODR, 7);
				}
				else
				{
					Macro_Set_Bit(GPIOA->ODR, 5);
					Macro_Set_Bit(GPIOA->ODR, 6);
					Macro_Set_Bit(GPIOA->ODR, 7);
				}

				blink_remain--;

				if(blink_remain == 0)
				{
					Macro_Set_Bit(GPIOA->ODR, 5);
					Macro_Set_Bit(GPIOA->ODR, 6);
					Macro_Set_Bit(GPIOA->ODR, 7);
					led_state[0] = led_state[1] = led_state[2] = 0;
				}
			}
		}

		// ---- SysTick busy-wait quantum(ms) ----
		SysTick->CTRL = 0;
		SysTick->LOAD = (unsigned int)((HCLK / 8000.0) * quantum + 0.5) - 1;
		SysTick->VAL = 0;
		Macro_Set_Bit(SysTick->CTRL, 0);
		while(!(Macro_Check_Bit_Set(SysTick->CTRL, 16)));
		SysTick->CTRL = 0;
	}
}
#endif
