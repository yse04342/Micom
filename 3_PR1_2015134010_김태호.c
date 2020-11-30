#include "stm32f4xx.h"
#include "GLCD.h"
#include <math.h>
void _GPIO_Init(void);
uint16_t KEY_Scan(void);
void _EXTI_Init(void);//점멸을 위해 쓰는것.

void BEEP(void); // 비프음
void DisplayInitScreen(void); //LCD 초기화면 설정
void DelayMS(unsigned short wMS); // 시간 딜레이
void DelayUS(unsigned short wUS);// 시간 딜레이
void BEEP3(void); // 비프음 연속 3번
void Floor(void); // 층 선택 모드
void First_Floor(void);// 1층 선택 함수
void Second_Floor(void);// 2층 선택 함수
void Third_Floor(void);// 3층 선택 함수
void Foured_Floor(void);// 4층 선택 함수
void Fifth_Floor(void);// 5층 선택 함수
void Sixed_Floor(void);// 6층 선택 함수
uint8_t	SW0_Flag,SW7_Flag; // 스위치 0번 7번 눌렀을때
uint8_t i=0,j=0,Start_Floor=1,Final_Floor=1,count=0,swap=1,b=0;  //Start_Floor : 시작층,Final_Floor: 끝층 ,b :몇 번째 눌렸는지 확인 용상수 swap : 출발 층 ->목표층 같은 키 눌렀을때 방지용 b
uint8_t One_Elevator=1,Two_Elevator=1;//왼쪽 엘레베이터와 오른쪽 엘레베이터 변수설정
void Execute(void);//실행모드 함수
void Hold(void);//중단모드(SW0번 눌렸을 때)
void Time_Delay(void);//딜레이 5초 함수sadasdasdsadas
int main(void)
{
  _GPIO_Init(); // GPIO (LED & SW) 초기화
  LCD_Init();	// LCD 모듈 초기화
  _EXTI_Init();// 외부 인터럽트에 대한 초기화
  DelayMS(100);
  //BEEP();
  
  
  DisplayInitScreen();	// LCD 초기화면
  LCD_SetTextColor(RGB_BLACK);// 글자색 : BULE
  while(1)
  {
    
    Floor();
    
    if(SW7_Flag&&count==0&&b==1)// 스위치 7번이 눌리고  눌린 횟수가 짝수번쨰이고 초기 상태가 아니라면?(b는 초기상태 제외 하기위한 변수)
    {
      
      SW0_Flag=0;//SW0의 플래그변수는 0이다. => 실행모드에 들어가기 직전에 층선택 모드 후 바로 중단 모드가 실행 안되게 해주는 것
      Execute(); ///실행모드로 들어간다.
      SW7_Flag=0;//SW7의 플래그변수는 0이다. 
      count=0;
      b=0;
      
    }
  } 
}
void _EXTI_Init(void)
{
  
  RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
  RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
  
  GPIOH->MODER 	&= 0x0000FFFF;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
  
  SYSCFG->EXTICR[2] |= 0x0007; 	// EXTI8에 대한 소스 입력은 GPIOH로 설정 (EXTICR3) (reset value: 0x0000)//PH8->EXTI8로 바꿔주는 역할
  EXTI->FTSR |= 0x0100;		// Falling Trigger Enable  (EXTI8:PH8)//눌렀을때 폴링
  EXTI->IMR  |= 0x0100;  	// EXTI8 인터럽트 mask (Interrupt Enable) //IMR 이 담당한다.
  NVIC->ISER[0] |= ( 1<<23  );   // Enable Interrupt EXTI8,9 Vector table Position 참조 //ISER[0] ~ [7]까지 있다. [0]은 (0번부터 31번까지 담당한다. 32는 [1]에 있다.) 1개마다 32bit
  //NVIC->ISER[0] |= ( 0x00800000  );
  
  SYSCFG->EXTICR[3] |= 0x7000; 	//EXIT15에 대한 소스 입력은 GPIOH로 설정// PH15->EXIT15	
  EXTI->FTSR |= 0xE000;		// Falling Trigger Enable  (15,14,13번)//눌렀을때 폴링
  EXTI->IMR  |= 0xE000;  	// EXTI15,14,13 인터럽트 mask (Interrupt Enable) //IMR 이 담당한다.// 0xE000 =0b 1110 0000 0000 0000
  NVIC->ISER[1] |= ( 1<<8  ); //  외부 인터럽트 넘버는 40번이다. 이떄 32개 단위 이므로  ISER[1]에 8번째(32+8)에 있다.
}

void _GPIO_Init(void)
{
  // LED (GPIO G) 설정
  RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
  GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
  GPIOG->OTYPER	&= ~0x000000FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
  GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
  
  // SW (GPIO H) 설정 
  RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
  GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
  GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state
  
  // Buzzer (GPIO F) 설정 
  RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
  GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
  GPIOF->OTYPER 	&= ~0x00000200;	// GPIOF 9 : Push-pull  	
  GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
  LCD_SetBackColor(RGB_YELLOW);	//글자배경색 : Yellow
  LCD_SetBackColor(RGB_YELLOW);	//글자배경색 : Yellow
}	

