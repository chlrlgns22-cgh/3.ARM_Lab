#include "device_driver.h"

// 1088BS Row(8) => PC0~PC7 (Active High, 1 row at a time)
// 1088BS Col(8) => 74HC595 Q0~Q7 (Active Low)

static unsigned char Dot_FB[8];		// 배경(벽) 프레임 - 항상 켜짐
static int Dot_Cur_Row;

static int Player_Row = -1, Player_Col = -1;
static int Blink_On = 1;
static int Blink_Count = 0;
#define BLINK_FRAMES	10		// 20 Frame(약 320ms)마다 점멸 토글

void Dot_Matrix_Init(void)
{
	int i;

	Shift595_Init();

	Macro_Set_Bit(RCC->AHB1ENR, 2); // GPIOC Clock Enable

	for(i = 0; i < 8; i++)
	{
		Macro_Write_Block(GPIOC->MODER, 0x3, 0x1, i*2); // PC0~7 Output
		Macro_Clear_Bit(GPIOC->ODR, i);                 // Row Off
	}

	for(i = 0; i < 8; i++) Dot_FB[i] = 0;
	Dot_Cur_Row = 0;

	// 2ms x 8 Row = 16ms(62.5Hz) Full Frame Refresh
	TIM4_Repeat_Interrupt_Enable(1, 2);
}

void Dot_Matrix_Clear(void)
{
	int i;
	for(i = 0; i < 8; i++) Dot_FB[i] = 0;
}

void Dot_Matrix_Set_Pixel(int row, int col, int on)
{
	if(on)
		Macro_Set_Bit(Dot_FB[row], col);
	else
		Macro_Clear_Bit(Dot_FB[row], col);
}

void Dot_Matrix_Load(const unsigned char *pattern)
{
	int i;
	for(i = 0; i < 8; i++) Dot_FB[i] = pattern[i];
}

// 점멸하는 현재 위치 표시 (벽과 별도 레이어)
void Dot_Matrix_Set_Player(int row, int col)
{
	Player_Row = row;
	Player_Col = col;
}

// TIM4 ISR에서 매 tick마다 호출 (Row 1개씩 순차 점등)
void Dot_Matrix_Scan_ISR(void)
{
	unsigned char data;

	Macro_Clear_Bit(GPIOC->ODR, Dot_Cur_Row);    // 현재 Row Off

	Dot_Cur_Row = (Dot_Cur_Row + 1) % 8;

	data = Dot_FB[Dot_Cur_Row];
	if(Dot_Cur_Row == Player_Row && Blink_On)
		Macro_Set_Bit(data, Player_Col);

	Shift595_Send_Byte((unsigned char)~data);    // Col은 Active Low라 반전해서 출력
	Macro_Set_Bit(GPIOC->ODR, Dot_Cur_Row);      // 다음 Row On

	if(Dot_Cur_Row == 0)                         // 한 프레임(8 Row) 끝날 때마다
	{
		if(++Blink_Count >= BLINK_FRAMES)
		{
			Blink_Count = 0;
			Blink_On = !Blink_On;
		}
	}
}
