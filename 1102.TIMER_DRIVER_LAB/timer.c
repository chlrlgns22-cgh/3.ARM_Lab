#include "device_driver.h"
#define TIM2_MAX 0xFFFF

void TIM2_Stopwatch_Start(void)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);

	// TIM2 CR1 설정: down count, one pulse
	Macro_Set_Bit(TIM2->CR1,4);
	Macro_Set_Bit(TIM2->CR1,3);
	// PSC 초기값 설정 => 20usec tick이 되도록 설계 (50KHz)
	TIM2->PSC = (TIMXCLK/50000 + 0.5) -1;
	// ARR 초기값 설정 => 최대값 0xFFFF 설정
	TIM2->ARR = TIM2_MAX;
	// UG 이벤트 발생
	Macro_Set_Bit(TIM2->EGR,0);
	// TIM2 start
	Macro_Set_Bit(TIM2->CR1,0);

}

unsigned int TIM2_Stopwatch_Stop(void)
{
	unsigned int time;

	// TIM2 stop
	Macro_Clear_Bit(TIM2->CR1,0);
	// CNT 초기 설정값 (0xffff)와 현재 CNT의 펄스수 차이를 구하고
	// 그 펄스수 하나가 20usec이므로 20을 곱한값을 time에 저장
	time = 20 * ((TIM2_MAX) - (TIM2->CNT));
	// 계산된 time 값을 리턴(단위는 usec)
	return time;
}

void TIM2_Delay(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);

	// TIM2 CR1 설정: down count, one pulse
	TIM2->CR1 = (1<<4)|(1<<3);
	// PSC 초기값 설정 => 20usec tick이 되도록 설계 (50KHz)
	TIM2->PSC = (TIMXCLK/50000 + 0.5) -1;
	// ARR 초기값 설정 => 요청한 time msec에 해당하는 초기값 설정
	TIM2->ARR = 1000/20*time;
	// UG 이벤트 발생
	Macro_Set_Bit(TIM2->EGR,0);

	// UIF(Update Interrupt Pending) Clear
	Macro_Clear_Bit(TIM2->SR, 0);
	// TIM2 start
	Macro_Set_Bit(TIM2->CR1,0);
	// Wait timeout
	while(Macro_Check_Bit_Clear(TIM2->SR, 0));

	// TIM2 Stop
	Macro_Clear_Bit(TIM2->CR1, 0);
}

void TIM4_Repeat(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, 2);

	// TIM4 CR1: ARPE=0, down counter, repeat mode
	TIM4->CR1 = (1<<4)|(0<<3);
	// PSC(50KHz),  ARR(reload시 값) 설정
	TIM4->PSC = (TIMXCLK/50000 + 0.5) -1;
	TIM4->ARR = 1000/20*time;
	// UG 이벤트 발생
	Macro_Set_Bit(TIM4->EGR,0);
	// Update Interrupt Pending Clear
	Macro_Clear_Bit(TIM4->SR, 0);
	// TIM4 start
	Macro_Set_Bit(TIM4->CR1, 0);

}

int TIM4_Check_Timeout(void)
{
	// 타이머가 timeout 이면 1 리턴, 아니면 0 리턴
	if(Macro_Check_Bit_Set(TIM4->SR,0)) 
	{
		Macro_Clear_Bit(TIM4->SR,0);
		return 1;
	}
	else return 0;
}

void TIM4_Stop(void)
{
	Macro_Clear_Bit(TIM4->CR1, 0);
}

void TIM4_Change_Value(int time)
{
	TIM4->ARR = 50 * time;
}