void DisplayInitScreen(void) // 초기 화면 설정
{
  GPIOG->ODR |= 0x0001; 	// GPIOG->ODR.5 'H'(LED ON)
  LCD_Clear(RGB_YELLOW);	// 화면 클리어(배경 : 노란색)
  LCD_SetFont(&Gulim8);	// 폰트 : 굴림 8
  LCD_SetBackColor(GET_RGB(255,0,255));// 글자배경색 : YELLOW
  LCD_SetTextColor(RGB_BLACK);// 글자색 : BLACK
  LCD_DisplayText(0,0,"Mecha-Elevator");  // 첫 번째 문장(Title) 출력
  LCD_DisplayText(5,6,"1->1");  // 첫 번째 문장(Title) 출력
  
  LCD_SetTextColor(RGB_BLUE);// 글자색 :  BLUE
  
  for(int a=2; a<=7; a++) // 왼쪽 엘레베이터 1층부터 6층까지 표시를 해준다.
  {
    LCD_DisplayChar(a,4,0x30+(8-a));  //왼쪽 엘레베이터 1층부터 6층까지 표시를 해준다.
  }
  LCD_DisplayText(0,16,"FL");  // FL모드
  LCD_DisplayText(4,6,"L:S");  // 왼쪽 엘레베이터 정지모드
  
  LCD_SetTextColor(RGB_RED);// 글자색 : RED
  for(int a=2; a<=7; a++) // 오른쪽 엘레베이터  1층부터 6층까지 표시를 해준다.
  {
    LCD_DisplayChar(a,12,0x30+(8-a)); // 오른쪽 엘레베이터  1층부터 6층까지 표시를 해준다.
  }
  LCD_SetBrushColor(RGB_BLUE); // 브러쉬색 : 파란색
  LCD_DrawFillRect(15,92,10,12); // 사각형  출력
  
  
  LCD_SetBrushColor(RGB_RED); // 브러쉬색 : 파란색
  LCD_DrawFillRect(105,92,10,12); // 사각형 내부
}

uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7  // 스위치 한번 눌리게 해주는 것
{ 
  uint16_t key;
  key = GPIOH->IDR & 0xFF00;	// any key pressed ?
  if(key == 0xFF00)		// if no key, check key off
  {  	if(key_flag == 0)
    return key;
  else
  {	DelayMS(10);
  key_flag = 0;
  return key;
  }
  }
  else				// if key input, check continuous key
  {	if(key_flag != 0)	// if continuous key, treat as no key input
    return 0xFF00;
  else			// if new key,delay for debounce
  {	key_flag = 1;
  DelayMS(10);
  return key;
  }
  }
}

void EXTI9_5_IRQHandler(void)		// EXTI 5~9 인터럽트 핸들러 //얘는 이미 선언되있어서 그냥 쓰면 된다.
{
  if(EXTI->PR & 0x0100) 		// EXTI8 Interrupt Pending?
  {
    EXTI->PR |= 0x0100; 		// Pending bit Clear//PR = Pending Register 
    SW0_Flag = 1;
  }
}

void EXTI15_10_IRQHandler(void)		// EXTI 15~10 인터럽트 핸들러 //얘는 이미 선언되있어서 그냥 쓰면 된다.
{
  if(EXTI->PR & 0x8000) 		// EXTI15 Interrupt Pending?
  {
    EXTI->PR |= 0x8000; 		// Pending bit Clear//PR = Pending Register 
    SW7_Flag = 1;
  }
}
void BEEP(void)			/* beep for 30 ms */ // 비프음 1번 출력 함수
{ 	
  	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
  	DelayMS(30);		// Delay 30 ms
  	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}

