#include "stm32f4xx.h"
#include "option.h"
#include "macro.h"
#include "malloc.h"

// Uart.c

extern void Uart2_Init(int baud);
extern void Uart2_Send_Byte(char data);
extern void Uart2_RX_Interrupt_Enable(int en);

extern void Uart1_Init(int baud);
extern void Uart1_Send_Byte(char data);
extern void Uart1_Send_String(char *pt);
extern void Uart1_Printf(char *fmt,...);
extern char Uart1_Get_Char(void);
extern char Uart1_Get_Pressed(void);

// SysTick.c

extern void SysTick_Run(unsigned int msec);
extern int SysTick_Check_Timeout(void);
extern unsigned int SysTick_Get_Time(void);
extern unsigned int SysTick_Get_Load_Time(void);
extern void SysTick_Stop(void);

// Led.c

extern void LED_Init(void);
extern void LED_On(void);
extern void LED_Off(void);

// Clock.c

extern void Clock_Init(void);

// Key.c

extern void Key_Poll_Init(void);
extern int Key_Get_Pressed(void);
extern void Key_Wait_Key_Released(void);
extern void Key_Wait_Key_Pressed(void);
extern void Key_ISR_Enable(int en);

// Timer.c

extern void TIM2_Delay(int time);
extern void TIM2_Stopwatch_Start(void);
extern unsigned int TIM2_Stopwatch_Stop(void);
extern void TIM4_Repeat(int time);
extern int TIM4_Check_Timeout(void);
extern void TIM4_Stop(void);
extern void TIM4_Change_Value(int time);
extern void TIM4_Repeat_Interrupt_Enable(int en, int time);
extern void TIM3_Out_Init(void);
extern void TIM3_Out_Freq_Generation(unsigned short freq);
extern void TIM3_Out_Stop(void);

// Shiftreg.c

extern void Shift595_Init(void);
extern void Shift595_Send_Byte(unsigned char data);

// Dotmatrix.c

extern void Dot_Matrix_Init(void);
extern void Dot_Matrix_Clear(void);
extern void Dot_Matrix_Set_Pixel(int row, int col, int on);
extern void Dot_Matrix_Load(const unsigned char *pattern);
extern void Dot_Matrix_Set_Player(int row, int col);
extern void Dot_Matrix_Scan_ISR(void);

// Adc.c

extern void ADC1_Init(void);
extern void ADC1_Select_Channel(int ch);
extern void ADC1_Start(void);
extern int ADC1_Get_Status(void);
extern int ADC1_Get_Data(void);
extern int ADC1_Read(int ch);

// Joystick.c

extern void Joystick_Init(void);
extern int Joystick_Get_X(void);
extern int Joystick_Get_Y(void);
extern int Joystick_Get_SW(void);