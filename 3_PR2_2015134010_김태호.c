#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"


#define NAVI_PUSH	0x03C0  //PI5 0000 0011 1100 0000 
#define NAVI_UP	0x03A0  //PI6 0000 0011 1010 0000 
#define NAVI_DOWN    0x0360  //PI7 0000 0011 0110 0000 
#define NAVI_RIGHT	0x02E0  //PI8 0000 0010 1110 0000 
#define NAVI_LEFT	0x01E0  //PI9 0000 0001 1110 0000 

void _GPIO_Init(void);
//uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);

void BEEP(void);
void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
//void Player(void);
void _EXTI_Init(void);
void UART5_Init(void);
void USART_BRR_Configuration(uint32_t USART_BaudRate);
void UART5_IRQHandler(void);
void SerialPutChar(uint8_t c);
void Serial_PutString(char *s);
void joystic(void);
uint8_t	SW0_Flag, SW1_Flag,SW5_Flag,SW6_Flag, SW7_Flag,SW0_Flag1,SW1_Flag1,SW2_Flag1,SW3_Flag1,SW4_Flag1; // 앞에 플래그 변수는 인터럽트 관련 플래그 변수, 플래그 뒤에 1이 붙은것은 나비스위치 관련 플래그 변수.
uint8_t count=0; // (카운트 = 0 : 처음 시작) (카운트 = 1 : 빨간돌 차례 )  (카운트 =2 : 파란돌 차례)
int u=5,t=5, x=5,y=5,RedplayerW=0, BlueplayerW=0,Turn=0; //u,t는 빨간돌의 (x,y)좌표이며 x,y가 파란돌의 (x,y)좌표이다.
int i[10][10] = {0}; // 중복 방지 배열

