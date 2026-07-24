# 코드 뜯어보기 - 8x8 도트매트릭스 조이스틱 미로 게임

버그를 찾는 리뷰가 아니라, **왜 이렇게 짜여있는지 이해하기 위한** 설명입니다.
전체 구조부터 보고, 파일 하나씩 개념 + 코드를 같이 봅니다.

## 0. 전체 구조

```
main.c            <- 게임 로직 (미로 생성, 이동, 충돌/도착 판정, 소리)
   |
   +-- dotmatrix.c <- "화면"을 담당 (8x8 LED를 어떻게 켤지)
   |     +-- shiftreg.c <- dotmatrix가 컬럼을 켤 때 74HC595를 통해서 켜야 하므로 사용
   |
   +-- joystick.c  <- "입력"을 담당 (조이스틱 X/Y/버튼)
   |     +-- adc.c <- joystick이 아날로그 값을 읽어야 하므로 ADC 하드웨어 직접 제어
   |
   +-- device_driver.h <- 위 모든 함수의 원형(prototype) 선언 모음
   +-- exception.c     <- 인터럽트가 실제로 발생했을 때 어떤 함수를 부를지 연결
```

핵심 아이디어: **main.c는 레지스터를 직접 안 만짐**. `Dot_Matrix_Set_Player(row, col)`, `Joystick_Get_X()` 같은 "의미 있는 이름의 함수"만 부르고, 그 아래 레이어(dotmatrix.c, joystick.c)가 실제 STM32 레지스터를 조작합니다. 이게 임베디드에서 말하는 "드라이버 계층 분리"입니다.

---

## 1. `shiftreg.c` - 74HC595 시프트 레지스터

### 왜 필요한가
8x8 매트릭스의 컬럼 8개를 켜려면 GPIO 핀이 8개 필요합니다. 그런데 74HC595라는 IC를 쓰면 **핀 3개(SER, SRCLK, RCLK)만으로 8개의 출력**을 만들 수 있습니다. "직렬로 데이터를 밀어넣고, 한 번에 병렬로 내보낸다"는 뜻에서 SIPO(Serial In Parallel Out) 라고 부릅니다.

### 동작 원리 (타이밍)
1. `SER` 핀에 내보내고 싶은 비트(0 또는 1)를 올려놓는다
2. `SRCLK`을 한 번 Low→High→Low로 움직이면(클럭 펄스), 그 순간의 SER 값이 IC 내부의 8칸짜리 시프트 레지스터 맨 앞에 밀려 들어가고, 기존에 있던 값들은 한 칸씩 뒤로 밀림
3. 이 과정을 8번 반복하면 8비트가 전부 IC 안에 들어감
4. `RCLK`을 한 번 움직이면, 내부에 있던 8비트가 한꺼번에 Q0~Q7 출력 핀으로 "래치"됨 (그 전까지는 출력에 반영 안 됨)

```c
void Shift595_Send_Byte(unsigned char data)
{
	int i;

	for(i = 7; i >= 0; i--)                      // MSB(7번 비트)부터 순서대로
	{
		if(Macro_Check_Bit_Set(data, i))         // data의 i번째 비트가 1이면
			Macro_Set_Bit(GPIOB->ODR, SHIFT_SER_PIN);   // SER = 1
		else
			Macro_Clear_Bit(GPIOB->ODR, SHIFT_SER_PIN); // SER = 0

		Macro_Set_Bit(GPIOB->ODR, SHIFT_SRCLK_PIN);   // SRCLK 상승 엣지 -> 이 비트가 내부로 밀려들어감
		Macro_Clear_Bit(GPIOB->ODR, SHIFT_SRCLK_PIN);
	}

	Macro_Set_Bit(GPIOB->ODR, SHIFT_RCLK_PIN);    // RCLK 상승 엣지 -> 8비트 전부 출력에 반영
	Macro_Clear_Bit(GPIOB->ODR, SHIFT_RCLK_PIN);
}
```

MSB(비트7)부터 보내는 이유: 마지막에 밀어넣은 비트가 Q0에 가장 가깝게 남기 때문에, "비트7을 제일 먼저 넣어야 최종적으로 비트0이 Q0에 온다"는 74HC595의 내부 시프트 방향과 우리가 원하는 "data의 비트 i = Qi" 매핑을 맞추기 위함입니다.

