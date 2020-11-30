//************************************************

// PR2. 반도체 공정 제어기

// 작성자: 2015134010 김태호

// 작성일: 2019.12.13

//***********************************************
#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"

#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define SW4_PUSH        0xEF00  //PH12
#define SW5_PUSH        0xDF00  //PH13
#define SW6_PUSH        0xBF00  //PH14
#define SW7_PUSH        0x7F00  //PH15

void _GPIO_Init(void);
void DisplayTitle(void);

void _ADC_Init(void);
uint16_t KEY_Scan(void);

void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void BEEP(void);
void TIMER2_PWM_Init(void);
void TIMER3_Init(void);
void _EXTI_Init(void);
uint16_t ADC_Value, atuo, Voltage;
uint8_t str[20];
int Count = 0, Mode = -1, flag[6], Time_Flag = 0, time[4], OnTime[3], SW_Flag[8]; // Count(실행모드,입력모드 구분), Mode(TE,PR,O2,DR값), flag값 설정, 시간설정값들SW0~7
int i, j, test = 0;; // for문 용
int stage = 0;// 각 단계 표시

void ADC_IRQHandler(void)
{
  ADC1->SR &= ~(1 << 1);		// EOC flag clear
  
  ADC_Value = ADC1->DR;		// Reading ADC result
  
  if (SW_Flag[5] == 1 && stage == 0) // 만약 stage가 0이고 스위치 5번이 눌렸다면?
  {
    LCD_SetTextColor(RGB_RED);  //글씨색 적색
    Count++; // 실행모드 =1 입력모드 =0
    
    if (Count % 2 == 1) // 실행 모드
    {
      Mode = -1; // 초기 값을 -1로 줌으로써 0부터 시작하게 만들어 준다.
      for (int p = 0; p < 4; p++) // 빈공간 4개 생성
        LCD_DisplayText(2 + p, 0, " ");
      LCD_DisplayText(1, 6, "R");//R표시
      GPIOG->ODR |= (1 << 1); // LED1 ON
      GPIOG->ODR &= ~(1 << 0);// LED0 OFF
    }
    else  // 입력모드
    {
      LCD_DisplayText(1, 6, "I"); // I표시
      GPIOG->ODR |= (1 << 0);// LED0 ON
      GPIOG->ODR &= ~(1 << 1);// LED1 OFF
    }
    SW_Flag[5] = 0; // SW 5 flag =0
  }
  
  if (SW_Flag[7] == 1 && stage == 1 && test == 1) //SW7눌러서 값 변경
  {
    Mode++; // Mode 변경
    if (Mode % 4 == 3) // 모드가 10이 DR모드면?
      j = 10; // 0~9단계
    else // 아니라면?
      j = 7; // 0~6단계
    SW_Flag[7] = 0;// SW 7 flag =0
  }
  
  if (stage == 1 && Count % 2 == 0) // stage가 1이고 입력모드라면?
  {
    for (i = 0; i < j; i++) //for문
    {
      if (ADC_Value >= 4095 * i / j && ADC_Value < 4095 * (i + 1) / j) // 각각 단계에서  시간단위를 보여준다.
      {
        time[0] = 10 + (5 * i); // TE 1.0 ~ 4.0(6단계)
        time[1] = 5 + (5 * i); //PR 0.5 ~ 3.5(6단계)
        time[2] = 0 + (5 * i);//O2 0.0 ~ 3.0 (6단계)
        time[3] = i; //DR 0~9 (9단계)
        
        if (Mode % 4 == 0) // Mode : 0 온도값 입력
        {
          LCD_SetTextColor(RGB_RED); //글씨색 : 빨간색
          LCD_DisplayChar(2, 4, time[0] / 10 + 0x30); // 바뀌는 시간 변경 1의자리
          LCD_DisplayChar(2, 6, time[0] % 10 + 0x30);// 바뀌는 시간 변경 1/10의자리
          LCD_DisplayChar(3, 4, Fram_Read(1204) / 10 + 0x30); // 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(3, 6, Fram_Read(1204) % 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(4, 4, Fram_Read(1205) / 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(4, 6, Fram_Read(1205) % 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(5, 4, Fram_Read(1206) + 0x30);// 나머지는 Fram에 저장된 값 그대로
          GPIOG->ODR &= ~(15 << 4); // LED 4~7 OFF
          GPIOG->ODR |= (1 << 4);//LED 4 ON
          LCD_DisplayText(2, 0, "*"); // * 표시                     
          LCD_SetTextColor(RGB_BLACK);  // 글씨색 : 검은색 
          LCD_DisplayText(3, 0, " "); // 빈공간
          LCD_DisplayText(4, 0, " ");// 빈공간
          LCD_DisplayText(5, 0, " ");// 빈공간
        }
        else if (Mode % 4 == 1) // Mode : 1 압력값 입력
        {
          LCD_SetTextColor(RGB_RED);
          LCD_DisplayChar(3, 4, time[1] / 10 + 0x30); // 바뀌는 시간 변경 1의자리
          LCD_DisplayChar(3, 6, time[1] % 10 + 0x30);// 바뀌는 시간 변경 1/10의자리
          LCD_DisplayChar(2, 4, Fram_Read(1203) / 10 + 0x30); // 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(2, 6, Fram_Read(1203) % 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(4, 4, Fram_Read(1205) / 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(4, 6, Fram_Read(1205) % 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(5, 4, Fram_Read(1206) + 0x30);// 나머지는 Fram에 저장된 값 그대로
          
          GPIOG->ODR &= ~(15 << 4);// LED 4~7 OFF
          GPIOG->ODR |= (1 << 5);//LED 5 ON
          LCD_DisplayText(3, 0, "*"); // * 표시                              
          LCD_SetTextColor(RGB_BLACK); // 글씨색 : 검은색 
          LCD_DisplayText(2, 0, " "); // 빈공간
          LCD_DisplayText(4, 0, " "); // 빈공간
          LCD_DisplayText(5, 0, " "); // 빈공간
        }
        else if (Mode % 4 == 2) // Mode : 2 산소값 입력
        {
          LCD_SetTextColor(RGB_RED); // 글씨색 : 빨간색
          LCD_DisplayChar(4, 4, time[2] / 10 + 0x30); // 바뀌는 시간 변경 1의자리
          LCD_DisplayChar(4, 6, time[2] % 10 + 0x30);// 바뀌는 시간 변경 1/10의자리
          LCD_DisplayChar(3, 4, Fram_Read(1204) / 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(3, 6, Fram_Read(1204) % 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(2, 4, Fram_Read(1203) / 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(2, 6, Fram_Read(1203) % 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(5, 4, Fram_Read(1206) + 0x30);// 나머지는 Fram에 저장된 값 그대로
          GPIOG->ODR &= ~(15 << 4);// LED 4~7 OFF
          GPIOG->ODR |= (1 << 6);//LED 6 ON
          LCD_DisplayText(4, 0, "*");      // * 표시                                    
          LCD_SetTextColor(RGB_BLACK);// 글씨색 : 검은색 
          LCD_DisplayText(2, 0, " ");// 빈공간
          LCD_DisplayText(3, 0, " ");// 빈공간
          LCD_DisplayText(5, 0, " ");// 빈공간
        }
        else if (Mode % 4 == 3) // Mode : 3 플라즈마값 입력
        {
          LCD_SetTextColor(RGB_RED);// 글씨색 : 빨간색
          LCD_DisplayChar(5, 4, time[3] + 0x30);// 바뀌는 DR 값 변경 1의자리
          LCD_DisplayChar(2, 4, Fram_Read(1203) / 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(2, 6, Fram_Read(1203) % 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(3, 4, Fram_Read(1204) / 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(3, 6, Fram_Read(1204) % 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(4, 4, Fram_Read(1205) / 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          LCD_DisplayChar(4, 6, Fram_Read(1205) % 10 + 0x30);// 나머지는 Fram에 저장된 값 그대로
          
          GPIOG->ODR &= ~(15 << 4);// LED 4~7 OFF
          GPIOG->ODR |= (1 << 7);//LED 7 ON
          LCD_DisplayText(5, 0, "*");// * 표시                           
          LCD_SetTextColor(RGB_BLACK);// 글씨색 : 검은색 
          LCD_DisplayText(2, 0, " ");// 빈공간
          LCD_DisplayText(3, 0, " ");// 빈공간
          LCD_DisplayText(4, 0, " ");// 빈공간
        }
      }
    }
  }
}

int main(void)
{
  LCD_Init();	// LCD 구동 함수
  DelayMS(10);	// LCD구동 딜레이
  DisplayTitle();	//LCD 초기화면구동 함수
  _EXTI_Init(); //EXTI 설정
  
  _ADC_Init(); // ADC 설정
  _GPIO_Init();// GPIO 설정
  
  TIMER2_PWM_Init();// 타이머 2번(PWM모드) 설정
  TIMER3_Init(); // 타이머 3번(CC모드) 설정
  Fram_Init();        // FRAM 초기화 
  Fram_Status_Config();  //Fram 설정
  GPIOG->ODR |= (1 << 0); // LED 0 ON
  GPIOG->ODR &= ~(1 << 1);// LED 1 OFF
  LCD_SetTextColor(RGB_RED); // 글씨색 : 빨간색
  LCD_DisplayChar(2, 4, Fram_Read(1203) / 10 + 0x30); //Fram 1203번지 1의 자리
  LCD_DisplayChar(2, 6, Fram_Read(1203) % 10 + 0x30);//Fram 1203번지 1/10의 자리
  
  LCD_DisplayChar(3, 4, Fram_Read(1204) / 10 + 0x30);//Fram 1204번지 1의 자리
  LCD_DisplayChar(3, 6, Fram_Read(1204) % 10 + 0x30);//Fram 1204번지 1/10의 자리
  
  LCD_DisplayChar(4, 4, Fram_Read(1205) / 10 + 0x30);//Fram 1205번지 1의 자리
  LCD_DisplayChar(4, 6, Fram_Read(1205) % 10 + 0x30);//Fram 1205번지 1/10의 자리
  
  LCD_DisplayChar(5, 4, Fram_Read(1206) + 0x30);//Fram 1206번지 값
  
  while (1)
  {
    ADC1->CR2 |= ADC_CR2_SWSTART; // 0x40000000 (1<<30) 
    
    if (SW_Flag[6] == 1 && Count % 2 == 1 && stage == 0) // 6번이 눌렸을 때 실행 모드
    {
      LCD_SetTextColor(RGB_RED); //글씨색 : 빨간색
      GPIOG->ODR |= (1 << 2); // LED 2 ON
      LCD_DisplayChar(1, 15, 'W'); // W 표시
      GPIOG->ODR |= (1 << 3); // LED 3 ON
      DelayMS(500); // 딜레이 0.5s
      
      while (OnTime[0] != Fram_Read(1203)) // 만약 지정 시간이 1203번지와 같지 않다면? ( 계속 반복하라)
      {
        Time_Flag = 1; // 시간 플래그 1
        LCD_DisplayText(2, 0, "*"); // * 표시
        GPIOG->ODR &= ~(15 << 4); //LED 4~7 OFF
        GPIOG->ODR |= (1 << 4); // LED 4 ON
        LCD_DisplayText(3, 0, " ");// 빈공간
        LCD_DisplayText(4, 0, " "); // 빈공간
        LCD_DisplayText(5, 0, " "); // 빈공간
      }
      while (OnTime[1] != Fram_Read(1204)) // 만약 지정 시간이 1204번지와 같지 않다면 ( 계속 반복하라 )
      {
        Time_Flag = 2;// 시간 플래그 2
        GPIOG->ODR &= ~(15 << 4); // LED 4~7 OFF
        GPIOG->ODR |= (1 << 5); // LED 5 ON
        LCD_DisplayText(2, 0, " ");// 빈공간
        LCD_DisplayText(3, 0, "*");// * 표시
        LCD_DisplayText(4, 0, " "); // 빈공간
        LCD_DisplayText(5, 0, " "); // 빈공간
      }
      while (OnTime[2] != Fram_Read(1205)) // 만약 지정 시간이 1205번지와 같지 않다면?(계속 반복하라)
      {
        Time_Flag = 3;  //시간 플래그 3
        
        TIM2->CCR3 = 2000 * (Fram_Read(1206) + 1);		// CCR3 value
        
        GPIOG->ODR &= ~(15 << 4); //  LED 4 ~ 7 OFF
        GPIOG->ODR |= (3 << 6); // LED 6,7 ON
        LCD_DisplayText(2, 0, " "); // 빈공간
        LCD_DisplayText(3, 0, " ");// 빈공간
        LCD_DisplayText(4, 0, "*"); // *표시
        LCD_DisplayText(5, 0, "*"); // *표시
      }
      
      GPIOG->ODR &= ~(15 << 4); // LED 4 ~ 7 OFF
      LCD_DisplayText(2, 0, " ");// 빈공간
      LCD_DisplayText(3, 0, " ");// 빈공간
      LCD_DisplayText(4, 0, " ");// 빈공간
      LCD_DisplayText(5, 0, " ");// 빈공간
      TIM2->CCR3 = 0;	// CCR3 value
      for (int k = 0; k < 3; k++) // *OnTime 값들 모두 0으로 초기화
      {
        OnTime[k] = 0;
      }
      Time_Flag = 0;// 타임 플래그 0
      
      DelayMS(500); // 강제 Delay
      
      GPIOG->ODR &= ~(1 << 3); // LED 3 OFF
      LCD_DisplayChar(1, 15, 'E'); // E 표시
      stage = 1; // stage는 1단계로 넘어간다.
      SW_Flag[5] = 0; // SW 5 flag =0
      SW_Flag[6] = 0; // SW 6 flag =0
    }
    
    if (SW_Flag[6] == 1 && Count % 2 == 1 && stage == 1) // 6번이 눌렸을 때 실행 모드 탈출 확인
    {
      GPIOG->ODR &= ~(1 << 2); //LED 2 OFF
      stage = 0; // stage는 0번으로 돌아감
      SW_Flag[5] = 0; // SW 5 flag =0
      SW_Flag[6] = 0; // SW 6 flag =0
    }
    
    else if (SW_Flag[6] == 1 && Count % 2 == 0 && stage == 0) // 6번이 눌렸을 때 입력 모드(세부 입력 모드)
    {
      Mode = 0; // Mode의 초기값은 바로 ++이 되므로 0-1로 설정 하였다
      j = 7;
      stage = 1; // 스테이지 1
      LCD_DisplayText(2, 0, "*");// 빈공간
      LCD_DisplayText(3, 0, " ");// 빈공간
      LCD_DisplayText(4, 0, " ");// 빈공간
      LCD_DisplayText(5, 0, " ");// 빈공간
      test = 1;
      SW_Flag[5] = 0; // SW 5 flag =0
      SW_Flag[6] = 0; // SW 6 flag =0
      SW_Flag[7] = 0; // SW 7 flag =0
    }
    
    else if (SW_Flag[6] == 1 && Count % 2 == 0 && stage == 1) //세부 입력 모드 탈출
    {
      LCD_SetTextColor(RGB_RED); // 글씨 색 : 빨간색
      stage = 0; // 스테이지 0
      for (int p = 0; p < 4; p++) //반복하라
        LCD_DisplayText(2 + p, 0, " "); // 빈공간 생성
      
      LCD_DisplayChar(2, 4, Fram_Read(1203) / 10 + 0x30); // 1203번지 1의자리
      LCD_DisplayChar(2, 6, Fram_Read(1203) % 10 + 0x30); // 1203번지 1/10의 자리
      
      LCD_DisplayChar(3, 4, Fram_Read(1204) / 10 + 0x30); // 1204번지 1의자리
      LCD_DisplayChar(3, 6, Fram_Read(1204) % 10 + 0x30);// 1204번지 1/10의 자리
      
      LCD_DisplayChar(4, 4, Fram_Read(1205) / 10 + 0x30);// 1205번지 1의 자리
      LCD_DisplayChar(4, 6, Fram_Read(1205) % 10 + 0x30); // 1205번지 1/10의 자리
      
      LCD_DisplayChar(5, 4, Fram_Read(1206) + 0x30); // 1206번지 값
      test = 0;
      SW_Flag[5] = 0; // SW 5 flag =0
      SW_Flag[6] = 0; // SW 6 flag =0
    }
    
    if (SW_Flag[4] && stage == 1) // Fram값 입력할 것
    {
      if (Mode % 4 == 0) // 온도값 입력
        Fram_Write(1203, time[0]); // 1203번지 입력
      else if (Mode % 4 == 1) // 압력값 입력
        Fram_Write(1204, time[1]); // 1204번지 입력
      else if (Mode % 4 == 2) // 산소값 입력
        Fram_Write(1205, time[2]); // 1205번지 입력
      else if (Mode % 4 == 3) // 플라즈마값 입력
        Fram_Write(1206, time[3]); // 1206번지 입력
      SW_Flag[4] = 0;// SW 4 flag =0
    }
  }
}
void _EXTI_Init(void) //인터럽트 설정
{
  RCC->AHB1ENR |= 0x0080;	// RCC_AHB1ENR GPIOH Enable
  RCC->APB2ENR |= 0x4000;	// Enable System Configuration Controller Clock
  
  GPIOH->MODER &= 0x00FF;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
  
  SYSCFG->EXTICR[3] |= 0x7777; 	// EXTI15~12에 대한 소스 입력은 GPIOH로 설정 (EXTICR3) (reset value: 0x0000)	
  
  EXTI->FTSR |= 0xF000;		// Falling Trigger Enable (EXTI15~12:PH15~12)
  EXTI->IMR |= 0xF000;  	// EXTI15~12 인터럽트 mask (Interrupt Enable)
  NVIC->ISER[1] |= (1 << 8);   // Enable Interrupt EXTI15 Vector table Position 참조
}

void EXTI15_10_IRQHandler(void)		// EXTI 15~10 인터럽트 핸들러
{
  if (EXTI->PR & 0x1000) 	// EXTI12 Interrupt Pending?
  {
    EXTI->PR |= 0x1000; 	// Pending bit Clear
    SW_Flag[4] = 1; // SW4이 눌린 상황이면 1로 저장
  }
  if (EXTI->PR & 0x2000) 	// EXTI13 Interrupt Pending?
  {
    EXTI->PR |= 0x2000; 	// Pending bit Clear
    SW_Flag[5] = 1; // SW5이 눌린 상황이면 1로 저장
  }
  if (EXTI->PR & 0x4000) 	// EXTI14 Interrupt Pending?
  {
    EXTI->PR |= 0x4000; 	// Pending bit Clear
    SW_Flag[6] = 1; // SW6이 눌린 상황이면 1로 저장
  }
  if (EXTI->PR & 0x8000) 	// EXTI15 Interrupt Pending?
  {
    EXTI->PR |= 0x8000; 	// Pending bit Clear
    SW_Flag[7] = 1; // SW7이 눌린 상황이면 1로 저장
  }
}
void TIMER2_PWM_Init(void)
{
  // TIM2 CH3 : PB10 
  // Clock Enable : GPIOB & TIMER2
  RCC->AHB1ENR |= (1 << 1);	// GPIOB CLOCK Enable
  RCC->APB1ENR |= (1 << 0);	// TIMER2 CLOCK Enable 
  
  // PB10을 출력설정하고 Alternate function(TIM2_CH3)으로 사용 선언 : PWM 출력
  GPIOB->MODER |= (2 << 2 * 10);	//  PB10 Output Alternate function mode					
  GPIOB->OSPEEDR |= (3 << 2 * 10);	// PB10 Output speed (100MHz High speed)
  GPIOB->OTYPER &= ~(1 << 10);	// PB10 Output type push-pull (reset state)
  GPIOB->PUPDR |= (1 << 2 * 11);	// PB10 Pull-up
  GPIOB->AFR[1] |= (1 << 4 * 2);	// (AFR[1].(11~8)=0b0001): Connect TIM2 pins(PA3) to AF1(TIM1..2)
  // TIM2 Channel 3 : PWM 1 mode
  // Assign 'PWM Pulse Period'
  TIM2->PSC = 8400 - 1;	// Prescaler 84,000,000Hz/8400 = 10,000 Hz(0.1ms)  (1~65536)
  TIM2->ARR = 20000 - 1;	// Auto reload  (0.1ms * 20000= 2s : PWM Period)
  
  // Setting CR1 : 0x0000 (Up counting)
  TIM2->CR1 &= ~(1 << 4);	// DIR=0(Up counter)(reset state5)
  TIM2->CR1 &= ~(1 << 1);	// UDIS=0(Update event Enabled): By one of following events
  //	- Counter Overflow/Underflow, 
  // 	- Setting the UG bit Set,
  //	- Update Generation through the slave mode controller 
  TIM2->CR1 &= ~(1 << 2);	// URS=0(Update event source Selection): one of following events
  //	- Counter Overflow/Underflow, 
  // 	- Setting the UG bit Set,
  //	- Update Generation through the slave mode controller 
  TIM2->CR1 &= ~(1 << 3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
  TIM2->CR1 &= ~(1 << 7);	// ARPE=0(ARR is NOT buffered) (reset state)
  TIM2->CR1 &= ~(3 << 8); 	// CKD(Clock division)=00(reset state)
  TIM2->CR1 &= ~(3 << 5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
  // Center-aligned mode: The counter counts Up and DOWN alternatively
  
  // Define the corresponding pin by 'Output'  
  // CCER(Capture/Compare Enable Register) : Enable "Channel 3" 
  TIM2->CCER |= (1 << 8);	// CC3E=1: OC3(TIM2_CH3) Active(Capture/Compare 3 output enable
  TIM2->CCER &= ~(1 << 9);	// CC3P=0: CC3 Output Polarity (OCPolarity_High : OC3으로 반전없이 출력)
  
  // Duty Ratio 
  TIM2->CCR3 = 2000;		// CCR3 value
  
  // 'Mode' Selection : Output mode, PWM 1
  // CCMR2(Capture/Compare Mode Register 2) : Setting the MODE of Ch3 or Ch4
  TIM2->CCMR2 &= ~(3 << 0); 	// CC3S(CC3 channel)= '0b00' : Output 
  TIM2->CCMR2 |= (1 << 3); 	// OC3PE=1: Output Compare 3 preload Enable
  TIM2->CCMR2 |= (6 << 4);	// OC3M=0b110: Output compare 3 mode: PWM 1 mode
  TIM2->CCMR2 |= (1 << 7);	// OC#CE=1: Output compare 3 Clear enable
  
  //Counter TIM2 enable
  TIM2->CR1 |= (1 << 7);	// ARPE: Auto-reload preload enable
  TIM2->CR1 |= (1 << 0);	// CEN: Counter TIM2 enable
}

void TIMER3_Init(void)
{
  // Clock Enable : TIMER3
  RCC->APB1ENR |= (1 << 1);	// TIMER3 Enable 
  
  NVIC->ISER[0] |= (1 << 29); // Enable Timer3 global Interrupt
  
  TIM3->PSC = 8400 - 1;	// Prescaler 84MHz/8400 = 10,000Hz (10us)
  TIM3->ARR = 1000;	// Auto reload  : 10us * 10K = 100ms(period)
  TIM3->CR1 |= (1 << 4);	// Downcounter
  TIM3->CR1 &= ~(3 << 8); 	// CKD(Clock division)=1(reset state)
  TIM3->CR1 &= ~(3 << 5); 	// CMS(Center-aligned mode Sel): No(reset state)
  
  TIM3->EGR |= (1 << 0);	// Update generation     0번비트를 1로 만들었다.
  
  TIM3->DIER |= (1 << 0);	// Enable the Tim3 Update interrupt // 인터럽트를 사용하겠다.
  TIM3->CR1 |= (1 << 0);	// Enable the Tim3 Counter      // TIM3 카운터를 사용하겠다. 	
}

void TIM3_IRQHandler() // 핸들러 설정
{
  TIM3->SR &= ~(1 << 0);	// Interrupt flag Clear
  LCD_SetTextColor(RGB_RED); // 글씨색 : RED
  
  if (Time_Flag == 1) // Time 플래그 1
  {
    OnTime[0]++;
    LCD_DisplayChar(2, 9, 0x30 + OnTime[0] / 10); // OnTime[0]값 1의 자리
    LCD_DisplayChar(2, 11, 0x30 + OnTime[0] % 10);// OnTime[0]값 1/10의 자리
  }
  if (Time_Flag == 2) // Time 플래그 2
  {
    OnTime[1]++;
    LCD_DisplayChar(3, 9, 0x30 + OnTime[1] / 10);// OnTime[1]값 1의 자리
    LCD_DisplayChar(3, 11, 0x30 + OnTime[1] % 10);// OnTime[1]값 1/10의 자리
  }
  if (Time_Flag == 3) // Time 플래그 3
  {
    OnTime[2]++;
    LCD_DisplayChar(4, 9, 0x30 + OnTime[2] / 10);// OnTime[2]값 1의 자리
    LCD_DisplayChar(4, 11, 0x30 + OnTime[2] % 10);//// OnTime[2]값 1/10의 자리
  }
}
void _ADC_Init(void)
{	// ADC1 PA1(pin 41)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	// (1<<0) ENABLE GPIOA CLK (stm32f4xx.h 참조)
  GPIOA->MODER |= (3 << 2 * 1);		// CONFIG GPIOA PIN1(PA1) TO ANALOG IN MODE
  
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;	//  ENABLE ADC1 CLK (stm32f4xx.h 참조)
  
  ADC->CCR &= ~(0X1F << 0);		// MULTI[4:0]: ADC_Mode_Independent
  ADC->CCR |= (1 << 16); 		// 0x00010000 ADCPRE:ADC_Prescaler_Div4 (ADC MAX Clock 36MHz, 84Mhz(APB2)/4 = 21MHz)
  
  ADC1->CR1 &= ~(3 << 24);		// RES[1:0]= :12bit Resolution
  ADC1->CR1 &= ~(1 << 8);		// SCAN=0 : ADC_ScanCovMode Disable
  ADC1->CR1 |= (1 << 5);		// EOCIE=1: Interrupt enable for EOC   ///
  
  ADC1->CR2 &= ~(1 << 1);		// CONT=0: ADC_Continuous ConvMode Disable
  ADC1->CR2 &= ~(3 << 28);		// EXTEN[1:0]=0b00: ADC_ExternalTrigConvEdge_None
  ADC1->CR2 &= ~(1 << 11);		// ALIGN=0: ADC_DataAlign_Right
  ADC1->CR2 &= ~(1 << 10);		// EOCS=0: The EOC bit is set at the end of each sequence of regular conversions
  
  ADC1->SQR1 &= ~(0xF << 20);	// L[3:0]=0b0000: ADC Regular channel sequece length 
  // 0b0000:1 conversion)
  //Channel selection, The Conversion Sequence of PIN1(ADC1_CH1) is first, Config sequence Range is possible from 0 to 17
  ADC1->SMPR2 |= (0x7 << (3 * 1));	// ADC1_CH1 Sample TIme_480Cycles (3*Channel_1)
  //Channel selection, The Conversion Sequence of PIN1(ADC1_CH1) is first, Config sequence Range is possible from 0 to 17
  ADC1->SQR3 |= (1 << 0);		// SQ1[4:0]=0b0001 : CH1
  NVIC->ISER[0] |= (1 << 18);	// Enable ADC global Interrupt
  
  ADC1->CR2 |= (1 << 0);		// ADON: ADC ON
}

void _GPIO_Init(void)
{
  // LED (GPIO G) 설정
  RCC->AHB1ENR |= 0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
  GPIOG->MODER |= 0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
  GPIOG->OTYPER &= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
  GPIOG->OSPEEDR |= 0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
  
  // SW (GPIO H) 설정 
  RCC->AHB1ENR |= 0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
  GPIOH->MODER &= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
  GPIOH->PUPDR &= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state
}

void BEEP(void)			// Beep for 20 ms 
{
  GPIOF->ODR |= (1 << 9);	// PF9 'H' Buzzer on
  DelayMS(20);		// Delay 20 ms
  GPIOF->ODR &= ~(1 << 9);	// PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
  register unsigned short i;
  for (i = 0; i < wMS; i++)
    DelayUS(1000);   // 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
  volatile int Dly = (int)wUS * 17;
  for (; Dly; Dly--);
}

void DisplayTitle(void) // 초기 화면
{
  LCD_Clear(RGB_YELLOW); // 바탕색 : 노란색
  LCD_SetFont(&Gulim10); // 굴림 : 10
  LCD_SetBackColor(RGB_YELLOW); // 배경 : 노란색
  
  LCD_SetTextColor(RGB_BLUE);	 // 글씨색 : 파란색
  LCD_DisplayText(0, 0, "SPC monitor");  // 글씨
  
  LCD_SetTextColor(RGB_BLACK);// 글씨색 : 검은색
  LCD_DisplayText(1, 1, "Mode:   Wafer: ");  // 글씨
  LCD_DisplayText(2, 1, "TE:1.0S 0.0S"); // 글씨
  LCD_DisplayText(3, 1, "PR:1.0S 0.0S");// 글씨
  LCD_DisplayText(4, 1, "O2:1.0S 0.0S");// 글씨
  LCD_DisplayText(5, 1, "DR:");// 글씨
  
  //////////////////////////////////////////////
  LCD_SetTextColor(GET_RGB(255, 0, 0)); // 글씨색 : 빨간색
  LCD_DisplayChar(1, 6, 'I');// 글씨
  LCD_DisplayChar(1, 15, 'E');// 글씨
  
  LCD_DisplayChar(2, 4, 0x31);// 글씨
  LCD_DisplayChar(3, 4, 0x30);// 글씨
  LCD_DisplayChar(4, 4, 0x30);// 글씨
  
  LCD_DisplayChar(2, 6, 0x30);// 글씨
  LCD_DisplayChar(3, 6, 0x35);// 글씨
  LCD_DisplayChar(4, 6, 0x30);// 글씨
  
  LCD_DisplayChar(2, 9, 0x30);// 글씨
  LCD_DisplayChar(3, 9, 0x30);// 글씨
  LCD_DisplayChar(4, 9, 0x30);// 글씨
  
  LCD_DisplayChar(2, 11, 0x30);// 글씨
  LCD_DisplayChar(3, 11, 0x30);// 글씨
  LCD_DisplayChar(4, 11, 0x30);// 글씨
  
  LCD_DisplayChar(5, 4, 0x31);// 글씨
}

uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{
  uint16_t key;
  key = GPIOH->IDR & 0xFF00;	// any key pressed ?
  if (key == 0xFF00)		// if no key, check key off
  {
    if (key_flag == 0)
      return key;
    else
    {
      DelayMS(10);
      key_flag = 0;
      return key;
    }
  }
  else				// if key input, check continuous key
  {
    if (key_flag != 0)	// if continuous key, treat as no key input
      return 0xFF00;
    else			// if new key,delay for debounce
    {
      key_flag = 1;
      DelayMS(10);
      return key;
    }
  }
}
