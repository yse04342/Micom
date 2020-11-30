//////////////////////////////////////////////////////////////////////
// PR1.�ڵ��� ���� �ý���

// �ۼ���: 2015134010 ����ȣ

// �ۼ���: 2019.12.04

/////////////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"
//#include "FRAM.h"

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
void TIMER1_Init(void);
void TIMER2_PWM_Init(void);
void USART1_Init(void);
void USART_BRR_Configuration(uint32_t USART_BaudRate);
void _EXTI_Init(void);
void SerialSendChar(uint8_t c);
void SerialSendString(char *s);
uint16_t ADC_Value, Voltage,V_trans;
uint8_t str[20],Flag;
char ch;
int Count,SW7_Flag;
void ADC_IRQHandler(void)
{
  ADC2->SR &= ~(1<<1);		// EOC flag clear
  
  ADC_Value = ADC2->DR;		// Reading ADC result
  
  //sprintf(str,"%4d",ADC_Value);	// ADC result : 12bits (0~4095)
  //LCD_DisplayText(0,6,str);
  Voltage = ADC_Value * 33 / 1023;   // 33 : 1023 =  Volatge : ADC_Value 
  V_trans = (Voltage-0)*(380-50)/(33)+50;  // �ּҰ� 5, �ִ밪 38���� ��ȯ�ϴ� �Լ� (Map�Լ�)
  
  if(Flag== 0x4D) // [�ƽ�Ű�ڵ�] �÷��װ� 'S'���?
  {
    LCD_DisplayChar(1,17,V_trans/100 + 0x30);// m�� 10���ڸ� ǥ��
    LCD_DisplayChar(1,18,V_trans%100/10 + 0x30); // m�� 1�� �ڸ� ǥ��
    LCD_SetPenColor(RGB_YELLOW); // �����
    LCD_SetBrushColor(RGB_YELLOW); // �����
    LCD_DrawFillRect(13,20,120,12); // ����� �簢��(�����)
    LCD_SetPenColor(RGB_RED);// ������ �� ����
    LCD_SetBrushColor(RGB_RED);// ���� �귯�� ���� 
    LCD_DrawFillRect(13,20,10+Voltage*3.3,12); //���� �簢�� ����
    if(V_trans>=50 && V_trans<90) // 5m ~ 8m�̸�?
    {
      TIM2->CCR4	= 5000;		// CCR4 value  5000 ->10%
      LCD_DisplayChar(2,6,0x30+1);    // DR 10% ǥ��
    }
    else if(V_trans>=90 && V_trans<130) // 9m ~ 12m�̸�?
    {
      TIM2->CCR4	= 10000;		// CCR4 value 10000 ->20%
      LCD_DisplayChar(2,6,0x30+2);    // DR 20% ǥ��
    }
    else if(V_trans>=130 && V_trans<170)// 13m ~ 16m�̸�?
    {
      TIM2->CCR4	= 15000;		// CCR4 value 15000 ->30%
      LCD_DisplayChar(2,6,0x30+3);    // DR 30% ǥ��
    }
    else if(V_trans>=170 && V_trans<210)// 17m ~ 20m�̸�?
    {
      TIM2->CCR4	= 20000;		// CCR4 value 20000 ->40%
      LCD_DisplayChar(2,6,0x30+4);    // DR 40% ǥ��
    }
    else if(V_trans>=210 && V_trans<250) // 21m ~ 24m�̸�?
    {
      TIM2->CCR4	= 25000;		// CCR4 value 25000 ->50%
      LCD_DisplayChar(2,6,0x30+5);    // DR 50% ǥ��
    }
    else if(V_trans>=250 && V_trans<290) // 25m ~ 28m�̸�?
    {
      TIM2->CCR4	= 30000;		// CCR4 value 30000 ->60%
      LCD_DisplayChar(2,6,0x30+6);    // DR 60% ǥ��
    }
    else if(V_trans>=290 && V_trans<330) // 29m ~ 32m�̸�?
    {
      TIM2->CCR4	= 35000;		// CCR4 value 35000 ->70%
      LCD_DisplayChar(2,6,0x30+7);    // DR 70% ǥ��
    }
    else if(V_trans>=330 && V_trans<370) // 33m ~ 36m�̸�?
    {
      TIM2->CCR4	= 40000;		// CCR4 value 40000 ->80%
      LCD_DisplayChar(2,6,0x30+8);    // DR 80% ǥ��
    }
    else if(V_trans>=370 && V_trans<390)// 37m ~ 38m�̸�?
    {
      TIM2->CCR4	= 45000;		// CCR4 value 45000 ->90%
      LCD_DisplayChar(2,6,0x30+9);    // DR 90% ǥ��
    }
    Fram_Write(1126,Flag); //FRAM 1126������ Flag��(M) ����
    LCD_DisplayChar(2,18,Fram_Read(1126)); //FRAM 1126���� ����� data(1byte) �о�´�.
    SerialSendChar(V_trans/100 + 0x30); // USART 1 10���ڸ� ����
    SerialSendChar(V_trans%100/10 + 0x30); // USART 1 1���ڸ� ����
    SerialSendChar('m');// USART 1 'm' ����
    SerialSendChar(' '); // // USART 1 ���� ����
  }
  else if(Flag==0x53) // [�ƽ�Ű�ڵ�] �÷��װ� 'S'���?
  {
    V_trans=0; // ��ȯ�� 0 ǥ��
    TIM2->CCR4	= 0;		// CCR4 value
    Fram_Write(1126,Flag); //FRAM 1126������ Flag��(S) ����
    LCD_SetPenColor(RGB_YELLOW); // �����
    LCD_SetBrushColor(RGB_YELLOW); // �����
    LCD_DrawFillRect(13,20,120,12); // ����� �簢��(�����)
    LCD_DisplayChar(2,6,0x30);    // DR 00% ǥ��
    LCD_DisplayText(1,17,"00"); // 0m ǥ��
    LCD_DisplayChar(2,18,Fram_Read(1126)); //FRAM 1126���� ����� data(1byte) �о�´�.
    SerialSendString("00m");// USART1 00m �۽�
    SerialSendChar(' ');// USART1 ���� �۽�
  }
}
int main(void)
{
  LCD_Init();	// LCD ���� �Լ�
  DelayMS(10);	// LCD���� ������
  DisplayTitle();	//LCD �ʱ�ȭ�鱸�� �Լ�
  
  _GPIO_Init();//GPIO ����
  _ADC_Init();//ADC ����
  _EXTI_Init();// ���ͷ�Ʈ ����
  TIMER1_Init(); // Ÿ�̸� 1��(CC���) ����
  TIMER2_PWM_Init(); //Ÿ�̸� 2��(PWM) ����
  USART1_Init();// USART 1�� ����
  Fram_Init();        // FRAM �ʱ�ȭ 
  Fram_Status_Config();  // FRAM ���� ����
  Flag=Fram_Read(1126);// �ʱ� Flag�� Fram 1126������ ����
  LCD_DisplayChar(2,18,Fram_Read(1126)); //FRAM 1126���� ����� data(1byte) �о�´�.
  while(1)
  {
    switch(KEY_Scan()) // Ű��ĵ ����
    {
    case 0xFE00 :  //SW0���� ������?
      Flag=0x4D; // Flag�� 'M'����
      Fram_Write(1126,Flag);//Fram 1126������ Flag����
      LCD_DisplayChar(2,18,Fram_Read(1126)); //FRAM 1126���� ����� data(1byte) �о�´�.
      Count=0;// Count �� 0���� ����
      break;
    }
    if(SW7_Flag) // [���ͷ�Ʈ] SW7�� ������?
    {
      Count=1; //Count�� 1������
      Flag=0x53; // Flag�� 'S'����
      Fram_Write(1126,Flag);//FRAM 1126���� ����� data(1byte) �о�´�.
      SW7_Flag=0; // SW7_Flag�� 0���� ���� (�ѹ��� ������ ����)
    }
  }
}
void _EXTI_Init(void) //���ͷ�Ʈ ����
{
  RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
  RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
  
  GPIOH->MODER 	&= 0x00FF;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
  
  SYSCFG->EXTICR[3] |= 0x7000; 	// EXTI15�� ���� �ҽ� �Է��� GPIOH�� ���� (EXTICR3) (reset value: 0x0000)	
  
  EXTI->FTSR |= 0x8000;		// Falling Trigger Enable (EXTI15:PH15)
  EXTI->IMR  |= 0x8000;  	// EXTI15 ���ͷ�Ʈ mask (Interrupt Enable)
  NVIC->ISER[1] |= ( 1 << 8 );   // Enable Interrupt EXTI15 Vector table Position ����
}