`Shift595_Init()`은 그냥 PB1/2/3을 GPIO 출력으로 설정하고 초기값을 0(Low)으로 맞추는 것뿐입니다.

---

## 2. `dotmatrix.c` - 8x8 화면 담당

### 개념 1: 멀티플렉싱(Multiplexing)과 잔상 효과
8x8 = 64개의 LED를 전부 개별 배선하면 핀이 64개+ 필요합니다. 대신 **Row(행) 8개, Col(열) 8개**로 배선해서 "Row와 Col이 동시에 활성화된 교차점의 LED만 켜지는" 격자 구조로 만듭니다. 문제는 이러면 한 번에 "한 행"만 원하는 패턴으로 켤 수 있다는 것 (여러 행을 동시에 다른 패턴으로 켤 수 없음).

해결책: **아주 빠르게 행을 하나씩 순서대로 켰다 끄는 것을 반복**합니다. 사람 눈은 초당 60번 정도보다 빠른 깜빡임은 구분 못 하기 때문에(잔상 효과), 8개 행을 16ms 안에 다 한 번씩 돌리면 마치 8개 행이 동시에 켜져 있는 것처럼 보입니다. 이게 우리 코드에서 "2ms마다 인터럽트 -> 행 하나씩 전진"하는 이유입니다.

```c
// 2ms x 8 Row = 16ms(62.5Hz) Full Frame Refresh
TIM4_Repeat_Interrupt_Enable(1, 2);
```

### 개념 2: 프레임버퍼(Framebuffer)
`Dot_FB[8]`이라는 배열 하나가 "지금 화면에 무엇을 그릴지"를 담고 있습니다. `Dot_FB[row]`의 비트 `col`이 1이면 그 칸의 LED가 켜져야 한다는 뜻입니다. 실제 하드웨어를 건드리는 스캔 코드는 이 배열만 보고 그대로 출력하면 되므로, "그림을 그리는 것"(Load, Set_Pixel)과 "그림을 실제로 표시하는 것"(Scan_ISR)이 완전히 분리됩니다.

### 개념 3: 왜 플레이어 위치는 따로 관리하나
벽(`Dot_FB`)은 항상 켜져 있어야 하고, 플레이어 위치는 깜빡여야 합니다. 만약 플레이어도 `Dot_FB`에 직접 비트를 세팅해버리면, 벽과 플레이어를 구분해서 껐다 켰다 할 수 없습니다. 그래서 `Player_Row`, `Player_Col`, `Blink_On`을 별도 변수로 두고, **스캔하는 매 순간 "벽 데이터 + (깜빡임이 켜진 상태일 때만) 플레이어 비트"를 합쳐서** 출력합니다.

```c
data = Dot_FB[Dot_Cur_Row];                     // 이 행의 벽 데이터
if(Dot_Cur_Row == Player_Row && Blink_On)       // 지금 스캔 중인 행이 플레이어가 있는 행이고, 깜빡임 ON 상태면
	Macro_Set_Bit(data, Player_Col);            // 플레이어 비트를 추가로 켬
```

### 핵심 함수: `Dot_Matrix_Scan_ISR()`
TIM4 인터럽트가 2ms마다 이 함수를 부릅니다(연결은 `exception.c`에서, 뒤에서 설명).

```c
void Dot_Matrix_Scan_ISR(void)
{
	unsigned char data;

	Macro_Clear_Bit(GPIOC->ODR, Dot_Cur_Row);    // ① 지금 켜져 있던 행을 끈다

	Dot_Cur_Row = (Dot_Cur_Row + 1) % 8;         // ② 다음 행으로 넘어간다 (0->1->...->7->0...)

	data = Dot_FB[Dot_Cur_Row];
	if(Dot_Cur_Row == Player_Row && Blink_On)
		Macro_Set_Bit(data, Player_Col);

	Shift595_Send_Byte((unsigned char)~data);    // ③ 컬럼 데이터를 74HC595로 내보낸다 (Active Low라 반전)
	Macro_Set_Bit(GPIOC->ODR, Dot_Cur_Row);      // ④ 다음 행을 켠다

	if(Dot_Cur_Row == 0)                         // ⑤ 한 바퀴(8행) 다 돌았으면
	{
		if(++Blink_Count >= BLINK_FRAMES)
		{
			Blink_Count = 0;
			Blink_On = !Blink_On;                // 점멸 상태 토글
		}
	}
}
```