void BEEP3(void)			/* beep for 30 ms */ // 비프음 3번 출력 함수
{ 	
  BEEP();
  DelayMS(100);		// Delay 30 ms
  BEEP();
  DelayMS(100);
  BEEP();
  DelayMS(100);
}
void Floor(void)
{
  SW7_Flag=0;//SW7의 플래그변수는 0이다. 
  
  switch(KEY_Scan())//if문 보다는 key_scan()을 하면 Chattering현상이 일어나지 않음.
  {
  case 0xFD00 : 	//SW1를 누른다면
    First_Floor();
    BEEP();
    break;
    
  case 0xFB00  : 	//SW2를 누른다면
    Second_Floor();
    BEEP();
    break;
    
  case 0xF700 : 	//SW3를 누른다면
    Third_Floor();
    BEEP();
    break;
    
  case 0xEF00 : 	//SW4를 누른다면
    Foured_Floor();
    BEEP();
    break;
    
  case 0xDF00 : 	//SW5를 누른다면 
    Fifth_Floor();
    BEEP();
    break;
    
  case 0xBF00 : 	//SW6를 누른다면
    Sixed_Floor();
    BEEP();
    break;
  }
}
void First_Floor(void)//1층
{
  GPIOG->ODR &= 1<<0;//스위치 0번을 제외하고 모두 OFF
  if(count==0)// 만약 count가 0이라면
  {
    Start_Floor=1;// 시작층 :1층
    LCD_DisplayChar(5,6,0x30+Start_Floor);  // 문장 Start_Floor->Final_Floor 출력
    count=1; // count는 1
    GPIOG->ODR |= 1<<Start_Floor; // 시작층의 LED를 킨다
  }
  else if(count==1)// 아니면 count가 1이라면
  {
    swap=Final_Floor;// 우선 이전에 저장된 도착층을 저장하고
    Final_Floor=1;// 도착 층은 1층
    GPIOG->ODR |= 1<<Final_Floor; // LED는 마지막 층에 해당하는 번호를 킨다
    
    if(Start_Floor!=Final_Floor)// 만약 시작층과 도착층이 같지 않다면?
    {
      LCD_DisplayChar(5,9,0x30+Final_Floor);  // 문장 Start_Floor->Final_Floor 출력
      count=0;//count는 0
      b=1;// b=1로 설정한다.(b=1이면 SW7번이 눌리게 설정하였음.
    }
    else // 서로 같다면?(변화가 없어야한다.)
    {
      Final_Floor=swap;// 저장되었던 마지막층을 다시 불러온다.
      count=1;// 그대로 count는 1
    }
    
  }
}
void Second_Floor(void)//2층
{
  GPIOG->ODR &= 0x0001;//스위치 0번을 제외하고 모두 OFF
  if(count==0)// 만약 count가 0이라면
  {
    Start_Floor=2;// 시작층 :2층
    GPIOG->ODR |= 1<<Start_Floor;// 시작층의 LED를 킨다
    LCD_DisplayChar(5,6,0x30+Start_Floor); // 문장 Start_Floor->Final_Floor 출력
    count=1;// count는 1
  }
  else if(count==1)// 아니면 count가 1이라면
  {
    swap=Final_Floor;// 우선 이전에 저장된 도착층을 저장하고
    Final_Floor=2;// 도착 층은 2층
    GPIOG->ODR |= 1<<Final_Floor;// LED는 마지막 층에 해당하는 번호를 킨다
    if(Start_Floor!=Final_Floor)// 만약 시작층과 도착층이 같지 않다면?
    {
      LCD_DisplayChar(5,9,0x30+Final_Floor);// 문장 Start_Floor->Final_Floor 출력
      count=0;//count는 0
      b=1;// b=1로 설정한다.(b=1이면 SW7번이 눌리게 설정하였음.
      
    }
    else// 서로 같다면?(변화가 없어야한다.)
    {
      Final_Floor=swap;// 저장되었던 마지막층을 다시 불러온다.
      count=1; // 그대로 count는 1
    }
  }
}
void Third_Floor(void)
{
  GPIOG->ODR &= 0x0001;//스위치 0번을 제외하고 모두 OFF
  if(count==0)// 만약 count가 0이라면
  {
    Start_Floor=3;// 시작층 :3층
    GPIOG->ODR |= 1<<Start_Floor;// 시작층의 LED를 킨다
    LCD_DisplayChar(5,6,0x30+Start_Floor);  // 문장 Start_Floor->Final_Floor 출력
    count=1;// count는 1
  }
  else if(count==1)// 아니면 count가 1이라면
  {
    swap=Final_Floor;// 우선 이전에 저장된 도착층을 저장하고
    Final_Floor=3;// 도착 층은 3층
    GPIOG->ODR |= 1<<Final_Floor;// LED는 마지막 층에 해당하는 번호를 킨다
    if(Start_Floor!=Final_Floor)// 만약 시작층과 도착층이 같지 않다면?
    {
      LCD_DisplayChar(5,9,0x30+Final_Floor);  // 문장 Start_Floor->Final_Floor 출력
      count=0;//count는 0
      b=1;// b=1로 설정한다.(b=1이면 SW7번이 눌리게 설정하였음.
      
    }
    else// 서로 같다면?(변화가 없어야한다.)
    {
      Final_Floor=swap;// 저장되었던 마지막층을 다시 불러온다.
      count=1; // 그대로 count는 1
    }
  }
}
void Foured_Floor(void)
{
  GPIOG->ODR &= 0x0001;//스위치 0번을 제외하고 모두 OFF
  if(count==0)// 만약 count가 0이라면
  {
    
    Start_Floor=4;// 시작층 :4층
    GPIOG->ODR |= 1<<Start_Floor;// 시작층의 LED를 킨다
    LCD_DisplayChar(5,6,0x30+Start_Floor); // 문장 Start_Floor->Final_Floor 출력
    count=1;// count는 1
  }
  else if(count==1)// 아니면 count가 1이라면
  {
    swap=Final_Floor;// 우선 이전에 저장된 도착층을 저장하고
    Final_Floor=4;// 도착 층은 4층
    GPIOG->ODR |= 1<<Final_Floor;// LED는 마지막 층에 해당하는 번호를 킨다
    if(Start_Floor!=Final_Floor)// 만약 시작층과 도착층이 같지 않다면?
    {
      LCD_DisplayChar(5,9,0x30+Final_Floor); // 문장 Start_Floor->Final_Floor 출력
      count=0;//count는 0
      b=1;// b=1로 설정한다.(b=1이면 SW7번이 눌리게 설정하였음.
      
    }
    else// 서로 같다면?(변화가 없어야한다.)
    {
      Final_Floor=swap;// 저장되었던 마지막층을 다시 불러온다.
      count=1; // 그대로 count는 1
    }
  }
}
void Fifth_Floor(void)
{
  GPIOG->ODR &= 0x0001;//스위치 0번을 제외하고 모두 OFF
  if(count==0)// 만약 count가 0이라면
  {
    Start_Floor=5;// 시작층 :5층
    GPIOG->ODR |= 1<<Start_Floor;// 시작층의 LED를 킨다
    LCD_DisplayChar(5,6,0x30+Start_Floor); // 문장 Start_Floor->Final_Floor 출력
    count=1;// count는 1
  }
  else if(count==1)// 아니면 count가 1이라면
  {
    swap=Final_Floor;// 우선 이전에 저장된 도착층을 저장하고
    Final_Floor=5;// 도착 층은 6층
    GPIOG->ODR |= 1<<Final_Floor;// LED는 마지막 층에 해당하는 번호를 킨다
    if(Start_Floor!=Final_Floor)// 만약 시작층과 도착층이 같지 않다면?
    {
      LCD_DisplayChar(5,9,0x30+Final_Floor); // 문장 Start_Floor->Final_Floor 출력
      count=0;//count는 0
      b=1;// b=1로 설정한다.(b=1이면 SW7번이 눌리게 설정하였음.
      
    }
    else// 서로 같다면?(변화가 없어야한다.)
    {
      Final_Floor=swap;// 저장되었던 마지막층을 다시 불러온다.
      count=1; // 그대로 count는 1
    }
  }
}
void Sixed_Floor(void)
{
  GPIOG->ODR &= 0x0001;//스위치 0번을 제외하고 모두 OFF
  if(count==0)// 만약 count가 0이라면
  {
    Start_Floor=6;// 시작층 :6층
    GPIOG->ODR |= 1<<Start_Floor;// 시작층의 LED를 킨다
    LCD_DisplayChar(5,6,0x30+Start_Floor);  // 문장 Start_Floor->Final_Floor 출력
    count=1;// count는 1
  }
  else if(count==1)// 아니면 count가 1이라면
  {
    swap=Final_Floor;// 우선 이전에 저장된 도착층을 저장하고
    Final_Floor=6;// 도착 층은 6층
    GPIOG->ODR |= 1<<Final_Floor;// LED는 마지막 층에 해당하는 번호를 킨다
    if(Start_Floor!=Final_Floor)// 만약 시작층과 도착층이 같지 않다면?
    {
      LCD_DisplayChar(5,9,0x30+Final_Floor);  // 문장 Start_Floor->Final_Floor 출력
      count=0;//count는 0
      b=1;// b=1로 설정한다.(b=1이면 SW7번이 눌리게 설정하였음.
      
    }
    else// 서로 같다면?(변화가 없어야한다.)
    {
      Final_Floor=swap;// 저장되었던 마지막층을 다시 불러온다.
      count=1; // 그대로 count는 1
    }
  }
}
void Execute(void)//실행모드
{
  DelayUS(1000);
  BEEP();
  LCD_SetTextColor(RGB_BLUE);// 글자색 : 파란색
  LCD_DisplayText(0,16,"EX");  // EX모드 출력
  
  GPIOG->ODR &= 0x00FE;//LED 0번은 OFF하고 나머지 LED는 현상태 유지 
  GPIOG->ODR |= 0x0080;//LED 7번 ON
  
  if(abs(Start_Floor-One_Elevator)<=abs(Start_Floor-Two_Elevator)) // Blue 엘레베이터 동작 조건   : 시작층과 왼쪽 엘레베이터의 차이보다 시작층과 오른쪽 엘레베이터의 층간 차이가 크거나 같을때
  {
    if(One_Elevator<Start_Floor) //시작점이 엘베보다 높으면 (UP)
    {
      LCD_SetTextColor(RGB_BLUE);// 글자색 :  파란색
      LCD_DisplayText(4,6,"L:U");  // L:U출력 (Left : Up)
      LCD_SetBrushColor(RGB_BLUE);// 브러쉬색 : 파란색
      LCD_DrawFillRect(15,32,10,72); // 사각형 내부
      LCD_SetBrushColor(RGB_YELLOW);// 브러쉬색 : 노란색(지우는 역할: 배경색과 같은색)
      LCD_DrawFillRect(15,32,10,72-(One_Elevator*12));// (지우는 역할: 배경색과 같은색)
      for(;One_Elevator<=Start_Floor; One_Elevator++)//왼쪽 엘레베이터가 시작층까지 도달할 때 까지 한층씩 증가
      {
        Hold();//Hold함수 출력확인
        LCD_SetBrushColor(RGB_BLUE);// 브러쉬색 : 파란색
        LCD_DrawFillRect(15,32,10,72); // 사각형 내부
        LCD_SetBrushColor(RGB_YELLOW);// 브러쉬색 : 노란색
        LCD_DrawFillRect(15,32,10,72-(One_Elevator*12)); // 사각형 내부
        DelayMS(500);// 딜레이 0.5초
      }
      LCD_SetBrushColor(RGB_BLUE);// 브러쉬색 : 파란색
      One_Elevator=Start_Floor;//시작층과 왼쪽 엘레베이터 층이 같다.
      LCD_DisplayText(4,6,"L:S");  // L:S 출력 (Left : Stop)
      DelayMS(500);// 딜레이 0.5초
    }
    
    else if(One_Elevator>Start_Floor)//엘베가 시작점보다 높으면 (DOWN)
    {
      LCD_SetTextColor(RGB_BLUE);// 글자색 :  파란색
      LCD_DisplayText(4,6,"L:D");   // L:D출력 (Left : Douw)
      LCD_SetBrushColor(RGB_BLUE);// 브러쉬색 : 파란색
      LCD_DrawFillRect(15,32,10,72); // 사각형 내부
      LCD_SetBrushColor(RGB_YELLOW);// 브러쉬색 : 노란색
      LCD_DrawFillRect(15,32,10,72-(One_Elevator*12)); // (지우는 역할: 배경색과 같은색)
      for(;One_Elevator>=Start_Floor; One_Elevator--)//왼쪽 엘레베이터가 시작층까지 도달할 때 까지 한층씩 감소
      {
        Hold();//Hold함수 출력확인
        LCD_SetBrushColor(RGB_BLUE);// 브러쉬색 : 파란색
        LCD_DrawFillRect(15,32,10,72); // 사각형 내부
        LCD_SetBrushColor(RGB_YELLOW);// 브러쉬색 : 노란색
        LCD_DrawFillRect(15,32,10,72-(One_Elevator*12)); // (지우는 역할: 배경색과 같은색)
        DelayMS(500);//딜레이 0.5초
      }
      LCD_SetBrushColor(RGB_BLUE);// 브러쉬색 : 파란색
      One_Elevator=Start_Floor;//시작층과 왼쪽 엘레베이터 층이 같다.
      LCD_DisplayText(4,6,"L:S"); // L:S 출력 (Left : Stop)
      DelayMS(500);//딜레이 0.5초
    }
    else//서로 같다면(변화는 없다.)
    {
      LCD_SetBrushColor(RGB_BLUE);// 글자색 :  파란색  
    }
    
    
    
    if(Start_Floor<Final_Floor)// 끝 층이 시작 층보다 높을 때(UP)
    {
      LCD_SetTextColor(RGB_BLUE);// 글자색 :  파란색
      LCD_DisplayText(4,6,"L:U");  // L:U출력 (Left : Up)
      LCD_SetBrushColor(RGB_BLUE);// 브러쉬색 : 파란색
      LCD_DrawFillRect(15,32,10,72); // 사각형 내부
      LCD_SetBrushColor(RGB_YELLOW);// 브러쉬색 : 노란색
      LCD_DrawFillRect(15,32,10,72-(Start_Floor*12));//(지우는 역할: 배경색과 같은색)
      for(;Start_Floor<=Final_Floor; Start_Floor++)//시작층을 끝점까지점점 한 층씩 증가시킨다
      {
        Hold();//Hold함수 출력확인
        
        LCD_SetBrushColor(RGB_BLUE);// 브러쉬색 : 파란색
        LCD_DrawFillRect(15,32,10,72); // 사각형 내부
        LCD_SetBrushColor(RGB_YELLOW);// 브러쉬색 : 노란색
        LCD_DrawFillRect(15,32,10,72-(Start_Floor*12)); //(지우는 역할: 배경색과 같은색)
        DelayMS(500);//딜레이 0.5초
      }
    }
    else if(Start_Floor>Final_Floor)// 끝 층이 시작 층보다 낮을때(DOWN)
    {
      LCD_SetTextColor(RGB_BLUE);// 글자색 : 파란색
      LCD_DisplayText(4,6,"L:D");  // L:D출력 (Left : Down)
      LCD_SetBrushColor(RGB_BLUE);// 브러쉬색 : 파란색
      LCD_DrawFillRect(15,32,10,72); // 사각형 내부
      LCD_SetBrushColor(RGB_YELLOW);// 브러쉬색 : 노란색
      LCD_DrawFillRect(15,32,10,72-(Start_Floor*12)); // (지우는 역할: 배경색과 같은색)
      for(;Start_Floor>=Final_Floor; Start_Floor--)//시작층을 끝점까지점점 한 층씩 증가시킨다
      {         
        Hold();//Hold 함수 출력 확인
        LCD_SetBrushColor(RGB_BLUE);//브러쉬 색 : 파란색
        LCD_DrawFillRect(15,32,10,72); // 사각형 내부
        LCD_SetBrushColor(RGB_YELLOW);//브러쉬 색 : 노란색
        LCD_DrawFillRect(15,32,10,72-(Start_Floor*12)); //지우는 역할: 배경색과 같은색)
        DelayMS(500);//딜레이 0.5초
      }
    }
    One_Elevator=Final_Floor;// 왼쪽 엘레베이터는 도착층이다.
    LCD_SetTextColor(RGB_BLUE);// 글자색 : 파란색
    LCD_DisplayText(0,16,"FL");  // FL모드
    LCD_DisplayText(4,6,"L:S");  //  L:S출력 (Left : Stop)
    LCD_SetTextColor(RGB_BLACK);// 글자색 : 
    GPIOG->ODR &= 0x0000;//LED전부 OFF
    GPIOG->ODR |= 0x0001;//LED0 ON
    BEEP3();// 도착시 Beep음 3번 연속 출력
  }
  
  
  else if(abs(Start_Floor-One_Elevator)>abs(Start_Floor-Two_Elevator))// RED 엘레베이터 동작 조건   : 시작층과 왼쪽 엘레베이터의 차이보다 시작층과 오른쪽 엘레베이터의 층간 차이가 작을 때
  {
    if(Two_Elevator<Start_Floor) //시작점이 엘베보다 높으면 (UP)
    {
      LCD_SetTextColor(RGB_RED);// 글자색 :  빨간색
      LCD_DisplayText(4,6,"R:U");  // R:U출력 (Right : Up)
      LCD_SetBrushColor(RGB_RED);//브러쉬 색 : 빨간색
      LCD_DrawFillRect(105,32,10,72); // 사각형 내부
      LCD_SetBrushColor(RGB_YELLOW);//브러쉬 색 : 노란색
      LCD_DrawFillRect(105,32,10,72-(Two_Elevator*12)); //(지우는 역할: 배경색과 같은색)
      for(;Two_Elevator<=Start_Floor; Two_Elevator++)//오른쪽 엘레베이터가 시작층까지 도달할 때 까지 한층씩 증가
      {
        Hold();//Hold함수 출력
        LCD_SetBrushColor(RGB_RED);// 브러쉬 색 : 빨간색
        LCD_DrawFillRect(105,32,10,72); // 사각형 내부
        LCD_SetBrushColor(RGB_YELLOW);//브러쉬 색 : 노란색
        LCD_DrawFillRect(105,32,10,72-(Two_Elevator*12)); //(지우는 역할: 배경색과 같은색)
        DelayMS(500);//딜레이 0.5초
      }
      LCD_SetTextColor(RGB_RED);// 글자색 : 빨간색/////////////////
      LCD_SetBrushColor(RGB_RED);//브러쉬 색 : 빨간색
      Two_Elevator=Start_Floor;// 오른쪽 엘레베이터는 시작층에 위치
      LCD_DisplayText(4,6,"R:S");  // R:S출력 (Right : Stop)
      DelayMS(500);// 딜레이 0.5초
    }
    else if(Two_Elevator>Start_Floor)//엘베가 시작점보다 높으면 (DOWN)
    {
      LCD_SetTextColor(RGB_RED);// 글자색 : 빨간색
      LCD_DisplayText(4,6,"R:D"); // R:D출력 (Right : Down)
      LCD_SetBrushColor(RGB_RED);//브러쉬 색 : 빨간색
      LCD_DrawFillRect(105,32,10,72); // 사각형 내부
      LCD_SetBrushColor(RGB_YELLOW);//브러쉬 색 : 노란색
      LCD_DrawFillRect(105,32,10,72-(Two_Elevator*12)); //(지우는 역할: 배경색과 같은색)
      for(;Two_Elevator>=Start_Floor; Two_Elevator--)//오른 쪽 엘레베이터가 시작층까지 도달할 때 까지 한층씩 증가
      {
        
        Hold();//Hold함수 출력 여부 확인
        LCD_SetBrushColor(RGB_RED);//브러쉬 색 : 빨간색
        LCD_DrawFillRect(105,32,10,72); // 사각형 내부
        LCD_SetBrushColor(RGB_YELLOW);//브러쉬 색 : 노란색
        LCD_DrawFillRect(105,32,10,72-(Two_Elevator*12)); //(지우는 역할: 배경색과 같은색)
        DelayMS(500);// 딜레이 0.5초
      }
      LCD_SetBrushColor(RGB_RED);//브러쉬 색 : 빨간색
      Two_Elevator=Start_Floor;//오른쪽 엘레베이터가 시작층에 도착
      LCD_SetTextColor(RGB_RED);// 글자색 : 빨간색/////////////////////////////
      LCD_DisplayText(4,6,"R:S");  // R:S출력 (Right : Stop)
      DelayMS(500);// 딜레이 0.5초
    }
    else//서로 같다면(변화는 없다.)
    {
      LCD_SetTextColor(RGB_RED);// 글자색 : 빨간색
    }
    
    
    if(Start_Floor<Final_Floor)// 끝 층이 시작 층보다 높을 때(UP)
    {
      LCD_SetTextColor(RGB_RED);// 글자색 : 빨간색
      LCD_DisplayText(4,6,"R:U");  // R:U출력 (Right : Up)
      LCD_SetBrushColor(RGB_RED);//브러쉬 색 : 빨간색
      LCD_DrawFillRect(105,32,10,72); // 사각형 내부
      LCD_SetBrushColor(RGB_YELLOW);//브러쉬 색 : 노란색
      LCD_DrawFillRect(105,32,10,72-(Start_Floor*12));  //(지우는 역할: 배경색과 같은색)
      for(;Start_Floor<=Final_Floor; Start_Floor++)//시작층을 끝점까지 점점 한 층씩 증가시킨다
      {
        Hold();//Hold 함수 확인
        LCD_SetBrushColor(RGB_RED);// 브러쉬색 : 빨간색
        LCD_DrawFillRect(105,32,10,72); // 사각형 내부
        LCD_SetBrushColor(RGB_YELLOW);// 브러쉬색 : 노란색
        LCD_DrawFillRect(105,32,10,72-(Start_Floor*12)); //(지우는 역할: 배경색과 같은색)
        DelayMS(500);// 딜레이 0.5초
      }
    }
    else if(Start_Floor>Final_Floor)// 끝 층이 시작 층보다 낮을때(DOWN)
    {
      LCD_SetTextColor(RGB_RED);// 글자색 : 빨간색
      LCD_DisplayText(4,6,"R:D");  // R:D출력 (Right : Down)
      LCD_SetBrushColor(RGB_RED);//브러쉬색 : 빨간색
      LCD_DrawFillRect(105,32,10,72); // 사각형 내부
      LCD_SetBrushColor(RGB_YELLOW);//브러쉬색 : 노란색
      LCD_DrawFillRect(105,32,10,72-(Start_Floor*12)); //(지우는 역할: 배경색과 같은색)
      for(;Start_Floor>=Final_Floor; Start_Floor--)//시작층을 끝점까지 점점 한 층씩 감소시킨다
      {
        Hold();//Hold 함수 확인
        LCD_SetBrushColor(RGB_RED);//브러쉬색 : 빨간색
        LCD_DrawFillRect(105,32,10,72); // 사각형 내부
        LCD_SetBrushColor(RGB_YELLOW);//브러쉬색 : 노란색
        LCD_DrawFillRect(105,32,10,72-(Start_Floor*12)); //(지우는 역할: 배경색과 같은색)
        DelayMS(500);//딜레이 0.5초
      }
    }
    Two_Elevator=Final_Floor;// 두번째 엘레베이터가 끝층에 도착
    LCD_SetTextColor(RGB_BLUE);// 글자색 : 파란색
    LCD_DisplayText(0,16,"FL");   // FL모드
    LCD_SetTextColor(RGB_RED);// 글자색 : 빨간색
    LCD_DisplayText(4,6,"R:S");  // R:S출력 (Right : Stop)
    LCD_SetTextColor(RGB_BLACK);// 글자색 : 검은색
    GPIOG->ODR &= 0x0000;//LED 모두 OFF
    GPIOG->ODR |= 0x0001;//LED 0번 출력
    BEEP3();// 도착시 Beep음 3번 연속 출력
  }
  
}
void Time_Delay(void)
{
  DelayUS(1000);
  BEEP();    
  LCD_SetTextColor(RGB_BLUE);// 글자색 :  파란색
  LCD_DisplayText(0,16,"HL");  // HL모드
  GPIOG->ODR &= 0x007E;// LED 0번 7번 OFF 나머지 이전 상태 유지
  for(int i=0; i<10; i++)
  {
    DelayMS(500);//0.5초씩 딜레이를 10번
    BEEP();//비프음 한번씩 출력
  }
  LCD_SetTextColor(RGB_BLUE);// 글자색 :  파란색
  LCD_DisplayText(0,16,"EX"); // EX모드
  GPIOG->ODR |= 0x0080;//LED 7번 ON
  
}
void Hold(void)
{        
  if(SW0_Flag)// 0번 스위치가 눌리면?
  {
    
    Time_Delay();//Time_Delay함수 출력
    SW0_Flag=0;//SW0의 플래그변수는 0이다.        
  }        
}
void DelayMS(unsigned short wMS)
{
  register unsigned short i;
  for (i=0; i<wMS; i++)
    DelayUS(1000);         		// 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
  volatile int Dly = (int)wUS*17;
  for(; Dly; Dly--);
}