void EXTI15_10_IRQHandler(void)		// EXTI 15~10 ���ͷ�Ʈ �ڵ鷯
{
  if(EXTI->PR & 0x8000) 	// EXTI15 Interrupt Pending?
  {
    EXTI->PR |= 0x8000; 	// Pending bit Clear
    SW7_Flag=1; // SW7�� ���� ��Ȳ�̸� 1�� ����
  }
}
void _ADC_Init(void)
{	// ADC2: PA1(pin 41)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	// (1<<0) ENABLE GPIOA CLK (stm32f4xx.h ����)
  GPIOA->MODER |= (3<<2*1);		// CONFIG GPIOA PIN1(PA1) TO ANALOG IN MODE
  
  RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;	//  ENABLE ADC2 CLK (stm32f4xx.h ����)
  
  ADC->CCR &= ~(0X1F<<0);		// MULTI[4:0]: ADC_Mode_Independent
  ADC->CCR |=  (1<<16); 		// 0x00010000 ADCPRE:ADC_Prescaler_Div4 (ADC MAX Clock 36MHz, 84Mhz(APB2)/4 = 21MHz)
  
  ADC2->CR1 |= (1<<24) ;		// RES[1:0]= :10bit Resolution
  ADC2->CR1 &= ~(1<<8);		// SCAN=0 : ADC_ScanCovMode Disable
  ADC2->CR1 |= (1<<5);		// EOCIE=1: Interrupt enable for EOC
  
  ADC2->CR2 &= ~(1<<1);		// CONT=0: ADC_Continuous ConvMode Disable
  
  ADC2->CR2 |=  (2<<28);		// EXTEN[1:0]: ADC_ExternalTrigConvEdge_Enable  (Falling Edge) // Ÿ�̸Ӹ� ����غ��� �ʹ�.
  ADC2->CR2 |= (2<<24);	        // Timer 1 CC3 Event Enable 
  
  ADC2->CR2 &= ~(1<<11);		// ALIGN=0: ADC_DataAlign_Right
  ADC2->CR2 &= ~(1<<10);		// EOCS=0: The EOC bit is set at the end of each sequence of regular conversions
  
  ADC2->SQR1 &= ~(0xF<<20);	// L[3:0]=0b0000: ADC Regular channel sequece length 
  // 0b0000:1 conversion)
  //Channel selection, The Conversion Sequence of PIN1(ADC2_CH1) is first, Config sequence Range is possible from 0 to 17
  ADC2->SQR3 |= (1<<0);		// SQ1[4:0]=0b0001 : CH1
  ADC2->SMPR2 |= (0x7<<(3*1));	// ADC2_CH1 Sample TIme_480Cycles (3*Channel_1)
  //Channel selection, The Conversion Sequence of PIN1(ADC2_CH1) is first, Config sequence Range is possible from 0 to 17
  
  NVIC->ISER[0] |= (1<<18);	// Enable ADC global Interrupt
  
  ADC2->CR2 |= (1<<0);		// ADON: ADC ON
  
}
void TIMER1_Init(void)
{
  // TIM1_CH3 (PE13) : 300ms ���ͷ�Ʈ �߻�
  // Clock Enable : GPIOE & TIMER13
  RCC->AHB1ENR	|= (1<<4);	// GPIOE Enable
  RCC->APB2ENR	|= (1<<0);	// TIMER1 Enable 
  
  // PE13�� ��¼����ϰ� Alternate function(TIM1_CH3)���� ��� ���� 
  GPIOE->MODER 	|= (2<<2*13);	// PE13 Output Alternate function mode					
  GPIOE->OSPEEDR 	|= (3<<2*13);	// PE13 Output speed (100MHz High speed)
  GPIOE->OTYPER	&= ~(1<<13);	// PE13 Output type push-pull (reset state)
  GPIOE->AFR[1]	|= (1 <<20); 	// Connect TIM1 pins(PE13) to AF1(TIM1..2)
  
  // Assign 'Interrupt Period' and 'Output Pulse Period'
  TIM1->PSC = 1680-1;	// Prescaler 168MHz/1680 = 0.1MHz (10us)
  TIM1->ARR = 30000-1;	// Auto reload  : 10us * 30K = 300ms(period)
  
  // CR1 : Up counting
  TIM1->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
  TIM1->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
  //	- Counter Overflow/Underflow, 
  // 	- Setting the UG bit Set,
  //	- Update Generation through the slave mode controller 
  TIM1->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
  //	- Counter Overflow/Underflow, 
  // 	- Setting the UG bit Set,
  //	- Update Generation through the slave mode controller 
  TIM1->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
  TIM1->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
  TIM1->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
  TIM1->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
  // Center-aligned mode: The counter counts Up and DOWN alternatively
  
  // Event & Interrup Enable : UI  
  TIM1->EGR |= (1<<0);    // UG: Update generation    
  
  ////////////////////////////////
  // Disable Tim1 Update interrupt
  // Define the corresponding pin by 'Output' 
  TIM1->CCER |= (1<<8);	// CC3E=1: CC3 channel Output Enable
  // OC3(TIM1_CH3) Active: �ش����� ���� ��ȣ���
  TIM1->CCER &= ~(1<<9);	// CC3P=0: CC3 channel Output Polarity (OCPolarity_High : OC3���� �������� ���)  
  
  // 'Mode' Selection : Output mode, toggle  
  TIM1->CCMR2 &= ~(3<<0); // CC3S(CC3 channel) = '0b00' : Output 
  TIM1->CCMR2 &= ~(1<<3); // OC3P=0: Output Compare 3 preload disable
  TIM1->CCMR2 |= (3<<4);	// OC3M=0b011: Output Compare 3 Mode : toggle
  // OC3REF toggles when CNT = CCR3
  TIM1->CCR3 = 3000;	// TIM1 CCR3 TIM1_Pulse
  TIM1->BDTR = (1<<15);
  ////////////////////////////////
  // Disable Tim1 CC3 interrupt
  TIM1->CR1	|= (1<<7);	// ARPE: Auto-reload preload enable
  TIM1->CR1 |= (1<<0);	// CEN: Enable the Tim1  					
}
void TIMER2_PWM_Init(void)
{   
  // TIM2 CH4 : PB11 
  // Clock Enable : GPIOB & TIMER2
  RCC->AHB1ENR	|= (1<<1);	// GPIOB CLOCK Enable
  RCC->APB1ENR 	|= (1<<0);	// TIMER2 CLOCK Enable 
  
  // PB11�� ��¼����ϰ� Alternate function(TIM2_CH4)���� ��� ���� : PWM ���
  GPIOB->MODER 	|= (2<<2*11);	//  PB11 Output Alternate function mode					
  GPIOB->OSPEEDR 	|= (3<<2*11);	// PB11 Output speed (100MHz High speed)
  GPIOB->OTYPER	&= ~(1<<11);	// PB11 Output type push-pull (reset state)
  GPIOB->PUPDR	|= (1<<2*11);	// PB11 Pull-up
  GPIOB->AFR[1]	|= (1<<4*3);	// (AFR[1].(15~12)=0b0001): Connect TIM2 pins(PA3) to AF1(TIM1..2)
  
  // TIM2 Channel 4 : PWM 1 mode
  // Assign 'PWM Pulse Period'
  TIM2->PSC	= 8400-1;	// Prescaler 84,000,000Hz/8400 = 10,000 Hz(0.1ms)  (1~65536)
  TIM2->ARR	= 50000-1;	// Auto reload  (0.1ms * 50000= 5s : PWM Period)
  
  // Setting CR1 : 0x0000 (Up counting)
  TIM2->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state5)
  TIM2->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
  //	- Counter Overflow/Underflow, 
  // 	- Setting the UG bit Set,
  //	- Update Generation through the slave mode controller 
  TIM2->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
  //	- Counter Overflow/Underflow, 
  // 	- Setting the UG bit Set,
  //	- Update Generation through the slave mode controller 
  TIM2->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
  TIM2->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
  TIM2->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
  TIM2->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
  // Center-aligned mode: The counter counts Up and DOWN alternatively
  
  // Define the corresponding pin by 'Output'  
  // CCER(Capture/Compare Enable Register) : Enable "Channel 4" 
  TIM2->CCER	|= (1<<12);	// CC4E=1: OC4(TIM2_CH4) Active(Capture/Compare 4 output enable
  TIM2->CCER	&= ~(1<<13);	// CC4P=0: CC4 Output Polarity (OCPolarity_High : OC4���� �������� ���)
  
  // Duty Ratio 
  TIM2->CCR4	= 500;		// CCR4 value
  
  // 'Mode' Selection : Output mode, PWM 1
  // CCMR2(Capture/Compare Mode Register 2) : Setting the MODE of Ch3 or Ch4
  TIM2->CCMR2 	&= ~(3<<8); 	// CC4S(CC3 channel)= '0b00' : Output 
  TIM2->CCMR2 	|= (1<<11); 	// OC4PE=1: Output Compare 4 preload Enable
  TIM2->CCMR2	|= (6<<12);	// OC4M=0b110: Output compare 4 mode: PWM 1 mode
  TIM2->CCMR2	|= (1<<15);	// OC4CE=1: Output compare 4 Clear enable
  
  //Counter TIM2 enable
  TIM2->CR1	|= (1<<7);	// ARPE: Auto-reload preload enable
  TIM2->CR1	|= (1<<0);	// CEN: Counter TIM2 enable
}
void USART1_Init(void)
{
  // USART1 : TX(PA9)
  RCC->AHB1ENR	|= (1<<0);	// RCC_AHB1ENR GPIOA Enable
  GPIOA->MODER	|= (2<<2*9);	// GPIOB PIN9 Output Alternate function mode					
  GPIOA->OSPEEDR	|= (3<<2*9);	// GPIOB PIN9 Output speed (100MHz Very High speed)
  GPIOA->AFR[1]	|= (7<<4);	// Connect GPIOA pin9 to AF7(USART1)
  
  // USART1 : RX(PA10)
  GPIOA->MODER 	|= (2<<2*10);	// GPIOA PIN10 Output Alternate function mode
  GPIOA->OSPEEDR	|= (3<<2*10);	// GPIOA PIN10 Output speed (100MHz Very High speed
  GPIOA->AFR[1]	|= (7<<8);	// Connect GPIOA pin10 to AF7(USART1)
  
  RCC->APB2ENR	|= (1<<4);	// RCC_APB2ENR USART1 Enable
  
  USART_BRR_Configuration(9600); // USART Baud rate Configuration
  
  USART1->CR1	&= ~(1<<12);	// USART_WordLength 8 Data bit
  USART1->CR1	&= ~(1<<10);	// NO USART_Parity
  USART1->CR1	|= (1<<2);	// 0x0004, USART_Mode_RX Enable
  USART1->CR1	|= (1<<3);	// 0x0008, USART_Mode_Tx Enable
  USART1->CR2	&= ~(3<<12);	// 0b00, USART_StopBits_1
  USART1->CR3	= 0x0000;	// No HardwareFlowControl, No DMA
  
  USART1->CR1 	|= (1<<5);	// 0x0020, RXNE interrupt Enable
  NVIC->ISER[1]	|= (1<<(37-32));// Enable Interrupt USART1 (NVIC 37��)
  USART1->CR1 	|= (1<<13);	//  0x2000, USART1 Enable
}
void USART1_IRQHandler(void)	
{       
  if ( (USART1->SR & USART_SR_RXNE) ) 	// USART_SR_RXNE=(1<<5) 
  {
    ch = (uint16_t)(USART1->DR & (uint16_t)0x01FF);	// ���ŵ� ���� ����
    if(ch == 'M') // ����� ���ڰ� M�̶��?
    {
      Flag=0x4D; // Flag�� M���� ����
    }
    else if(ch=='S')// ����� ���ڰ� S���?
    {
      Flag=0x53; // Flag�� S���� ����
    }
  } 
  // DR �� ������ SR.RXNE bit(flag bit)�� clear �ȴ�. �� clear �� �ʿ���� 
}
void SerialSendChar(uint8_t Ch) // 1���� ������ �Լ�
{
  while((USART1->SR & USART_SR_TXE) == RESET); // USART_SR_TXE=(1<<7), �۽� ������ ���±��� ���
  
  USART1->DR = (Ch & 0x01FF);	// ���� (�ִ� 9bit �̹Ƿ� 0x01FF�� masking)
}


