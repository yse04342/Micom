//************************************************

// PR2. �ݵ�ü ���� �����

// �ۼ���: 2015134010 ����ȣ

// �ۼ���: 2019.12.13

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
int Count = 0, Mode = -1, flag[6], Time_Flag = 0, time[4], OnTime[3], SW_Flag[8]; // Count(������,�Է¸�� ����), Mode(TE,PR,O2,DR��), flag�� ����, �ð���������SW0~7
int i, j, test = 0;; // for�� ��
int stage = 0;// �� �ܰ� ǥ��

void ADC_IRQHandler(void)
{
  ADC1->SR &= ~(1 << 1);		// EOC flag clear
  
  ADC_Value = ADC1->DR;		// Reading ADC result
  
  if (SW_Flag[5] == 1 && stage == 0) // ���� stage�� 0�̰� ����ġ 5���� ���ȴٸ�?
  {
    LCD_SetTextColor(RGB_RED);  //�۾��� ����
    Count++; // ������ =1 �Է¸�� =0
    
    if (Count % 2 == 1) // ���� ���
    {
      Mode = -1; // �ʱ� ���� -1�� �����ν� 0���� �����ϰ� ����� �ش�.
      for (int p = 0; p < 4; p++) // ����� 4�� ����
        LCD_DisplayText(2 + p, 0, " ");
      LCD_DisplayText(1, 6, "R");//Rǥ��
      GPIOG->ODR |= (1 << 1); // LED1 ON
      GPIOG->ODR &= ~(1 << 0);// LED0 OFF
    }
    else  // �Է¸��
    {
      LCD_DisplayText(1, 6, "I"); // Iǥ��
      GPIOG->ODR |= (1 << 0);// LED0 ON
      GPIOG->ODR &= ~(1 << 1);// LED1 OFF
    }
    SW_Flag[5] = 0; // SW 5 flag =0
  }
  
  if (SW_Flag[7] == 1 && stage == 1 && test == 1) //SW7������ �� ����
  {
    Mode++; // Mode ����
    if (Mode % 4 == 3) // ��尡 10�� DR����?
      j = 10; // 0~9�ܰ�
    else // �ƴ϶��?
      j = 7; // 0~6�ܰ�
    SW_Flag[7] = 0;// SW 7 flag =0
  }
  
  if (stage == 1 && Count % 2 == 0) // stage�� 1�̰� �Է¸����?
  {
    for (i = 0; i < j; i++) //for��
    {
      if (ADC_Value >= 4095 * i / j && ADC_Value < 4095 * (i + 1) / j) // ���� �ܰ迡��  �ð������� �����ش�.
      {
        time[0] = 10 + (5 * i); // TE 1.0 ~ 4.0(6�ܰ�)
        time[1] = 5 + (5 * i); //PR 0.5 ~ 3.5(6�ܰ�)
        time[2] = 0 + (5 * i);//O2 0.0 ~ 3.0 (6�ܰ�)
        time[3] = i; //DR 0~9 (9�ܰ�)
        
        if (Mode % 4 == 0) // Mode : 0 �µ��� �Է�
        {
          LCD_SetTextColor(RGB_RED); //�۾��� : ������
          LCD_DisplayChar(2, 4, time[0] / 10 + 0x30); // �ٲ�� �ð� ���� 1���ڸ�
          LCD_DisplayChar(2, 6, time[0] % 10 + 0x30);// �ٲ�� �ð� ���� 1/10���ڸ�
          LCD_DisplayChar(3, 4, Fram_Read(1204) / 10 + 0x30); // �������� Fram�� ����� �� �״��
          LCD_DisplayChar(3, 6, Fram_Read(1204) % 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(4, 4, Fram_Read(1205) / 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(4, 6, Fram_Read(1205) % 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(5, 4, Fram_Read(1206) + 0x30);// �������� Fram�� ����� �� �״��
          GPIOG->ODR &= ~(15 << 4); // LED 4~7 OFF
          GPIOG->ODR |= (1 << 4);//LED 4 ON
          LCD_DisplayText(2, 0, "*"); // * ǥ��                     
          LCD_SetTextColor(RGB_BLACK);  // �۾��� : ������ 
          LCD_DisplayText(3, 0, " "); // �����
          LCD_DisplayText(4, 0, " ");// �����
          LCD_DisplayText(5, 0, " ");// �����
        }
        else if (Mode % 4 == 1) // Mode : 1 �з°� �Է�
        {
          LCD_SetTextColor(RGB_RED);
          LCD_DisplayChar(3, 4, time[1] / 10 + 0x30); // �ٲ�� �ð� ���� 1���ڸ�
          LCD_DisplayChar(3, 6, time[1] % 10 + 0x30);// �ٲ�� �ð� ���� 1/10���ڸ�
          LCD_DisplayChar(2, 4, Fram_Read(1203) / 10 + 0x30); // �������� Fram�� ����� �� �״��
          LCD_DisplayChar(2, 6, Fram_Read(1203) % 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(4, 4, Fram_Read(1205) / 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(4, 6, Fram_Read(1205) % 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(5, 4, Fram_Read(1206) + 0x30);// �������� Fram�� ����� �� �״��
          
          GPIOG->ODR &= ~(15 << 4);// LED 4~7 OFF
          GPIOG->ODR |= (1 << 5);//LED 5 ON
          LCD_DisplayText(3, 0, "*"); // * ǥ��                              
          LCD_SetTextColor(RGB_BLACK); // �۾��� : ������ 
          LCD_DisplayText(2, 0, " "); // �����
          LCD_DisplayText(4, 0, " "); // �����
          LCD_DisplayText(5, 0, " "); // �����
        }
        else if (Mode % 4 == 2) // Mode : 2 ��Ұ� �Է�
        {
          LCD_SetTextColor(RGB_RED); // �۾��� : ������
          LCD_DisplayChar(4, 4, time[2] / 10 + 0x30); // �ٲ�� �ð� ���� 1���ڸ�
          LCD_DisplayChar(4, 6, time[2] % 10 + 0x30);// �ٲ�� �ð� ���� 1/10���ڸ�
          LCD_DisplayChar(3, 4, Fram_Read(1204) / 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(3, 6, Fram_Read(1204) % 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(2, 4, Fram_Read(1203) / 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(2, 6, Fram_Read(1203) % 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(5, 4, Fram_Read(1206) + 0x30);// �������� Fram�� ����� �� �״��
          GPIOG->ODR &= ~(15 << 4);// LED 4~7 OFF
          GPIOG->ODR |= (1 << 6);//LED 6 ON
          LCD_DisplayText(4, 0, "*");      // * ǥ��                                    
          LCD_SetTextColor(RGB_BLACK);// �۾��� : ������ 
          LCD_DisplayText(2, 0, " ");// �����
          LCD_DisplayText(3, 0, " ");// �����
          LCD_DisplayText(5, 0, " ");// �����
        }
        else if (Mode % 4 == 3) // Mode : 3 �ö���� �Է�
        {
          LCD_SetTextColor(RGB_RED);// �۾��� : ������
          LCD_DisplayChar(5, 4, time[3] + 0x30);// �ٲ�� DR �� ���� 1���ڸ�
          LCD_DisplayChar(2, 4, Fram_Read(1203) / 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(2, 6, Fram_Read(1203) % 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(3, 4, Fram_Read(1204) / 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(3, 6, Fram_Read(1204) % 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(4, 4, Fram_Read(1205) / 10 + 0x30);// �������� Fram�� ����� �� �״��
          LCD_DisplayChar(4, 6, Fram_Read(1205) % 10 + 0x30);// �������� Fram�� ����� �� �״��
          
          GPIOG->ODR &= ~(15 << 4);// LED 4~7 OFF
          GPIOG->ODR |= (1 << 7);//LED 7 ON
          LCD_DisplayText(5, 0, "*");// * ǥ��                           
          LCD_SetTextColor(RGB_BLACK);// �۾��� : ������ 
          LCD_DisplayText(2, 0, " ");// �����
          LCD_DisplayText(3, 0, " ");// �����
          LCD_DisplayText(4, 0, " ");// �����
        }
      }
    }
  }
}

int main(void)
{
  LCD_Init();	// LCD ���� �Լ�
  DelayMS(10);	// LCD���� ������
  DisplayTitle();	//LCD �ʱ�ȭ�鱸�� �Լ�
  _EXTI_Init(); //EXTI ����
  
  _ADC_Init(); // ADC ����
  _GPIO_Init();// GPIO ����
  
  TIMER2_PWM_Init();// Ÿ�̸� 2��(PWM���) ����
  TIMER3_Init(); // Ÿ�̸� 3��(CC���) ����
  Fram_Init();        // FRAM �ʱ�ȭ 
  Fram_Status_Config();  //Fram ����
  GPIOG->ODR |= (1 << 0); // LED 0 ON
  GPIOG->ODR &= ~(1 << 1);// LED 1 OFF
  LCD_SetTextColor(RGB_RED); // �۾��� : ������
  LCD_DisplayChar(2, 4, Fram_Read(1203) / 10 + 0x30); //Fram 1203���� 1�� �ڸ�
  LCD_DisplayChar(2, 6, Fram_Read(1203) % 10 + 0x30);//Fram 1203���� 1/10�� �ڸ�
  
  LCD_DisplayChar(3, 4, Fram_Read(1204) / 10 + 0x30);//Fram 1204���� 1�� �ڸ�
  LCD_DisplayChar(3, 6, Fram_Read(1204) % 10 + 0x30);//Fram 1204���� 1/10�� �ڸ�
  
  LCD_DisplayChar(4, 4, Fram_Read(1205) / 10 + 0x30);//Fram 1205���� 1�� �ڸ�
  LCD_DisplayChar(4, 6, Fram_Read(1205) % 10 + 0x30);//Fram 1205���� 1/10�� �ڸ�
  
  LCD_DisplayChar(5, 4, Fram_Read(1206) + 0x30);//Fram 1206���� ��
  
  while (1)
  {
    ADC1->CR2 |= ADC_CR2_SWSTART; // 0x40000000 (1<<30) 
    
    if (SW_Flag[6] == 1 && Count % 2 == 1 && stage == 0) // 6���� ������ �� ���� ���
    {
      LCD_SetTextColor(RGB_RED); //�۾��� : ������
      GPIOG->ODR |= (1 << 2); // LED 2 ON
      LCD_DisplayChar(1, 15, 'W'); // W ǥ��
      GPIOG->ODR |= (1 << 3); // LED 3 ON
      DelayMS(500); // ������ 0.5s
      
      while (OnTime[0] != Fram_Read(1203)) // ���� ���� �ð��� 1203������ ���� �ʴٸ�? ( ��� �ݺ��϶�)
      {
        Time_Flag = 1; // �ð� �÷��� 1
        LCD_DisplayText(2, 0, "*"); // * ǥ��
        GPIOG->ODR &= ~(15 << 4); //LED 4~7 OFF
        GPIOG->ODR |= (1 << 4); // LED 4 ON
        LCD_DisplayText(3, 0, " ");// �����
        LCD_DisplayText(4, 0, " "); // �����
        LCD_DisplayText(5, 0, " "); // �����
      }
      while (OnTime[1] != Fram_Read(1204)) // ���� ���� �ð��� 1204������ ���� �ʴٸ� ( ��� �ݺ��϶� )
      {
        Time_Flag = 2;// �ð� �÷��� 2
        GPIOG->ODR &= ~(15 << 4); // LED 4~7 OFF
        GPIOG->ODR |= (1 << 5); // LED 5 ON
        LCD_DisplayText(2, 0, " ");// �����
        LCD_DisplayText(3, 0, "*");// * ǥ��
        LCD_DisplayText(4, 0, " "); // �����
        LCD_DisplayText(5, 0, " "); // �����
      }
      while (OnTime[2] != Fram_Read(1205)) // ���� ���� �ð��� 1205������ ���� �ʴٸ�?(��� �ݺ��϶�)
      {
        Time_Flag = 3;  //�ð� �÷��� 3
        
        TIM2->CCR3 = 2000 * (Fram_Read(1206) + 1);		// CCR3 value
        
        GPIOG->ODR &= ~(15 << 4); //  LED 4 ~ 7 OFF
        GPIOG->ODR |= (3 << 6); // LED 6,7 ON
        LCD_DisplayText(2, 0, " "); // �����
        LCD_DisplayText(3, 0, " ");// �����
        LCD_DisplayText(4, 0, "*"); // *ǥ��
        LCD_DisplayText(5, 0, "*"); // *ǥ��
      }
      
      GPIOG->ODR &= ~(15 << 4); // LED 4 ~ 7 OFF
      LCD_DisplayText(2, 0, " ");// �����
      LCD_DisplayText(3, 0, " ");// �����
      LCD_DisplayText(4, 0, " ");// �����
      LCD_DisplayText(5, 0, " ");// �����
      TIM2->CCR3 = 0;	// CCR3 value
      for (int k = 0; k < 3; k++) // *OnTime ���� ��� 0���� �ʱ�ȭ
      {
        OnTime[k] = 0;
      }
      Time_Flag = 0;// Ÿ�� �÷��� 0
      
      DelayMS(500); // ���� Delay
      
      GPIOG->ODR &= ~(1 << 3); // LED 3 OFF
      LCD_DisplayChar(1, 15, 'E'); // E ǥ��
      stage = 1; // stage�� 1�ܰ�� �Ѿ��.
      SW_Flag[5] = 0; // SW 5 flag =0
      SW_Flag[6] = 0; // SW 6 flag =0
    }
    
    if (SW_Flag[6] == 1 && Count % 2 == 1 && stage == 1) // 6���� ������ �� ���� ��� Ż�� Ȯ��
    {
      GPIOG->ODR &= ~(1 << 2); //LED 2 OFF
      stage = 0; // stage�� 0������ ���ư�
      SW_Flag[5] = 0; // SW 5 flag =0
      SW_Flag[6] = 0; // SW 6 flag =0
    }
    
    else if (SW_Flag[6] == 1 && Count % 2 == 0 && stage == 0) // 6���� ������ �� �Է� ���(���� �Է� ���)
    {
      Mode = 0; // Mode�� �ʱⰪ�� �ٷ� ++�� �ǹǷ� 0-1�� ���� �Ͽ���
      j = 7;
      stage = 1; // �������� 1
      LCD_DisplayText(2, 0, "*");// �����
      LCD_DisplayText(3, 0, " ");// �����
      LCD_DisplayText(4, 0, " ");// �����
      LCD_DisplayText(5, 0, " ");// �����
      test = 1;
      SW_Flag[5] = 0; // SW 5 flag =0
      SW_Flag[6] = 0; // SW 6 flag =0
      SW_Flag[7] = 0; // SW 7 flag =0
    }
    
    else if (SW_Flag[6] == 1 && Count % 2 == 0 && stage == 1) //���� �Է� ��� Ż��
    {
      LCD_SetTextColor(RGB_RED); // �۾� �� : ������
      stage = 0; // �������� 0
      for (int p = 0; p < 4; p++) //�ݺ��϶�
        LCD_DisplayText(2 + p, 0, " "); // ����� ����
      
      LCD_DisplayChar(2, 4, Fram_Read(1203) / 10 + 0x30); // 1203���� 1���ڸ�
      LCD_DisplayChar(2, 6, Fram_Read(1203) % 10 + 0x30); // 1203���� 1/10�� �ڸ�
      
      LCD_DisplayChar(3, 4, Fram_Read(1204) / 10 + 0x30); // 1204���� 1���ڸ�
      LCD_DisplayChar(3, 6, Fram_Read(1204) % 10 + 0x30);// 1204���� 1/10�� �ڸ�
      
      LCD_DisplayChar(4, 4, Fram_Read(1205) / 10 + 0x30);// 1205���� 1�� �ڸ�
      LCD_DisplayChar(4, 6, Fram_Read(1205) % 10 + 0x30); // 1205���� 1/10�� �ڸ�
      
      LCD_DisplayChar(5, 4, Fram_Read(1206) + 0x30); // 1206���� ��
      test = 0;
      SW_Flag[5] = 0; // SW 5 flag =0
      SW_Flag[6] = 0; // SW 6 flag =0
    }
    
    if (SW_Flag[4] && stage == 1) // Fram�� �Է��� ��
    {
      if (Mode % 4 == 0) // �µ��� �Է�
        Fram_Write(1203, time[0]); // 1203���� �Է�
      else if (Mode % 4 == 1) // �з°� �Է�
        Fram_Write(1204, time[1]); // 1204���� �Է�
      else if (Mode % 4 == 2) // ��Ұ� �Է�
        Fram_Write(1205, time[2]); // 1205���� �Է�
      else if (Mode % 4 == 3) // �ö���� �Է�
        Fram_Write(1206, time[3]); // 1206���� �Է�
      SW_Flag[4] = 0;// SW 4 flag =0
    }
  }
}
void _EXTI_Init(void) //���ͷ�Ʈ ����
{
  RCC->AHB1ENR |= 0x0080;	// RCC_AHB1ENR GPIOH Enable
  RCC->APB2ENR |= 0x4000;	// Enable System Configuration Controller Clock
  
  GPIOH->MODER &= 0x00FF;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
  
  SYSCFG->EXTICR[3] |= 0x7777; 	// EXTI15~12�� ���� �ҽ� �Է��� GPIOH�� ���� (EXTICR3) (reset value: 0x0000)	
  
  EXTI->FTSR |= 0xF000;		// Falling Trigger Enable (EXTI15~12:PH15~12)
  EXTI->IMR |= 0xF000;  	// EXTI15~12 ���ͷ�Ʈ mask (Interrupt Enable)
  NVIC->ISER[1] |= (1 << 8);   // Enable Interrupt EXTI15 Vector table Position ����
}

void EXTI15_10_IRQHandler(void)		// EXTI 15~10 ���ͷ�Ʈ �ڵ鷯
{
  if (EXTI->PR & 0x1000) 	// EXTI12 Interrupt Pending?
  {
    EXTI->PR |= 0x1000; 	// Pending bit Clear
    SW_Flag[4] = 1; // SW4�� ���� ��Ȳ�̸� 1�� ����
  }
  if (EXTI->PR & 0x2000) 	// EXTI13 Interrupt Pending?
  {
    EXTI->PR |= 0x2000; 	// Pending bit Clear
    SW_Flag[5] = 1; // SW5�� ���� ��Ȳ�̸� 1�� ����
  }
  if (EXTI->PR & 0x4000) 	// EXTI14 Interrupt Pending?
  {
    EXTI->PR |= 0x4000; 	// Pending bit Clear
    SW_Flag[6] = 1; // SW6�� ���� ��Ȳ�̸� 1�� ����
  }
  if (EXTI->PR & 0x8000) 	// EXTI15 Interrupt Pending?
  {
    EXTI->PR |= 0x8000; 	// Pending bit Clear
    SW_Flag[7] = 1; // SW7�� ���� ��Ȳ�̸� 1�� ����
  }
}
void TIMER2_PWM_Init(void)
{
  // TIM2 CH3 : PB10 
  // Clock Enable : GPIOB & TIMER2
  RCC->AHB1ENR |= (1 << 1);	// GPIOB CLOCK Enable
  RCC->APB1ENR |= (1 << 0);	// TIMER2 CLOCK Enable 
  
  // PB10�� ��¼����ϰ� Alternate function(TIM2_CH3)���� ��� ���� : PWM ���
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
  TIM2->CCER &= ~(1 << 9);	// CC3P=0: CC3 Output Polarity (OCPolarity_High : OC3���� �������� ���)
  
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
  
  TIM3->EGR |= (1 << 0);	// Update generation     0����Ʈ�� 1�� �������.
  
  TIM3->DIER |= (1 << 0);	// Enable the Tim3 Update interrupt // ���ͷ�Ʈ�� ����ϰڴ�.
  TIM3->CR1 |= (1 << 0);	// Enable the Tim3 Counter      // TIM3 ī���͸� ����ϰڴ�. 	
}

void TIM3_IRQHandler() // �ڵ鷯 ����
{
  TIM3->SR &= ~(1 << 0);	// Interrupt flag Clear
  LCD_SetTextColor(RGB_RED); // �۾��� : RED
  
  if (Time_Flag == 1) // Time �÷��� 1
  {
    OnTime[0]++;
    LCD_DisplayChar(2, 9, 0x30 + OnTime[0] / 10); // OnTime[0]�� 1�� �ڸ�
    LCD_DisplayChar(2, 11, 0x30 + OnTime[0] % 10);// OnTime[0]�� 1/10�� �ڸ�
  }
  if (Time_Flag == 2) // Time �÷��� 2
  {
    OnTime[1]++;
    LCD_DisplayChar(3, 9, 0x30 + OnTime[1] / 10);// OnTime[1]�� 1�� �ڸ�
    LCD_DisplayChar(3, 11, 0x30 + OnTime[1] % 10);// OnTime[1]�� 1/10�� �ڸ�
  }
  if (Time_Flag == 3) // Time �÷��� 3
  {
    OnTime[2]++;
    LCD_DisplayChar(4, 9, 0x30 + OnTime[2] / 10);// OnTime[2]�� 1�� �ڸ�
    LCD_DisplayChar(4, 11, 0x30 + OnTime[2] % 10);//// OnTime[2]�� 1/10�� �ڸ�
  }
}
void _ADC_Init(void)
{	// ADC1 PA1(pin 41)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	// (1<<0) ENABLE GPIOA CLK (stm32f4xx.h ����)
  GPIOA->MODER |= (3 << 2 * 1);		// CONFIG GPIOA PIN1(PA1) TO ANALOG IN MODE
  
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;	//  ENABLE ADC1 CLK (stm32f4xx.h ����)
  
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
  // LED (GPIO G) ����
  RCC->AHB1ENR |= 0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
  GPIOG->MODER |= 0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
  GPIOG->OTYPER &= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
  GPIOG->OSPEEDR |= 0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
  
  // SW (GPIO H) ���� 
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

void DisplayTitle(void) // �ʱ� ȭ��
{
  LCD_Clear(RGB_YELLOW); // ������ : �����
  LCD_SetFont(&Gulim10); // ���� : 10
  LCD_SetBackColor(RGB_YELLOW); // ��� : �����
  
  LCD_SetTextColor(RGB_BLUE);	 // �۾��� : �Ķ���
  LCD_DisplayText(0, 0, "SPC monitor");  // �۾�
  
  LCD_SetTextColor(RGB_BLACK);// �۾��� : ������
  LCD_DisplayText(1, 1, "Mode:   Wafer: ");  // �۾�
  LCD_DisplayText(2, 1, "TE:1.0S 0.0S"); // �۾�
  LCD_DisplayText(3, 1, "PR:1.0S 0.0S");// �۾�
  LCD_DisplayText(4, 1, "O2:1.0S 0.0S");// �۾�
  LCD_DisplayText(5, 1, "DR:");// �۾�
  
  //////////////////////////////////////////////
  LCD_SetTextColor(GET_RGB(255, 0, 0)); // �۾��� : ������
  LCD_DisplayChar(1, 6, 'I');// �۾�
  LCD_DisplayChar(1, 15, 'E');// �۾�
  
  LCD_DisplayChar(2, 4, 0x31);// �۾�
  LCD_DisplayChar(3, 4, 0x30);// �۾�
  LCD_DisplayChar(4, 4, 0x30);// �۾�
  
  LCD_DisplayChar(2, 6, 0x30);// �۾�
  LCD_DisplayChar(3, 6, 0x35);// �۾�
  LCD_DisplayChar(4, 6, 0x30);// �۾�
  
  LCD_DisplayChar(2, 9, 0x30);// �۾�
  LCD_DisplayChar(3, 9, 0x30);// �۾�
  LCD_DisplayChar(4, 9, 0x30);// �۾�
  
  LCD_DisplayChar(2, 11, 0x30);// �۾�
  LCD_DisplayChar(3, 11, 0x30);// �۾�
  LCD_DisplayChar(4, 11, 0x30);// �۾�
  
  LCD_DisplayChar(5, 4, 0x31);// �۾�
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