순서가 중요한 이유: **①(끄기) → ③(컬럼 데이터 준비) → ④(켜기)** 순서를 지켜야 "이전 행의 패턴이 남아있는 상태에서 다음 행이 잠깐 켜지는" 고스트 현상(Ghosting)이 안 생깁니다. 만약 순서를 바꿔서 컬럼 데이터부터 바꾸고 행을 안 껐다면, 잠깐이지만 이전 행에 새 컬럼 패턴이 겹쳐 보일 수 있습니다.

`~data`(반전)를 하는 이유: 컬럼은 74HC595 출력이 **Low일 때 켜지는** 구조라는 걸 실제 배선 테스트로 확인했기 때문입니다 (자세한 내용은 대화 중 극성 디버깅 과정 참고). `Dot_FB`나 `Player` 쪽 코드는 전부 "1=켜짐"이라는 직관적인 규칙으로 통일해서 짜고, 실제로 내보낼 때만 반전시켜서 하드웨어 극성을 흡수한 것입니다.

---

## 3. `adc.c` - 아날로그 값 읽기 (ADC)

### 개념: ADC가 하는 일
조이스틱은 그냥 가변저항(퍼텐쇼미터) 2개입니다. 손잡이를 기울이면 저항값이 바뀌고, 그에 따라 출력 전압(0~3.3V 사이 어딘가)이 바뀝니다. STM32는 전압을 직접 이해 못 하니, **ADC(Analog-to-Digital Converter)** 가 이 전압을 0~4095(12비트) 사이의 숫자로 변환해줍니다.

### 레지스터 하나씩
```c
void ADC1_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 0);                // GPIOA 클럭 켜기 (PA6/PA7을 쓰려면 필요)
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x3, 12); // PA6을 "Analog 모드"로 (0b11)
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x3, 14); // PA7도 Analog 모드로

	Macro_Set_Bit(RCC->APB2ENR, 8);                // ADC1 자체에 클럭 공급 (ADC1은 APB2 버스에 붙어있음)

	Macro_Write_Block(ADC1->SMPR2, 0x7, 0x7, 18);  // CH6의 샘플링 시간 = 480 클럭 (느릴수록 정확)
	Macro_Write_Block(ADC1->SMPR2, 0x7, 0x7, 21);  // CH7도 동일하게 설정
	Macro_Write_Block(ADC1->SQR1, 0xF, 0x0, 20);   // "이번 변환 시퀀스에 채널 1개만 쓴다"는 설정

	Macro_Write_Block(ADC->CCR, 0x3, 0x2, 16);     // ADC 클럭 분주(÷6) - ADC는 너무 빠른 클럭에서 안 돌아감
	Macro_Set_Bit(ADC1->CR2, 0);                   // ADON 비트 - ADC 전원 On
}
```

- `SMPR2`: 채널마다 "한 번 읽을 때 얼마나 오래 붙잡고 있을지"를 정하는 레지스터. 값이 클수록 느리지만 정확 (조이스틱처럼 천천히 변하는 신호엔 넉넉하게 잡아도 무방).
- `SQR1`/`SQR3`: "이번에 변환할 채널이 몇 개고(SQR1), 그 첫 번째가 몇 번 채널인지(SQR3)"를 정함. 우리는 항상 "채널 1개만" 쓰고, 그 채널 번호만 `ADC1_Select_Channel()`로 바꿔가며 X/Y를 번갈아 읽습니다.

```c
void ADC1_Select_Channel(int ch)
{
	Macro_Write_Block(ADC1->SQR3, 0x1F, ch, 0);   // "1번째로 변환할 채널 = ch"
}

int ADC1_Read(int ch)
{
	ADC1_Select_Channel(ch);   // 채널 선택 (6번 또는 7번)
	ADC1_Start();              // 변환 시작 (SWSTART 비트 세팅)
	while(!ADC1_Get_Status());  // 변환이 끝날 때까지 대기 (Polling 방식)
	return ADC1_Get_Data();    // 결과값(0~4095) 읽기
}
```