void SerialSendString(char *str) // �������� ������ �Լ�
{
  while (*str != '\0') // ���Ṯ�ڰ� ������ ������ ����, ���Ṯ�ڰ� �����Ŀ��� ������ �޸� ���� �߻����ɼ� ����.
  {
    SerialSendChar(*str);	// �����Ͱ� ����Ű�� ���� �����͸� �۽�
    str++; 			// ������ ��ġ ����
  }
}

// Baud rate  
void USART_BRR_Configuration(uint32_t USART_BaudRate)
{ 
  uint32_t tmpreg = 0x00;
  uint32_t APB2clock = 84000000;	//PCLK2_Frequency
  uint32_t integerdivider = 0x00;
  uint32_t fractionaldivider = 0x00;
  
  // Determine the integer part 
  if ((USART1->CR1 & USART_CR1_OVER8) != 0) // USART_CR1_OVER8=(1<<15)
  {                                         // USART1->CR1.OVER8 = 1 (8 oversampling)
    // Computing 'Integer part' when the oversampling mode is 8 Samples 
    integerdivider = ((25 * APB2clock) / (2 * USART_BaudRate));    
  }
  else  // USART1->CR1.OVER8 = 0 (16 oversampling)
  {	// Computing 'Integer part' when the oversampling mode is 16 Samples 
    integerdivider = ((25 * APB2clock) / (4 * USART_BaudRate));    
  }
  tmpreg = (integerdivider / 100) << 4;
  
  // Determine the fractional part 
  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));
  
  // Implement the fractional part in the register 
  if ((USART1->CR1 & USART_CR1_OVER8) != 0)	// 8 oversampling
  {
    tmpreg |= (((fractionaldivider * 8) + 50) / 100) & (0x07);
  }
  else 						// 16 oversampling
  {
    tmpreg |= (((fractionaldivider * 16) + 50) / 100) & (0x0F);
  }
  // Write to USART BRR register
  USART1->BRR = (uint16_t)tmpreg;
}