int main(void)
{
  _GPIO_Init(); // GPIO (LED & SW) 초기화
  LCD_Init();	// LCD 모듈 초기화
  _EXTI_Init();  //인터럽트
  UART5_Init();
  Fram_Init();        // FRAM 초기화  //반드시 가져와야함
  Fram_Status_Config(); // Fram_Satus_config();반드시 가져와야함
  DelayMS(100);
  
  DisplayInitScreen();	// LCD 초기화면
  // BEEP();
  GPIOG->ODR = 0xFF00;	// LED 초기값: LED0~7 Off
  //우리 Fram을 지우면 0으로 초기화 되어있음
  //Fram_Write(주소(0~8191),데이터(1Byte)
  //Fram_Read (주소) -  Return(1Byte)
  //LCD_DisplayChar(1,0,Fram_Read(100)+0x30); //FRAM 100번지 저장된 data(1byte) 읽어 
  
  while(1)
  { 
    if(SW0_Flag) // 스위치 0번을 눌렀을 때
    {
      if(Turn==0 && (count==0  || count==2))  //처음 시작하거나 빨간돌의 순서가 왔을 때
      {
        GPIOG->ODR = 0xFF00;	// LED 초기값: LED0~7 Off
        GPIOG->ODR |= 0x0001; // LED 0 ON
        LCD_SetTextColor(RGB_RED);// 글자색 : Red
        LCD_DrawText(2,116,"*"); //빨간색
        LCD_DrawText(145,116," "); //빨간색
        LCD_SetBrushColor(RGB_RED);
        Serial_PutString("1");
        Turn=1; //빨간 돌을 제어
        count=1;//빨간 돌을 제어
      }
      SW0_Flag = 0;
      SW1_Flag = 0; // 1번과 6번을 0으로 계속 해주는 이유는 중간에 켜지는 것을 방지하기 위함이다.
      SW6_Flag = 0;
    }
    else if(SW7_Flag)
    {
      if(Turn==0 && (count==0  || count==1)) //처음 시작하거나 파란돌의 순서가 왔을 때
      {
        GPIOG->ODR = 0xFF00;	// LED 초기값: LED0~7 Off
        GPIOG->ODR |= 0x0080; // LED 7 ON
        LCD_SetTextColor(RGB_BLUE);// 글자색 : BLUE
        LCD_DrawText(145,116,"*"); //파란색
        LCD_DrawText(2,116," "); //파란색
        LCD_SetBrushColor(RGB_BLUE);
        Serial_PutString("2");
        Turn=3;// 파란돌을 제어
        count=2;// 파란돌을 제어
      }
      SW7_Flag = 0;
      SW1_Flag = 0; // 1번과 6번을 0으로 계속 해주는 이유는 중간에 켜지는 것을 방지하기 위함이다.
      SW6_Flag = 0;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////파란돌 빨간돌 선택 구간 ///////////////////////////////////////////////////////////////////////
    
    joystic(); // 조이스틱 설정
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    if(SW1_Flag && Turn == 0 && count==1) // 빨간색이 경기에서 우승하고 스위치 1번을 눌렀을때
    {
      LCD_SetTextColor(RGB_RED);
      RedplayerW++; // 이긴횟수 증가
      if(RedplayerW>9) // 만약 이긴횟수가 9보다 커지게 되면?
      {
        RedplayerW=0; //0으로 초기화
      }
      Turn=0; // 초기상태 되돌림
      count=0; // 초기상태 되돌림
      Fram_Write(300,RedplayerW);  // 변수값을 301번지에 저장
      RedplayerW=Fram_Read(300);// 301번지 값을 읽어온다.
      LCD_DrawChar(58,116,RedplayerW+0x30); //빨간색
      DelayMS(500); //5초 대기
      u=y=x=t=5; // 초기 좌표값을 빨간돌 파란돌 모두 5,5로 초기화
      DisplayInitScreen(); // 초기화면
      for(int j=0; j<10; j++)
        for(int k=0; k<10; k++)
          i[j][k] = 0; // 중복 방지 배열 초기화
      SW1_Flag = 0; 
      
    }
    
    else if(SW6_Flag && Turn == 0 && count==2)  //파란색이 경기에서 우승하고 스위치 6번을 눌렀을 때
    {
      LCD_SetTextColor(RGB_BLUE);// 글자색 : BLUE
      BlueplayerW++;// 이긴횟수 증가
      if(BlueplayerW>9)// 만약 이긴횟수가 9보다 커지게 되면?
      {
        BlueplayerW=0;//0으로 초기화
      }
      
      Turn=0; // 초기상태 되돌림
      count=0; // 초기상태 되돌림
      Fram_Write(301,BlueplayerW); // 변수값을 301번지에 저장
      BlueplayerW=Fram_Read(301);// 301번지 값을 읽어온다.
      LCD_DrawChar(88,116,BlueplayerW+0x30); //빨간색
      DelayMS(500);
      u=y=x=t=5; // 초기 좌표값을 빨간돌 파란돌 모두 5,5로 초기화
      DisplayInitScreen();// 초기화면
      for(int j=0; j<10; j++)
        for(int k=0; k<10; k++)
          i[j][k] = 0; // 중복 방지 배열 초기화
      SW6_Flag = 0;
    }
  }
}



void _EXTI_Init(void)
{
  
  RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
  RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
  GPIOH->MODER 	&= 0x0000FFFF;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
  
  SYSCFG->EXTICR[2] |= 0x0077; 	// EXTI8에 대한 소스 입력은 GPIOH로 설정 (EXTICR3) (reset value: 0x0000)//PH8->EXTI8로 바꿔주는 역할
  EXTI->FTSR |= 0x0300;		// Falling Trigger Enable  (EXTI8:PH8)//눌렀을때 폴링
  EXTI->IMR  |= 0x0300;  	// EXTI8,9 인터럽트 mask (Interrupt Enable) //IMR 이 담당한다.
  NVIC->ISER[0] |= ( 1<<23  );   // Enable Interrupt EXTI8,9 Vector table Position 참조 //ISER[0] ~ [7]까지 있다. [0]은 (0번부터 31번까지 담당한다. 32는 [1]에 있다.) 1개마다 32bit
  //NVIC->ISER[0] |= ( 0x00800000  );
  
  SYSCFG->EXTICR[3] |= 0x7770; 	//EXIT15,14에 대한 소스 입력은 GPIOH로 설정// PH15->EXIT15	
  EXTI->FTSR |= 0xE000;		// Falling Trigger Enable  (15,14,13번)//눌렀을때 폴링
  EXTI->IMR  |= 0xE000;  	// EXTI15,14 인터럽트 mask (Interrupt Enable) //IMR 이 담당한다.// 0xE000 =0b 1110 0000 0000 0000
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
  GPIOH->MODER	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
  GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state
  
  // Buzzer (GPIO F) 설정 
  RCC->AHB1ENR    |=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
  GPIOF->MODER    |=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
  GPIOF->OTYPER 	&= ~0x00000200;	// GPIOF 9 : Push-pull  	
  GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
  
  //Joy Stick SW(PORT I) 설정
  RCC->AHB1ENR 	|= 0x0100;	// RCC_AHB1ENR GPIOI Enable
  GPIOI->MODER   &= ~0x000FFC00;	// GPIOI 5~9 : Input mode (reset state)
  GPIOI->PUPDR    &= ~0x000FFC00;	// GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)
  
}	
void EXTI9_5_IRQHandler(void)		// EXTI 5~9 인터럽트 핸들러 //얘는 이미 선언되있어서 그냥 쓰면 된다.
{
  if(EXTI->PR & 0x0100) 		// EXTI8 Interrupt Pending?
  {
    EXTI->PR |= 0x0100; 		// Pending bit Clear//PR = Pending Register 
    SW0_Flag = 1;
  }
  if(EXTI->PR & 0x0200) 		// EXTI9 Interrupt Pending?
  {
    EXTI->PR |= 0x0200; 		// Pending bit Clear//PR = Pending Register 
    SW1_Flag = 1;
  }
}
void EXTI15_10_IRQHandler(void)		// EXTI 15~10 인터럽트 핸들러 //얘는 이미 선언되있어서 그냥 쓰면 된다.
{
  if(EXTI->PR & 0x8000) 		// EXTI15 Interrupt Pending?
  {
    EXTI->PR |= 0x8000; 		// Pending bit Clear//PR = Pending Register 
    SW7_Flag = 1;
  }
  if(EXTI->PR & 0x4000) 		// EXTI14 Interrupt Pending?
  {
    EXTI->PR |= 0x4000; 		// Pending bit Clear//PR = Pending Register 
    SW6_Flag = 1;
  }
  if(EXTI->PR & 0x2000) 		// EXTI13 Interrupt Pending?
  {
    EXTI->PR |= 0x2000; 		// Pending bit Clear//PR = Pending Register 
    SW5_Flag = 1;
  }
}
void DisplayInitScreen(void) // 초기상태
{
  GPIOG->ODR = 0xFF00;	// LED 초기값: LED0~7 Off
  LCD_Clear(RGB_YELLOW);	// 화면 클리어
  LCD_SetFont(&Gulim8);	// 폰트 : 굴림 8
  
  LCD_SetBackColor(RGB_YELLOW);// 글자배경색 : Green
  LCD_SetTextColor(RGB_BLACK);// 글자색 : Black
  LCD_DisplayText(0,1,"Mecha-OMOK");  // Title
  LCD_SetPenColor(RGB_BLACK);
  
  RedplayerW=Fram_Read(300); //빨간돌 우승횟수는 300번지에 저장
  BlueplayerW=Fram_Read(301);//파란돌 우승횟수는 301번지에 저장
  for(int i=1; i<=10; i++)//10X10줄 선
  {
    LCD_DrawHorLine(35,19+i*9,81); // 가로선 10개 : 간격 9
    LCD_DrawVerLine(26+i*9,28,81); // 세로선 10개 : 간격 9
  }
  LCD_DrawText(20,15,"0");
  LCD_DrawText(20,65,"5");
  LCD_DrawText(20,105,"9");
  LCD_DrawText(75,15,"5");
  LCD_DrawText(115,15,"9"); // 칸수 표현
  LCD_DrawText(70,116,"VS");
  
  LCD_SetTextColor(RGB_BLUE);// 글자색 : Blue
  LCD_DrawText(105,116,"( , )");//파란색
  Fram_Write(301,BlueplayerW); // 301번지에 파란 돌의 우숭횟수를 읽어온다
  LCD_DrawChar(88,116,Fram_Read(301)+0x30); //파란색
  LCD_DrawChar(111,116,0x30+u);//파란색
  LCD_DrawChar(128,116,0x30+t);//파란색
  LCD_DrawText(145,116," "); //빨간색
  
  LCD_SetTextColor(RGB_RED);// 글자색 : Red
  LCD_DrawText(12,116,"( , )"); //빨간색
  Fram_Write(300,RedplayerW);
  LCD_DrawChar(58,116,Fram_Read(300)+0x30); //빨간색
  LCD_DrawChar(18,116,0x30+u);//빨간색
  LCD_DrawChar(36,116,0x30+t);//빨간색
  LCD_DrawText(2,116," "); //빨간색
  LCD_SetBrushColor(RGB_BLACK);//검은색
  LCD_DrawFillRect(78,71,6,6);
}
//uint8_t key_flag = 0;
/*uint16_t KEY_Scan(void)	// input key SW0 - SW7 
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
}*/

void BEEP(void)			/* beep for 30 ms */
{ 	
  GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
  DelayMS(30);		// Delay 30 ms
  GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}

uint8_t joy_flag = 0;
uint16_t JOY_Scan(void)	// input joy stick NAVI_* 
{ 
  uint16_t key;
  key = GPIOI->IDR & 0x03E0;	// any key pressed ?
  if(key == 0x03E0)		// if no key, check key off
  {  	if(joy_flag == 0)
    return key;
  else
  {	DelayMS(10);
  joy_flag = 0;
  return key;
  }
  }
  else				// if key input, check continuous key
  {	if(joy_flag != 0)	// if continuous key, treat as no key input
    return 0x03E0;
  else			// if new key,delay for debounce
  {	joy_flag = 1;
  DelayMS(10);
  return key;
  }
  }
}


void joystic(void) // 나비 스위치 함수
{
  switch(JOY_Scan())
  {
  case NAVI_LEFT: 	// NAVI_LEFT
    Serial_PutString("l"); 
    if(Turn==1) // 빨간색으로 했을 때
    {
      
      LCD_SetTextColor(RGB_RED);// 글자색 : Red
      LCD_SetBrushColor(RGB_RED);
      
      if(u<=0) // 만약 빨간돌의 x축 값이 0보다 작다면?
      {     
        u=0;
        LCD_DrawChar(18,116,0x30+u);//빨간색
      }
      else //그렇지 않다면?
      {      
        u--; // 좌표값을 1만큼 빼주면 된다.
        BEEP();
        LCD_DrawChar(18,116,0x30+u);//빨간색   
      }   
    }
    
    else if(Turn==3) //파란색으로 했을 때
    {
      LCD_SetTextColor(RGB_BLUE);// 글자색 : Blue
      LCD_SetBrushColor(RGB_BLUE);
      
      if(x<=0)// 만약 파란돌의 x축 값이 0보다 작다면?
      {     
        x=0;
        LCD_DrawChar(111,116,0x30+x);//파란색
      }
      else //그렇지 않다면?
      {     
        BEEP();
        x--;// 좌표값을 1만큼 빼주면 된다.
        LCD_DrawChar(111,116,0x30+x);//파란색
      }
      
    }
    
    break;
    
  case NAVI_RIGHT: 	// NAVI_RIGHT
    Serial_PutString("r");
    if(Turn==1) // 빨간색으로 했을 때
    {
      LCD_SetTextColor(RGB_RED);// 글자색 : Red
      LCD_SetBrushColor(RGB_RED);
      if(u>=9)// 만약 빨간돌의 x축 값이 9보다 크다면?
      {     
        u=9;
        LCD_DrawChar(18,116,0x30+u);//빨간색
      }
      else //그렇지 않다면?
      {     
        BEEP();
        u++;// 좌표값을 1만큼 더해주면 된다.
        LCD_DrawChar(18,116,0x30+u);//빨간색
      }
    }
    else if(Turn==3)
    {
      LCD_SetTextColor(RGB_BLUE);// 글자색 : Blue
      LCD_SetBrushColor(RGB_BLUE);
      if(x>=9)// 만약 파란돌의 x축 값이 9보다 크다면?
      {     
        x=9;
        LCD_DrawChar(111,116,0x30+x);//파란색
        
      }
      else //그렇지 않다면?
      {     
        BEEP();
        x++;// 좌표값을 1만큼 더해주면 된다.
        LCD_DrawChar(111,116,0x30+x);//파란색
      }
      
    }
    break;
    
  case NAVI_UP: 	// NAVI_UP
    Serial_PutString("u");
    if(Turn==1) // 빨간색으로 했을 때
    {
      LCD_SetTextColor(RGB_RED);// 글자색 : Red
      LCD_SetBrushColor(RGB_RED);
      if(t<=0)// 만약 빨간돌의 y축 값이 0보다 작다면?
      {
        t=0; 
        LCD_DrawChar(36,116,0x30+t); //빨간색
      }
      else//그렇지 않다면? 
      {     
        BEEP();
        t--;// 좌표값을 1만큼 빼주면 된다.
        LCD_DrawChar(36,116,0x30+t); //빨간색
      }
    }
    else if(Turn==3)
    {
      LCD_SetTextColor(RGB_BLUE);// 글자색 : Blue
      LCD_SetBrushColor(RGB_BLUE);
      if(y<=0)// 만약 파란돌의 y축 값이 0보다 작다면?
      {     
        y=0;
        LCD_DrawChar(128,116,0x30+y);//파란색
      }
      else 
      {     
        BEEP();
        y--;// 좌표값을 1만큼 빼주면 된다.
        LCD_DrawChar(128,116,0x30+y);//파란색
      }
      
    }
    
    break;
    
  case NAVI_DOWN: 	// NAVI_UP
    Serial_PutString("d");
    if(Turn==1) // 빨간색으로 했을 때
    {                        
      LCD_SetTextColor(RGB_RED);// 글자색 : Red
      LCD_SetBrushColor(RGB_RED);
      if(t>=9)// 만약 빨간돌의 y축 값이 9보다 크다면?
      { 
        t=9;
        LCD_DrawChar(36,116,0x30+t); //빨간색
      }
      else //그렇지 않다면?
      {     
        BEEP();
        t++;// 좌표값을 1만큼 더해주면 된다. 
        LCD_DrawChar(36,116,0x30+t); //빨간색
      }
    }
    else if(Turn==3)
    {
      LCD_SetTextColor(RGB_BLUE);// 글자색 : Blue
      LCD_SetBrushColor(RGB_BLUE);
      if(y>=9)// 만약 파란돌의 y축 값이 9보다 크다면?
      {     
        y=9;
        LCD_DrawChar(128,116,0x30+y);//파란색
      }
      else 
      {     
        BEEP();
        y++;/// 좌표값을 1만큼 더해주면 된다.
        LCD_DrawChar(128,116,0x30+y);//파란색
      }
    }
    
    break;
    
  case NAVI_PUSH:
    Serial_PutString("p");
    if(Turn==1) //빨간 돌이였다면?
    {
      if(i[u][t]==1) //중복 점 이라면?
      {
        Turn=1; // 원상태(놓기 직전 상태) 복귀
        count=1;
      }
      
      else
      {
        BEEP();
        DelayMS(100);
        BEEP();
        DelayMS(100);
        BEEP();///////////////////////////////비프음 세번 울리기//////////////////////////////
        LCD_DrawFillRect(32+u*9,21+t*9,9,9);//빨간색
        i[u][t]=1; // 놓이는 순간 중복을 방지하기 위해 사용
        Turn=0; // 
        count=1;
      }      
    }
    
    else if(Turn==3) // 파란 돌 이였다면 ?
    {
      if(i[x][y]==1) //원상태(놓기 직전 상태) 복귀
      {
        Turn=3;
        count=2;
      }
      else
      {
        BEEP();
        DelayMS(100);
        BEEP();
        DelayMS(100);
        BEEP();///////////////////////////////비프음 세번 울리기//////////////////////////////
        LCD_DrawFillRect(32+x*9,21+y*9,9,9);//파란색
        i[x][y]=1;// 놓인돌 방지
        Turn=0; //다시 돌 설정
        count=2; // 파란돌을 설정
      }
    }
    
    break;
  }
  
}
void UART5_Init(void) //
{
  // UART5 : TX(PC12)
  RCC->AHB1ENR	|= 0x00000004;	// RCC_AHB1ENR GPIOC Enable
  GPIOC->MODER	|= 0x02000000;	// GPIOC PIN12 Output Alternate function mode					
  GPIOC->OSPEEDR	|= 0x03000000;	// GPIOC PIN12 Output speed (100MHz Very High speed)
  GPIOC->OTYPER	|= 0x00000000;	// GPIOC PIN12 Output type push-pull (reset state)
  GPIOC->PUPDR	|= 0x01000000;	// GPIOC PIN12 Pull-up
  GPIOC->AFR[1]	|= 0x00080000;	// Connect GPIOC pin12 to AF8(USART5)
  
  /*
  UART5 : RX(PA10)
  GPIOA->MODER 	|= 0x200000;	// GPIOA PIN10 Output Alternate function mode
  GPIOA->OSPEEDR	|= 0x00300000;	// GPIOA PIN10 Output speed (100MHz Very High speed
  GPIOA->AFR[1]	|= 0x700;	//Connect GPIOA pin10 to AF7(USART1)
  */
  RCC->AHB1ENR |= 0x08 ;
  GPIOD->MODER 	|= 0x020;	// GPIOA PIN10 Output Alternate function mode
  GPIOD->OSPEEDR	|= 0x00000030;	// GPIOA PIN10 Output speed (100MHz Very High speed
  GPIOD->AFR[0]	|= 0x0800;	//Connect GPIOA pin10 to AF7(USART1)
  
  
  
  
  RCC->APB1ENR	|= 0x00100000;	// RCC_APB2ENR UART5 Enable
  
  USART_BRR_Configuration(19200); // USART Baud rate Configuration
  
  USART1->CR1	&= ~USART_CR1_M;	// USART_WordLength 8 Data bit
  UART5->CR1	|= USART_CR1_M;	// USART_WordLength 8 Data bit
  UART5->CR1	&= ~USART_CR1_PCE ;	// USART_Parity_No
  
  UART5->CR1	|= USART_CR1_RE;	// 0x0004, USART_Mode_RX Enable
  UART5->CR1	|= USART_CR1_TE ;	// 0x0008, USART_Mode_Tx Enable
  UART5->CR2	&= ~USART_CR2_STOP;	// USART_StopBits_1
  UART5->CR3	= 0x0000;	// No HardwareFlowControl, No DMA
  
  UART5->CR1 	|= USART_CR1_RXNEIE;	//  0x0020, RXNE interrupt Enable
  // UART5->CR1 	|= USART_CR1_TXEIE;	//  0x0020, TXEIE interrupt Enable              // 이게 1이면 외부 인터럽트 바로발생 ㅇㅋ? 
  
  NVIC->ISER[1]	|= (1 << (53-32)); 	// Enable Interrupt UART5 (NVIC 37번)
  UART5->CR1 	|= USART_CR1_UE;	//  0x2000, UART5 Enable
}

void UART5_IRQHandler(void)
{       
  
  if ( (UART5->SR & USART_SR_RXNE) )
  {
    uint16_t ch;
    
    ch = (uint16_t)(UART5->DR & (uint16_t)0x01FF);	// 수신된 문자 저장  0000000111111111
    //SerialPutChar(ch);     // 돌려주기 에코                                            0000000000110001
    if(ch == 'l') // 0x6C = l
    {
      //LCD_SetTextColor(RGB_BLUE);
      if(Turn==1) // 빨간색으로 했을 때
      {
        LCD_SetTextColor(RGB_BLUE);// 글자색 : Red
        LCD_SetBrushColor(RGB_BLUE);
        if(u<=0) // 만약 빨간돌의 x축 값이 0보다 작다면?
        {     
          u=0;
          LCD_DrawChar(111,116,0x30+u);//파란색
        }
        else //그렇지 않다면?
        {     
          u--; // 좌표값을 1만큼 빼주면 된다.
          BEEP();
          LCD_DrawChar(111,116,0x30+u);//파란색
        }
      }
      else if(Turn==3) //파란색으로 했을 때
      {
        LCD_SetTextColor(RGB_RED);// 글자색 : Blue
        LCD_SetBrushColor(RGB_RED);
        if(x<=0)// 만약 파란돌의 x축 값이 0보다 작다면?
        {     
          x=0;
          LCD_DrawChar(18,116,0x30+x);//빨간색
        }
        else //그렇지 않다면?
        {     
          BEEP();
          x--;// 좌표값을 1만큼 빼주면 된다.
          LCD_DrawChar(18,116,0x30+x);//빨간색
        }
      }
    }
    
    
    
    if(ch == 'r')  // 0x72 ='r'
    {
      //LCD_SetTextColor(RGB_BLUE);
      if(Turn==1) // 빨간색으로 했을 때
      {
        LCD_SetTextColor(RGB_BLUE);// 글자색 : Red
        LCD_SetBrushColor(RGB_BLUE);
        if(u>=9) 
        {     
          u=9;
          LCD_DrawChar(111,116,0x30+u);
        }
        else //그렇지 않다면?
        {     
          u++; 
          BEEP();
          LCD_DrawChar(111,116,0x30+u);
        }
      }
      else if(Turn==3) 
      {
        LCD_SetTextColor(RGB_RED);
        LCD_SetBrushColor(RGB_RED);
        if(x>=9)
        {     
          x=9;
          LCD_DrawChar(18,116,0x30+x);
        }
        else //그렇지 않다면?
        {     
          BEEP();
          x++;
          LCD_DrawChar(18,116,0x30+x);
        }
      }
    }
    
    
    if(ch == 'u')  // 0x75 = 'u'
    {
      if(Turn==1) // 빨간색으로 했을 때
      {                        
        LCD_SetTextColor(RGB_BLUE);// 글자색 : Red
        LCD_SetBrushColor(RGB_BLUE);
        if(t<=0)// 만약 빨간돌의 y축 값이 9보다 크다면?
        { 
          t=0;
          LCD_DrawChar(128,116,0x30+t); //빨간색
        }
        else //그렇지 않다면?
        {     
          BEEP();
          t--;// 좌표값을 1만큼 더해주면 된다. 
          LCD_DrawChar(128,116,0x30+t); //빨간색
        }
      }
      else if(Turn==3)
      {
        LCD_SetTextColor(RGB_RED);// 글자색 : Blue
        LCD_SetBrushColor(RGB_RED);
        if(y<=0)// 만약 파란돌의 y축 값이 9보다 크다면?
        {     
          y=0;
          LCD_DrawChar(36,116,0x30+y);//파란색
        }
        else 
        {     
          BEEP();
          y--;/// 좌표값을 1만큼 더해주면 된다.
          LCD_DrawChar(36,116,0x30+y);//파란색
        }
      }
    }
    if(ch == 'd') //0x64= 'd'
    {
      if(Turn==1) // 빨간색으로 했을 때
      {
        LCD_SetTextColor(RGB_BLUE);// 글자색 : Red
        LCD_SetBrushColor(RGB_BLUE);
        if(t>=9)// 만약 빨간돌의 y축 값이 0보다 작다면?
        {
          t=9; 
          LCD_DrawChar(128,116,0x30+t);//파란색
        }
        else//그렇지 않다면? 
        {     
          BEEP();
          t++;// 좌표값을 1만큼 빼주면 된다.
          LCD_DrawChar(128,116,0x30+t);//파란색
        }
      }
      else if(Turn==3)
      {
        LCD_SetTextColor(RGB_BLUE);// 글자색 : Blue
        LCD_SetBrushColor(RGB_BLUE);
        if(y>=9)// 만약 파란돌의 y축 값이 0보다 작다면?
        {     
          y=9;
          LCD_DrawChar(36,116,0x30+y);//파란색
        }
        else 
        {     
          BEEP();
          y++;// 좌표값을 1만큼 빼주면 된다.
          LCD_DrawChar(36,116,0x30+y);//파란색
        }
        
        
      }
    }
    
    if(ch == 'p') //ox70 = 'p'
    {
      if(Turn==1) //빨간 돌이였다면?
      {
        if(i[u][t]==1) //중복 점 이라면?
        {
          Turn=1; // 원상태(놓기 직전 상태) 복귀
          count=1;
        }
        
        else
        {
          BEEP();
          DelayMS(100);
          BEEP();
          DelayMS(100);
          BEEP();///////////////////////////////비프음 세번 울리기//////////////////////////////
          LCD_SetBrushColor(RGB_BLUE);
          LCD_DrawFillRect(32+u*9,21+t*9,9,9);//빨간색
          i[u][t]=1; // 놓이는 순간 중복을 방지하기 위해 사용
          Turn=0; // 
          count=1;
        }      
      }
      
      else if(Turn==3) // 파란 돌 이였다면 ?
      {
        if(i[x][y]==1) //원상태(놓기 직전 상태) 복귀
        {
          Turn=3;
          count=2;
        }
        else
        {
          BEEP();
          DelayMS(100);
          BEEP();
          DelayMS(100);
          BEEP();///////////////////////////////비프음 세번 울리기//////////////////////////////
          LCD_SetBrushColor(RGB_RED);
          LCD_DrawFillRect(32+x*9,21+y*9,9,9);//파란색
          i[x][y]=1;// 놓인돌 방지
          Turn=0; //다시 돌 설정
          count=2; // 파란돌을 설정
        }
      }
    }
    
    else if(ch == 0x31)
    {
      GPIOG->ODR = 0xFF00;	// LED 초기값: LED0~7 Off
      GPIOG->ODR |= 0x0001; // LED 0 ON
      LCD_SetTextColor(RGB_BLUE);// 글자색 : Red
      LCD_DrawText(2,116," "); //빨간색
      LCD_DrawText(145,116,"*"); //빨간색
      LCD_SetBrushColor(RGB_BLUE);
      Turn=1; //빨간 돌을 제어
      count=1;//빨간 돌을 제어
    }
    else if(ch == 0x32)
    {
      GPIOG->ODR = 0xFF00;	// LED 초기값: LED0~7 Off
      GPIOG->ODR |= 0x0080; // LED 0 ON
      LCD_SetTextColor(RGB_RED);// 글자색 : Red
      LCD_DrawText(2,116,"*"); //빨간색
      LCD_DrawText(145,116,""); //빨간색
      LCD_SetBrushColor(RGB_RED);
      Turn=3;// 파란돌을 제어
      count=2;// 파란돌을 제어
    }                
    
  }
  
  // DR 을 읽으면 SR.RXNE bit(flag bit)는 clear 된다 
}
void SerialPutChar(uint8_t Ch) // 1문자 보내기 함수
{
  while((UART5->SR & USART_SR_TXE) == RESET); //  USART_SR_TXE:0x0080, 송신 가능한 상태까지 대기   0 이면 끝 
  
  UART5->DR = (Ch & 0x01FF);	// 전송    0000 0001 1111 1111    0x0001   
  //         0000 0000 0000 0001    0x0031
  //           0000 0000 0000 0001
}

void Serial_PutString(char *str) // 여러문자 보내기 함수
{
  while (*str != '\0') // 종결문자가 나오기 전까지 구동, 종결문자가 나온후에도 구동시 메모리 오류 발생가능성 있음.
  {
    SerialPutChar(*str);	// 포인터가 가르키는 곳의 데이터를 송신
    str++; 			// 포인터 수치 증가
  }
}

void USART_BRR_Configuration(uint32_t USART_BaudRate) //통신속도 결정
{ 
  uint32_t tmpreg = 0x00;
  uint32_t APB2clock = 42000000;	//PCLK2_Frequency
  uint32_t integerdivider = 0x00;
  uint32_t fractionaldivider = 0x00;
  
  // Determine the integer part 
  if ((UART5->CR1 & USART_CR1_OVER8) != 0) // USART_CR1_OVER8 : 0x8000
  {                                                                  // USART1->CR1.OVER8 = 1 (8 oversampling)
    // Integer part computing in case Oversampling mode is 8 Samples 
    integerdivider = ((25 * APB2clock) / (2 * USART_BaudRate));    
  }
  else  // USART1->CR1.OVER8 = 0 (16 oversampling)
  {	// Integer part computing in case Oversampling mode is 16 Samples 
    integerdivider = ((25 * APB2clock) / (4 * USART_BaudRate));    
  }
  tmpreg = (integerdivider / 100) << 4;
  
  // Determine the fractional part 
  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));
  
  // Implement the fractional part in the register 
  if ((UART5->CR1 & USART_CR1_OVER8) != 0)
  {
    tmpreg |= (((fractionaldivider * 8) + 50) / 100) & (0x07);
  }
  else // if ((USARTx->CR1 & USART_CR1_OVER8) == 0) 
  {
    tmpreg |= (((fractionaldivider * 16) + 50) / 100) & (0x0F);
  }
  
  // Write to USART BRR register
  UART5->BRR = (uint16_t)tmpreg;
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