`while(!ADC1_Get_Status());`가 바로 **폴링(Polling)** 방식입니다. 인터럽트를 안 쓰고, "다 됐냐?"를 CPU가 계속 물어보면서 기다리는 방식 - 코드가 단순해지는 대신 그동안 CPU가 다른 일을 못 합니다. ADC 변환 자체가 마이크로초 단위로 빨라서 이 정도는 문제없습니다.

`ADC1_Get_Status()`가 상태 플래그(EOC=변환완료)를 확인하고, true면 플래그를 지워주는 것도 같이 처리합니다(다음 변환을 위해 필수).

---

## 4. `joystick.c` - 조이스틱을 "의미 있는 값"으로

이 파일은 사실 `adc.c`와 GPIO를 그냥 "조이스틱 언어"로 포장만 한 겁니다.

```c
int Joystick_Get_X(void) { return ADC1_Read(6); }   // VRx는 ADC 채널 6번(PA6)에 연결돼있으니까
int Joystick_Get_Y(void) { return ADC1_Read(7); }   // VRy는 채널 7번(PA7)
```

버튼(SW)은 ADC가 아니라 그냥 디지털 GPIO 입력입니다. 조이스틱 버튼은 누르면 GND로 연결되는 구조(active-low)라서, "풀업(Pull-Up)"을 걸어서 평소엔 3.3V(High)로 떠 있다가 누르면 0V(Low)로 떨어지게 만듭니다.

```c
int Joystick_Get_SW(void)
{
	return Macro_Check_Bit_Clear(GPIOC->IDR, JOY_SW_PIN);  // 핀이 Low면(=눌림) 1을 리턴
}
```

`Macro_Check_Bit_Clear`라는 이름 그대로 "그 비트가 0인가?"를 물어보는 매크로입니다.

---

## 5. `device_driver.h`, `exception.c` - 연결 고리

### `device_driver.h`
C언어는 함수를 쓰기 전에 "이런 함수가 있다"라고 미리 선언(prototype)을 해줘야 다른 파일(`main.c`)에서 가져다 쓸 수 있습니다. `extern void Dot_Matrix_Init(void);` 같은 줄들이 그 선언입니다. 실제 구현(몸통)은 각 `.c` 파일에 있고, 이 헤더는 "이런 이름과 파라미터로 부르면 된다"는 약속만 모아둔 것입니다.

### `exception.c`와 인터럽트
Cortex-M(STM32) 칩은 "타이머가 다 됐다", "UART로 데이터가 왔다" 같은 이벤트가 생기면 CPU가 지금 하던 일을 멈추고 정해진 함수로 점프합니다. 이 함수 이름은 아무거나 짓는 게 아니라 **컴파일러/링커가 정한 고정된 이름**(`TIM4_IRQHandler` 같은)이어야 자동으로 연결됩니다.

```c
void TIM4_IRQHandler(void)
{
	Macro_Clear_Bit(TIM4->SR,0);      // "인터럽트 발생했다"는 하드웨어 플래그를 지움 (안 지우면 계속 또 인터럽트가 걸림)
	NVIC_ClearPendingIRQ(30);         // NVIC(인터럽트 컨트롤러)에도 "처리했다"고 알림
	TIM4_Expired = 1;

	Dot_Matrix_Scan_ISR();            // <- 우리가 추가한 한 줄. 실제 "화면 한 칸 갱신" 작업은 여기서 함
}
```

즉 "2ms마다 TIM4_IRQHandler가 자동으로 불림 -> 그 안에서 Dot_Matrix_Scan_ISR을 호출" 하는 구조라서, `main.c`는 화면 갱신에 대해 신경 쓸 필요 없이 게임 로직만 짜면 됩니다 (인터럽트가 백그라운드에서 알아서 계속 화면을 갱신해줌).

---

## 6. `main.c` - 게임 로직

### 6-1. 미로 데이터 표현
```c
static unsigned char Wall_Map[8];   // Wall_Map[row]의 col번째 비트 = 1이면 벽
```
`dotmatrix.c`의 `Dot_FB`와 똑같은 "행 하나 = 1바이트, 비트 하나 = 칸 하나" 표현을 그대로 재사용합니다. 그래서 `Dot_Matrix_Load(Wall_Map)` 한 줄이면 미로 데이터를 그대로 화면에 그릴 수 있습니다.