/**************************************************************************
// 보충 설명자료
// 다음은 stm32f4xx.h에 있는 RCC관련 주요 선언문임 
#define HSE_STARTUP_TIMEOUT    ((uint16_t)0x05000)   // Time out for HSE start up 
typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

#define FLASH_BASE            ((uint32_t)0x08000000) // FLASH(up to 1 MB) base address in the alias region                          
#define CCMDATARAM_BASE       ((uint32_t)0x10000000) // CCM(core coupled memory) data RAM(64 KB) base address in the alias region   
#define SRAM1_BASE            ((uint32_t)0x20000000) // SRAM1(112 KB) base address in the alias region                              

#if defined(STM32F40_41xxx) 
#define SRAM2_BASE            ((uint32_t)0x2001C000) // SRAM2(16 KB) base address in the alias region                               
#define SRAM3_BASE            ((uint32_t)0x20020000) // SRAM3(64 KB) base address in the alias region                               
#endif   

#define PERIPH_BASE           ((uint32_t)0x40000000) // Peripheral base address in the alias region                                 
#define BKPSRAM_BASE          ((uint32_t)0x40024000) // Backup SRAM(4 KB) base address in the alias region                          

// Peripheral memory map  
#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000)
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000)
#define AHB2PERIPH_BASE       (PERIPH_BASE + 0x10000000)

// AHB1 peripherals  
#define GPIOA_BASE            (AHB1PERIPH_BASE + 0x0000)
#define GPIOB_BASE            (AHB1PERIPH_BASE + 0x0400)
#define GPIOC_BASE            (AHB1PERIPH_BASE + 0x0800)
#define GPIOD_BASE            (AHB1PERIPH_BASE + 0x0C00)
#define GPIOE_BASE            (AHB1PERIPH_BASE + 0x1000)
#define GPIOF_BASE            (AHB1PERIPH_BASE + 0x1400)
#define GPIOG_BASE            (AHB1PERIPH_BASE + 0x1800)
#define GPIOH_BASE            (AHB1PERIPH_BASE + 0x1C00)
#define GPIOI_BASE            (AHB1PERIPH_BASE + 0x2000)
#define GPIOJ_BASE            (AHB1PERIPH_BASE + 0x2400)
#define GPIOK_BASE            (AHB1PERIPH_BASE + 0x2800)
#define CRC_BASE              (AHB1PERIPH_BASE + 0x3000)
#define RCC_BASE              (AHB1PERIPH_BASE + 0x3800)
#define FLASH_R_BASE          (AHB1PERIPH_BASE + 0x3C00)
#define SYSCFG_BASE           (APB2PERIPH_BASE + 0x3800)
#define EXTI_BASE             (APB2PERIPH_BASE + 0x3C00)

typedef struct
{
__IO uint32_t CR;            // RCC clock control register, Address offset: 0x00  
__IO uint32_t PLLCFGR;       // RCC PLL configuration register, Address offset: 0x04  
__IO uint32_t CFGR;          // RCC clock configuration register, Address offset: 0x08  
__IO uint32_t CIR;           // RCC clock interrupt register, Address offset: 0x0C  
__IO uint32_t AHB1RSTR;      // RCC AHB1 peripheral reset register, Address offset: 0x10  
__IO uint32_t AHB2RSTR;      // RCC AHB2 peripheral reset register, Address offset: 0x14  
__IO uint32_t AHB3RSTR;      // RCC AHB3 peripheral reset register, Address offset: 0x18  
uint32_t      RESERVED0;     // Reserved, 0x1C                                                                    
__IO uint32_t APB1RSTR;      // RCC APB1 peripheral reset register, Address offset: 0x20  
__IO uint32_t APB2RSTR;      // RCC APB2 peripheral reset register, Address offset: 0x24  
uint32_t      RESERVED1[2];  // Reserved, 0x28-0x2C                                                                
__IO uint32_t AHB1ENR;       // RCC AHB1 peripheral clock register, Address offset: 0x30  
__IO uint32_t AHB2ENR;       // RCC AHB2 peripheral clock register, Address offset: 0x34  
__IO uint32_t AHB3ENR;       // RCC AHB3 peripheral clock register, Address offset: 0x38  
uint32_t      RESERVED2;     // Reserved, 0x3C                                                                     
__IO uint32_t APB1ENR;       // RCC APB1 peripheral clock enable register, Address offset: 0x40  
__IO uint32_t APB2ENR;       // RCC APB2 peripheral clock enable register, Address offset: 0x44  
uint32_t      RESERVED3[2];  // Reserved, 0x48-0x4C                                                                
__IO uint32_t AHB1LPENR;     // RCC AHB1 peripheral clock enable in low power mode register, Address offset: 0x50  
__IO uint32_t AHB2LPENR;     // RCC AHB2 peripheral clock enable in low power mode register, Address offset: 0x54  
__IO uint32_t AHB3LPENR;     // RCC AHB3 peripheral clock enable in low power mode register, Address offset: 0x58  
uint32_t      RESERVED4;     // Reserved, 0x5C                                                                     
__IO uint32_t APB1LPENR;     // RCC APB1 peripheral clock enable in low power mode register, Address offset: 0x60  
__IO uint32_t APB2LPENR;     // RCC APB2 peripheral clock enable in low power mode register, Address offset: 0x64  
uint32_t      RESERVED5[2];  // Reserved, 0x68-0x6C                                                                
__IO uint32_t BDCR;          // RCC Backup domain control register, Address offset: 0x70  
__IO uint32_t CSR;           // RCC clock control & status register, Address offset: 0x74  
uint32_t      RESERVED6[2];  // Reserved, 0x78-0x7C                                                                
__IO uint32_t SSCGR;         // RCC spread spectrum clock generation register, Address offset: 0x80  
__IO uint32_t PLLI2SCFGR;    // RCC PLLI2S configuration register, Address offset: 0x84  
__IO uint32_t PLLSAICFGR;    // RCC PLLSAI configuration register, Address offset: 0x88  
__IO uint32_t DCKCFGR;       // RCC Dedicated Clocks configuration register, Address offset: 0x8C  
} RCC_TypeDef;


typedef struct
{
__IO uint32_t ACR;      // FLASH access control register,   Address offset: 0x00  
__IO uint32_t KEYR;     // FLASH key register,              Address offset: 0x04  
__IO uint32_t OPTKEYR;  // FLASH option key register,       Address offset: 0x08  
__IO uint32_t SR;       // FLASH status register,           Address offset: 0x0C  
__IO uint32_t CR;       // FLASH control register,          Address offset: 0x10  
__IO uint32_t OPTCR;    // FLASH option control register ,  Address offset: 0x14  
__IO uint32_t OPTCR1;   // FLASH option control register 1, Address offset: 0x18  
} FLASH_TypeDef;

typedef struct
{
__IO uint32_t MODER;    // GPIO port mode register,               Address offset: 0x00       
__IO uint32_t OTYPER;   // GPIO port output type register,        Address offset: 0x04       
__IO uint32_t OSPEEDR;  // GPIO port output speed register,       Address offset: 0x08       
__IO uint32_t PUPDR;    // GPIO port pull-up/pull-down register,  Address offset: 0x0C       
__IO uint32_t IDR;      // GPIO port input data register,         Address offset: 0x10       
__IO uint32_t ODR;      // GPIO port output data register,        Address offset: 0x14       
__IO uint16_t BSRRL;    // GPIO port bit set/reset low register,  Address offset: 0x18       
__IO uint16_t BSRRH;    // GPIO port bit set/reset high register, Address offset: 0x1A       
__IO uint32_t LCKR;     // GPIO port configuration lock register, Address offset: 0x1C       
__IO uint32_t AFR[2];   // GPIO alternate function registers,     Address offset: 0x20-0x24  
} GPIO_TypeDef;

typedef struct
{
__IO uint32_t IMR;    // EXTI Interrupt mask register, Address offset: 0x00 
__IO uint32_t EMR;    // EXTI Event mask register, Address offset: 0x04 
__IO uint32_t RTSR;   // EXTI Rising trigger selection register,  Address offset: 0x08
__IO uint32_t FTSR;   // EXTI Falling trigger selection register, Address offset: 0x0C
__IO uint32_t SWIER;  // EXTI Software interrupt event register,  Address offset: 0x10 
__IO uint32_t PR;     // EXTI Pending register, Address offset: 0x14 
} EXTI_TypeDef;

typedef struct
{
__IO uint32_t MEMRMP;       // SYSCFG memory remap register, Address offset: 0x00 
__IO uint32_t PMC;          // SYSCFG peripheral mode configuration register, Address offset: 0x04
__IO uint32_t EXTICR[4];    // SYSCFG external interrupt configuration registers, Address offset: 0x08-0x14 
uint32_t      RESERVED[2];  // Reserved, 0x18-0x1C     
__IO uint32_t CMPCR;        // SYSCFG Compensation cell control register,Address offset: 0x20

} SYSCFG_TypeDef;

#define GPIOA 	((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB	((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC   ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD   ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE  	((GPIO_TypeDef *) GPIOE_BASE)
#define GPIOF   ((GPIO_TypeDef *) GPIOF_BASE)
#define GPIOG   ((GPIO_TypeDef *) GPIOG_BASE)
#define GPIOH   ((GPIO_TypeDef *) GPIOH_BASE)
#define GPIOI   ((GPIO_TypeDef *) GPIOI_BASE)
#define GPIOJ   ((GPIO_TypeDef *) GPIOJ_BASE)
#define GPIOK   ((GPIO_TypeDef *) GPIOK_BASE)

#define CRC             ((CRC_TypeDef *) CRC_BASE)
#define RCC             ((RCC_TypeDef *) RCC_BASE)
#define FLASH           ((FLASH_TypeDef *) FLASH_R_BASE)

#define SYSCFG          ((SYSCFG_TypeDef *) SYSCFG_BASE)
#define EXTI            ((EXTI_TypeDef *) EXTI_BASE)

#define FLASH_ACR_PRFTEN             ((uint32_t)0x00000100)
#define FLASH_ACR_ICEN               ((uint32_t)0x00000200)
#define FLASH_ACR_DCEN               ((uint32_t)0x00000400)
#define FLASH_ACR_ICRST              ((uint32_t)0x00000800)
#define FLASH_ACR_DCRST              ((uint32_t)0x00001000)
#define FLASH_ACR_BYTE0_ADDRESS      ((uint32_t)0x40023C00)
#define FLASH_ACR_BYTE2_ADDRESS      ((uint32_t)0x40023C03)

#define FLASH_ACR_LATENCY_5WS        ((uint32_t)0x00000005)

typedef struct {
__IO uint32_t ISER[8];  // Offset: 0x000 Interrupt Set Enable Register    
uint32_t RESERVED0[24];                                   
__IO uint32_t ICER[8];  // Offset: 0x080 Interrupt Clear Enable Register  
uint32_t RSERVED1[24];                                    
__IO uint32_t ISPR[8];  // Offset: 0x100 Interrupt Set Pending Register   
uint32_t RESERVED2[24];                                   
__IO uint32_t ICPR[8];  // Offset: 0x180 Interrupt Clear Pending Register
uint32_t RESERVED3[24];                                   
__IO uint32_t IABR[8];  // Offset: 0x200 Interrupt Active bit Register      
uint32_t RESERVED4[56];                                   
__IO uint8_t  IP[240];  // Offset: 0x300 Interrupt Priority Register (8Bit) 
uint32_t RESERVED5[644];                                  
__O  uint32_t STIR;  // Offset: 0xE00 Software Trigger Interrupt Register    
}  NVIC_Type; 

// Memory mapping of Cortex-M4 Hardware 
#define SCS_BASE     (0xE000E000)    // System Control Space Base Address 
#define NVIC_BASE   (SCS_BASE +  0x0100)  // NVIC Base Address  
#define NVIC        ((NVIC_Type *)  NVIC_BASE) // NVIC configuration struct                                           

*/ 