#include "device_driver.h"
#include <stdio.h>
#include <stdlib.h>

volatile int Key_Pressed;
volatile int Uart_Data_In;
volatile unsigned char Uart_Data;
volatile int TIM4_Expired;

static void Sys_Init(int baud)
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2);
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}

// 미로 데이터 : row0(위)~row7(아래), bit=col, 1=벽, 0=통로
// 왼쪽아래(7,0) 시작 -> 오른쪽위(0,7) 도착
static unsigned char Wall_Map[8];

#define START_ROW	7
#define START_COL	0
#define GOAL_ROW	0
#define GOAL_COL	7

#define JOY_MARGIN	600		// 중립값 기준 이만큼 벗어나야 이동으로 인식

static int Is_Wall(int row, int col)
{
	return Macro_Check_Bit_Set(Wall_Map[row], col);
}

// 후보 칸의 이미 열린 이웃 수 (4방향) - 1이어야(=지금 오는 방향만) 통로가 서로 붙는(지름길 생기는) 것을 막음
static int Count_Open_Neighbors(int row, int col)
{
	int cnt = 0;

	if(row > 0 && !Is_Wall(row-1, col)) cnt++;
	if(row < 7 && !Is_Wall(row+1, col)) cnt++;
	if(col > 0 && !Is_Wall(row, col-1)) cnt++;
	if(col < 7 && !Is_Wall(row, col+1)) cnt++;

	return cnt;
}

// 재귀 백트래킹 1회 시도 - 구석(특히 도착점)이 다른 통로에 둘러싸여 영영 못 뚫리는 경우가 드물게 있음
static void Maze_Try_Generate(void)
{
	static int stack_row[64], stack_col[64];
	int sp = 0;
	int row = START_ROW, col = START_COL;
	int i;
	const int dr[4] = {-1, 1, 0, 0};
	const int dc[4] = {0, 0, -1, 1};

	for(i = 0; i < 8; i++) Wall_Map[i] = 0xFF;	// 전부 벽으로 초기화

	Macro_Clear_Bit(Wall_Map[row], col);		// 시작점 오픈
	stack_row[sp] = row;
	stack_col[sp] = col;
	sp++;

	while(sp > 0 && !(row == GOAL_ROW && col == GOAL_COL))
	{
		int order[4] = {0, 1, 2, 3};
		int j, k, tmp, found = 0;

		row = stack_row[sp-1];
		col = stack_col[sp-1];

		for(j = 3; j > 0; j--)					// 방향 순서 셔플
		{
			k = rand() % (j+1);
			tmp = order[j]; order[j] = order[k]; order[k] = tmp;
		}

		for(j = 0; j < 4; j++)
		{
			int nr = row + dr[order[j]];
			int nc = col + dc[order[j]];

			if(nr < 0 || nr > 7 || nc < 0 || nc > 7) continue;
			if(!Is_Wall(nr, nc)) continue;					// 이미 통로면 skip
			if(Count_Open_Neighbors(nr, nc) != 1) continue;	// 다른 통로와 합쳐지면 skip

			Macro_Clear_Bit(Wall_Map[nr], nc);
			stack_row[sp] = nr;
			stack_col[sp] = nc;
			sp++;
			row = nr;
			col = nc;
			found = 1;
			break;
		}

		if(!found) sp--;		// 막다른 길 : 백트래킹
	}
}

// 도착점이 못 뚫리는 경우를 대비해 뚫릴 때까지 재시도 (평균 1~2회, 최대 몇 회면 충분)
static void Maze_Generate(void)
{
	int tries;

	for(tries = 0; tries < 50; tries++)
	{
		Maze_Try_Generate();
		if(!Is_Wall(GOAL_ROW, GOAL_COL)) return;
	}
}

// 1103.TIMER_OUTPUT_LAB 에서 쓴 12음계표 재사용
enum key{C1, C1_, D1, D1_, E1, F1, F1_, G1, G1_, A1, A1_, B1, C2, C2_, D2, D2_, E2, F2, F2_, G2, G2_, A2, A2_, B2};
static const unsigned short tone_value[] = {261,277,293,311,329,349,369,391,415,440,466,493,523,554,587,622,659,698,739,783,830,880,932,987};

static void Buzzer_Beep(unsigned char tone, int duration)
{
	TIM3_Out_Freq_Generation(tone_value[tone]);
	TIM2_Delay(duration);
	TIM3_Out_Stop();
}

static void Beep_Collision(void)
{
	int i;

	for(i = 0; i < 2; i++)
	{
		TIM3_Out_Freq_Generation(150);	// 스케일 밖 저음 = 충돌/에러 느낌
		TIM2_Delay(60);
		TIM3_Out_Stop();
		TIM2_Delay(60);
	}
}

static void Beep_Success(void)
{
	Buzzer_Beep(C1, 100);
	Buzzer_Beep(E1, 100);
	Buzzer_Beep(G1, 100);
	Buzzer_Beep(C2, 250);	// 도-미-솔-도(한 옥타브 위) 클리어 징글
}

void Main(void)
{
	int row = START_ROW, col = START_COL;
	int center_x, center_y;
	int nrow, ncol;
	int x, y;
	int moved;

	Sys_Init(115200);
	printf("\nMaze Dot Test\n");

	Dot_Matrix_Init();
	Joystick_Init();
	TIM3_Out_Init();

	// 부팅 시점(조이스틱을 안 건드린 상태) 값을 중립점으로 삼음
	center_x = Joystick_Get_X();
	center_y = Joystick_Get_Y();
	printf("Center X=%d Y=%d\n", center_x, center_y);

	srand(center_x ^ (center_y << 16));	// 조이스틱 노이즈로 시드
	Maze_Generate();
	Dot_Matrix_Load(Wall_Map);
	Dot_Matrix_Set_Player(row, col);

	for(;;)
	{
		if(Joystick_Get_SW())			// 버튼 누르면 현재 미로에서 시작점으로 리셋
		{
			row = START_ROW;
			col = START_COL;
			Dot_Matrix_Set_Player(row, col);

			while(Joystick_Get_SW());	// 버튼 뗄 때까지 대기 (연속 리셋 방지)
			TIM2_Delay(200);
			continue;
		}

		x = Joystick_Get_X();
		y = Joystick_Get_Y();
		nrow = row;
		ncol = col;
		moved = 0;

		if(x < center_x - JOY_MARGIN && col > 0)       { ncol--; moved = 1; }
		else if(x > center_x + JOY_MARGIN && col < 7)  { ncol++; moved = 1; }
		else if(y < center_y - JOY_MARGIN && row > 0)  { nrow--; moved = 1; }
		else if(y > center_y + JOY_MARGIN && row < 7)  { nrow++; moved = 1; }

		if(moved)
		{
			if(Is_Wall(nrow, ncol))
			{
				Beep_Collision();		// 벽 충돌 : 이동 취소, 이전 위치 유지
			}
			else
			{
				row = nrow;
				col = ncol;
				Dot_Matrix_Set_Player(row, col);

				if(row == GOAL_ROW && col == GOAL_COL)
				{
					Beep_Success();		// 도착 : 성공음
					row = START_ROW;
					col = START_COL;
					Maze_Generate();		// 다음 판을 위한 새 미로
					Dot_Matrix_Load(Wall_Map);
					Dot_Matrix_Set_Player(row, col);
				}
			}

			TIM2_Delay(300);	// 연속 이동 방지용 디바운스
		}
	}
}