### 6-2. 미로 생성 알고리즘 (재귀 백트래킹) - 개념
"랜덤하게 미로를 만드는데, 항상 도착 가능해야 한다"를 만족시키는 표준적인 방법 중 하나가 **재귀 백트래킹(Recursive Backtracking)** 입니다.

핵심 아이디어: 벽으로 꽉 찬 상태에서 시작해서, **시작점부터 무작위로 인접한 칸을 골라 "뚫으면서" 전진**합니다. 더 이상 갈 곳이 없으면(막다른 길) 최근에 왔던 길로 한 칸 되돌아가서(백트래킹) 다른 방향을 시도합니다. 이 과정을 스택(Stack) 자료구조로 구현합니다 - "지금까지 지나온 경로"를 스택에 쌓아두고, 막히면 `pop`해서 이전 칸으로 돌아가는 것입니다.

```c
static void Maze_Try_Generate(void)
{
	static int stack_row[64], stack_col[64];   // 지나온 경로를 기록하는 스택
	int sp = 0;                                 // stack pointer (스택에 몇 개 쌓여있는지)
	int row = START_ROW, col = START_COL;

	for(i = 0; i < 8; i++) Wall_Map[i] = 0xFF;   // 처음엔 전부 벽(1)으로 채움

	Macro_Clear_Bit(Wall_Map[row], col);         // 시작점만 뚫어놓고 시작
	stack_row[sp] = row; stack_col[sp] = col; sp++;

	while(sp > 0 && !(row == GOAL_ROW && col == GOAL_COL))
	{
		row = stack_row[sp-1];    // 지금 스택 맨 위 = "현재 내가 서 있는 칸"
		col = stack_col[sp-1];

		// 4방향을 무작위 순서로 섞어서 시도
		for(j = 0; j < 4; j++)
		{
			nr = row + dr[order[j]];
			nc = col + dc[order[j]];

			if(범위를 벗어나면) continue;
			if(!Is_Wall(nr, nc)) continue;              // 이미 뚫린 칸이면 스킵
			if(Count_Open_Neighbors(nr, nc) != 1) continue;  // (*) 아래 설명

			Macro_Clear_Bit(Wall_Map[nr], nc);   // 그 칸을 뚫는다
			stack_row[sp] = nr; stack_col[sp] = nc; sp++;  // 스택에 쌓는다(전진)
			found = 1;
			break;
		}

		if(!found) sp--;   // 4방향 다 실패 -> 막다른 길 -> 한 칸 되돌아가기(백트래킹)
	}
}
```

**(*) `Count_Open_Neighbors(nr, nc) != 1` 이 왜 필요한가 (이번 프로젝트에서 가장 까다로웠던 부분)**

일반적인 미로 생성 알고리즘은 "칸과 칸 사이에 벽이 따로 있는" 2배 해상도 그리드를 씁니다(칸-벽-칸-벽...). 그런데 우리는 8x8을 그대로 다 쓰기로 했기 때문에(칸=벽 구분이 없음), 아무 제약 없이 인접한 칸을 막 뚫으면 **서로 다른 두 통로가 우연히 옆으로 붙어버려서 지름길이 생기는** 문제가 있습니다. 그래서 "새로 뚫으려는 칸 주변에 이미 뚫린 칸이 지금 오는 방향 하나뿐이어야 한다"는 규칙을 추가로 검사해서, 통로끼리 서로 붙지 않고 항상 폭 1칸짜리 구불구불한 길이 되도록 강제합니다.

**도착점이 막히는 버그와 재시도 로직**

문제는 이 규칙 때문에 아주 가끔(시뮬레이션으로 약 17%) **도착점 자신이 다른 두 통로에 둘러싸여서 영영 못 뚫리는 경우**가 생깁니다 (도착점은 모서리라 이웃이 원래 2개뿐이라 특히 취약). 그래서:

```c
static void Maze_Generate(void)
{
	int tries;
	for(tries = 0; tries < 50; tries++)
	{
		Maze_Try_Generate();
		if(!Is_Wall(GOAL_ROW, GOAL_COL)) return;   // 도착점이 열렸으면 성공, 끝
	}
	// 50번 다 실패하면 그냥 포기 (확률상 거의 일어날 수 없음)
}
```
"안 되면 처음부터 완전히 새로 만들어서 다시 시도"하는 단순한 방식으로 "항상 도착 가능"을 보장합니다. 시뮬레이션 결과 평균 1.2회, 최악의 경우도 5회 안에 성공했습니다.