void _GPIO_Init(void)
{
  // LED (GPIO G) ����
  RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
  GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
  GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
  GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
  
  // SW (GPIO H) ���� 
  RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
  GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
  GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state
  
  // Buzzer (GPIO F) ���� 
  RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
  GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
  GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
  GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
  
  //NAVI.SW(PORT I) ����
  RCC->AHB1ENR 	|= 0x00000100;	// RCC_AHB1ENR GPIOI Enable
  GPIOI->MODER 	= 0x00000000;	// GPIOI PIN8~PIN15 Input mode (reset state)
  GPIOI->PUPDR    = 0x00000000;	// GPIOI PIN8~PIN15 Floating input (No Pull-up, pull-down) (reset state)
}	

void BEEP(void)			// Beep for 20 ms 
{ 	GPIOF->ODR |= (1<<9);	// PF9 'H' Buzzer on
DelayMS(20);		// Delay 20 ms
GPIOF->ODR &= ~(1<<9);	// PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
  register unsigned short i;
  for (i=0; i<wMS; i++)
    DelayUS(1000);   // 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
  volatile int Dly = (int)wUS*17;
  for(; Dly; Dly--);
}

void DisplayTitle(void)
{
  LCD_Clear(RGB_YELLOW);
  LCD_SetFont(&Gulim10);		//��Ʈ 
  LCD_SetBackColor(RGB_YELLOW);	//���ڹ���
  LCD_SetTextColor(RGB_BLACK);	//���ڻ�
  LCD_DisplayText(0,0,"Tracking Car"); // ���� ���
  LCD_DisplayText(1,0,"D:"); // D ���
  LCD_DisplayText(2,0,"S(DR):00%"); // S(DR)���
  LCD_DisplayText(1,17,"00"); // 00 ���
  LCD_SetBackColor(RGB_YELLOW);	//���ڹ���
}

uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
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