### 6-3. 조이스틱 캘리브레이션
```c
center_x = Joystick_Get_X();   // 부팅 시점(안 건드렸다고 가정)의 값을 "중립"으로 저장
center_y = Joystick_Get_Y();
...
if(x < center_x - JOY_MARGIN && col > 0) { ncol--; ... }   // 중립보다 충분히 낮으면 왼쪽
```
이론적으로는 조이스틱 중립이 정확히 절반(약 2048)이어야 하지만, 실제로 측정해보니 저가형 조이스틱이라 중립점이 3000 근처로 치우쳐 있었습니다. 그래서 "고정된 절대값"이 아니라 "부팅 시 측정한 중립값 대비 얼마나 벗어났는지"로 판단하도록 만들어서, 조이스틱 개체마다 다른 오차를 자동으로 보정합니다.

### 6-4. 이동/충돌/도착 처리
```c
nrow = row; ncol = col;              // 일단 "이동해볼 좌표"를 따로 계산
if(조이스틱 기울임) { ncol--; moved = 1; }  // 실제 row/col은 아직 안 바꿈

if(moved)
{
	if(Is_Wall(nrow, ncol))          // 이동하려는 칸이 벽이면
		Beep_Collision();            // 충돌음만 내고, row/col은 그대로 (= "이동 취소")
	else
	{
		row = nrow; col = ncol;      // 벽이 아니면 그제서야 실제 좌표 갱신
		Dot_Matrix_Set_Player(row, col);
		if(도착점이면) { Beep_Success(); 리셋; Maze_Generate(); }
	}
}
```
"이동하기 전에 미리 벽인지 검사"하는 방식이라서, 벽에 부딪혔을 때 "이동한 뒤 되돌리기"를 할 필요 없이 애초에 좌표를 안 바꾸면 됩니다 (더 간단하고 안전한 방식).

### 6-5. 디바운스(Debounce)
```c
TIM2_Delay(300);   // 이동 한 번 하고 나면 300ms 동안 아무것도 안 함
```
사람이 조이스틱을 "한 번 툭" 기울여도 실제로는 그 상태가 몇십~몇백 ms 동안 유지됩니다. 만약 딜레이가 없으면 그 몇백ms 동안 루프가 수천 번 돌면서 "이동"이 계속 반복 실행되어 한 번 툭 친 게 여러 칸 이동으로 처리됩니다. 이동 후 일정 시간 아무 입력도 안 받도록 강제로 멈추는 것이 디바운스입니다.

### 6-6. 리셋 버튼
```c
if(Joystick_Get_SW())
{
	row = START_ROW; col = START_COL;
	Dot_Matrix_Set_Player(row, col);
	while(Joystick_Get_SW());   // 버튼을 뗄 때까지 여기서 대기
	TIM2_Delay(200);
	continue;                    // 이번 루프는 여기서 끝내고 처음부터 다시
}
```
`while(Joystick_Get_SW());`는 "버튼이 눌린 동안은 다음 줄로 안 넘어간다"는 뜻으로, 버튼을 계속 누르고 있어도 리셋이 반복 실행되지 않도록 막습니다.

---

## 정리: 각 개념이 어디서 왔는지

| 개념 | 어디서 쓰였나 |
|---|---|
| 시프트 레지스터(SIPO) | `shiftreg.c` - 74HC595로 GPIO 절약 |
| 멀티플렉싱 + 잔상효과 | `dotmatrix.c` - 8x8을 3개 핀+8개 핀으로 표시 |
| 프레임버퍼 | `dotmatrix.c` - `Dot_FB[8]` |
| 인터럽트(ISR) | `dotmatrix.c`(스캔) + `exception.c`(연결) |
| ADC(아날로그->디지털) | `adc.c` - 조이스틱 전압 읽기 |
| 폴링 | `adc.c` - 변환 완료 대기 |
| 풀업 저항 + active-low | `joystick.c` - 버튼 입력 |
| 재귀 백트래킹 + 스택 | `main.c` - 미로 생성 |
| 캘리브레이션 | `main.c` - 조이스틱 중립점 자동 측정 |
| 디바운스 | `main.c` - 이동/버튼 반복 방지 |
