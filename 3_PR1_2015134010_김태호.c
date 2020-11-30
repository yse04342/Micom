#include "stm32f4xx.h"
#include "GLCD.h"
#include <math.h>
void _GPIO_Init(void);
uint16_t KEY_Scan(void);
void _EXTI_Init(void);//������ ���� ���°�.

void BEEP(void); // ������
void DisplayInitScreen(void); //LCD �ʱ�ȭ�� ����
void DelayMS(unsigned short wMS); // �ð� ������
void DelayUS(unsigned short wUS);// �ð� ������
void BEEP3(void); // ������ ���� 3��
void Floor(void); // �� ���� ���
void First_Floor(void);// 1�� ���� �Լ�
void Second_Floor(void);// 2�� ���� �Լ�
void Third_Floor(void);// 3�� ���� �Լ�
void Foured_Floor(void);// 4�� ���� �Լ�
void Fifth_Floor(void);// 5�� ���� �Լ�
void Sixed_Floor(void);// 6�� ���� �Լ�
uint8_t	SW0_Flag,SW7_Flag; // ����ġ 0�� 7�� ��������
uint8_t i=0,j=0,Start_Floor=1,Final_Floor=1,count=0,swap=1,b=0;  //Start_Floor : ������,Final_Floor: ���� ,b :�� ��° ���ȴ��� Ȯ�� ���� swap : ��� �� ->��ǥ�� ���� Ű �������� ������ b
uint8_t One_Elevator=1,Two_Elevator=1;//���� ���������Ϳ� ������ ���������� ��������
void Execute(void);//������ �Լ�
void Hold(void);//�ߴܸ��(SW0�� ������ ��)
void Time_Delay(void);//������ 5�� �Լ�sadasdasdsadas
int main(void)
{
  _GPIO_Init(); // GPIO (LED & SW) �ʱ�ȭ
  LCD_Init();	// LCD ��� �ʱ�ȭ
  _EXTI_Init();// �ܺ� ���ͷ�Ʈ�� ���� �ʱ�ȭ
  DelayMS(100);
  //BEEP();
  
  
  DisplayInitScreen();	// LCD �ʱ�ȭ��
  LCD_SetTextColor(RGB_BLACK);// ���ڻ� : BULE
  while(1)
  {
    
    Floor();
    
    if(SW7_Flag&&count==0&&b==1)// ����ġ 7���� ������  ���� Ƚ���� ¦�������̰� �ʱ� ���°� �ƴ϶��?(b�� �ʱ���� ���� �ϱ����� ����)
    {
      
      SW0_Flag=0;//SW0�� �÷��׺����� 0�̴�. => �����忡 ���� ������ ������ ��� �� �ٷ� �ߴ� ��尡 ���� �ȵǰ� ���ִ� ��
      Execute(); ///������� ����.
      SW7_Flag=0;//SW7�� �÷��׺����� 0�̴�. 
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
  
  SYSCFG->EXTICR[2] |= 0x0007; 	// EXTI8�� ���� �ҽ� �Է��� GPIOH�� ���� (EXTICR3) (reset value: 0x0000)//PH8->EXTI8�� �ٲ��ִ� ����
  EXTI->FTSR |= 0x0100;		// Falling Trigger Enable  (EXTI8:PH8)//�������� ����
  EXTI->IMR  |= 0x0100;  	// EXTI8 ���ͷ�Ʈ mask (Interrupt Enable) //IMR �� ����Ѵ�.
  NVIC->ISER[0] |= ( 1<<23  );   // Enable Interrupt EXTI8,9 Vector table Position ���� //ISER[0] ~ [7]���� �ִ�. [0]�� (0������ 31������ ����Ѵ�. 32�� [1]�� �ִ�.) 1������ 32bit
  //NVIC->ISER[0] |= ( 0x00800000  );
  
  SYSCFG->EXTICR[3] |= 0x7000; 	//EXIT15�� ���� �ҽ� �Է��� GPIOH�� ����// PH15->EXIT15	
  EXTI->FTSR |= 0xE000;		// Falling Trigger Enable  (15,14,13��)//�������� ����
  EXTI->IMR  |= 0xE000;  	// EXTI15,14,13 ���ͷ�Ʈ mask (Interrupt Enable) //IMR �� ����Ѵ�.// 0xE000 =0b 1110 0000 0000 0000
  NVIC->ISER[1] |= ( 1<<8  ); //  �ܺ� ���ͷ�Ʈ �ѹ��� 40���̴�. �̋� 32�� ���� �̹Ƿ�  ISER[1]�� 8��°(32+8)�� �ִ�.
}

void _GPIO_Init(void)
{
  // LED (GPIO G) ����
  RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
  GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
  GPIOG->OTYPER	&= ~0x000000FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
  GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
  
  // SW (GPIO H) ���� 
  RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
  GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
  GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state
  
  // Buzzer (GPIO F) ���� 
  RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
  GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
  GPIOF->OTYPER 	&= ~0x00000200;	// GPIOF 9 : Push-pull  	
  GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
  LCD_SetBackColor(RGB_YELLOW);	//���ڹ��� : Yellow
  LCD_SetBackColor(RGB_YELLOW);	//���ڹ��� : Yellow
}	

void DisplayInitScreen(void) // �ʱ� ȭ�� ����
{
  GPIOG->ODR |= 0x0001; 	// GPIOG->ODR.5 'H'(LED ON)
  LCD_Clear(RGB_YELLOW);	// ȭ�� Ŭ����(��� : �����)
  LCD_SetFont(&Gulim8);	// ��Ʈ : ���� 8
  LCD_SetBackColor(GET_RGB(255,0,255));// ���ڹ��� : YELLOW
  LCD_SetTextColor(RGB_BLACK);// ���ڻ� : BLACK
  LCD_DisplayText(0,0,"Mecha-Elevator");  // ù ��° ����(Title) ���
  LCD_DisplayText(5,6,"1->1");  // ù ��° ����(Title) ���
  
  LCD_SetTextColor(RGB_BLUE);// ���ڻ� :  BLUE
  
  for(int a=2; a<=7; a++) // ���� ���������� 1������ 6������ ǥ�ø� ���ش�.
  {
    LCD_DisplayChar(a,4,0x30+(8-a));  //���� ���������� 1������ 6������ ǥ�ø� ���ش�.
  }
  LCD_DisplayText(0,16,"FL");  // FL���
  LCD_DisplayText(4,6,"L:S");  // ���� ���������� �������
  
  LCD_SetTextColor(RGB_RED);// ���ڻ� : RED
  for(int a=2; a<=7; a++) // ������ ����������  1������ 6������ ǥ�ø� ���ش�.
  {
    LCD_DisplayChar(a,12,0x30+(8-a)); // ������ ����������  1������ 6������ ǥ�ø� ���ش�.
  }
  LCD_SetBrushColor(RGB_BLUE); // �귯���� : �Ķ���
  LCD_DrawFillRect(15,92,10,12); // �簢��  ���
  
  
  LCD_SetBrushColor(RGB_RED); // �귯���� : �Ķ���
  LCD_DrawFillRect(105,92,10,12); // �簢�� ����
}

uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7  // ����ġ �ѹ� ������ ���ִ� ��
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

void EXTI9_5_IRQHandler(void)		// EXTI 5~9 ���ͷ�Ʈ �ڵ鷯 //��� �̹� ������־ �׳� ���� �ȴ�.
{
  if(EXTI->PR & 0x0100) 		// EXTI8 Interrupt Pending?
  {
    EXTI->PR |= 0x0100; 		// Pending bit Clear//PR = Pending Register 
    SW0_Flag = 1;
  }
}

void EXTI15_10_IRQHandler(void)		// EXTI 15~10 ���ͷ�Ʈ �ڵ鷯 //��� �̹� ������־ �׳� ���� �ȴ�.
{
  if(EXTI->PR & 0x8000) 		// EXTI15 Interrupt Pending?
  {
    EXTI->PR |= 0x8000; 		// Pending bit Clear//PR = Pending Register 
    SW7_Flag = 1;
  }
}
void BEEP(void)			/* beep for 30 ms */ // ������ 1�� ��� �Լ�
{ 	
  	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
  	DelayMS(30);		// Delay 30 ms
  	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}

void BEEP3(void)			/* beep for 30 ms */ // ������ 3�� ��� �Լ�
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
  SW7_Flag=0;//SW7�� �÷��׺����� 0�̴�. 
  
  switch(KEY_Scan())//if�� ���ٴ� key_scan()�� �ϸ� Chattering������ �Ͼ�� ����.
  {
  case 0xFD00 : 	//SW1�� �����ٸ�
    First_Floor();
    BEEP();
    break;
    
  case 0xFB00  : 	//SW2�� �����ٸ�
    Second_Floor();
    BEEP();
    break;
    
  case 0xF700 : 	//SW3�� �����ٸ�
    Third_Floor();
    BEEP();
    break;
    
  case 0xEF00 : 	//SW4�� �����ٸ�
    Foured_Floor();
    BEEP();
    break;
    
  case 0xDF00 : 	//SW5�� �����ٸ� 
    Fifth_Floor();
    BEEP();
    break;
    
  case 0xBF00 : 	//SW6�� �����ٸ�
    Sixed_Floor();
    BEEP();
    break;
  }
}
void First_Floor(void)//1��
{
  GPIOG->ODR &= 1<<0;//����ġ 0���� �����ϰ� ��� OFF
  if(count==0)// ���� count�� 0�̶��
  {
    Start_Floor=1;// ������ :1��
    LCD_DisplayChar(5,6,0x30+Start_Floor);  // ���� Start_Floor->Final_Floor ���
    count=1; // count�� 1
    GPIOG->ODR |= 1<<Start_Floor; // �������� LED�� Ų��
  }
  else if(count==1)// �ƴϸ� count�� 1�̶��
  {
    swap=Final_Floor;// �켱 ������ ����� �������� �����ϰ�
    Final_Floor=1;// ���� ���� 1��
    GPIOG->ODR |= 1<<Final_Floor; // LED�� ������ ���� �ش��ϴ� ��ȣ�� Ų��
    
    if(Start_Floor!=Final_Floor)// ���� �������� �������� ���� �ʴٸ�?
    {
      LCD_DisplayChar(5,9,0x30+Final_Floor);  // ���� Start_Floor->Final_Floor ���
      count=0;//count�� 0
      b=1;// b=1�� �����Ѵ�.(b=1�̸� SW7���� ������ �����Ͽ���.
    }
    else // ���� ���ٸ�?(��ȭ�� ������Ѵ�.)
    {
      Final_Floor=swap;// ����Ǿ��� ���������� �ٽ� �ҷ��´�.
      count=1;// �״�� count�� 1
    }
    
  }
}
void Second_Floor(void)//2��
{
  GPIOG->ODR &= 0x0001;//����ġ 0���� �����ϰ� ��� OFF
  if(count==0)// ���� count�� 0�̶��
  {
    Start_Floor=2;// ������ :2��
    GPIOG->ODR |= 1<<Start_Floor;// �������� LED�� Ų��
    LCD_DisplayChar(5,6,0x30+Start_Floor); // ���� Start_Floor->Final_Floor ���
    count=1;// count�� 1
  }
  else if(count==1)// �ƴϸ� count�� 1�̶��
  {
    swap=Final_Floor;// �켱 ������ ����� �������� �����ϰ�
    Final_Floor=2;// ���� ���� 2��
    GPIOG->ODR |= 1<<Final_Floor;// LED�� ������ ���� �ش��ϴ� ��ȣ�� Ų��
    if(Start_Floor!=Final_Floor)// ���� �������� �������� ���� �ʴٸ�?
    {
      LCD_DisplayChar(5,9,0x30+Final_Floor);// ���� Start_Floor->Final_Floor ���
      count=0;//count�� 0
      b=1;// b=1�� �����Ѵ�.(b=1�̸� SW7���� ������ �����Ͽ���.
      
    }
    else// ���� ���ٸ�?(��ȭ�� ������Ѵ�.)
    {
      Final_Floor=swap;// ����Ǿ��� ���������� �ٽ� �ҷ��´�.
      count=1; // �״�� count�� 1
    }
  }
}
void Third_Floor(void)
{
  GPIOG->ODR &= 0x0001;//����ġ 0���� �����ϰ� ��� OFF
  if(count==0)// ���� count�� 0�̶��
  {
    Start_Floor=3;// ������ :3��
    GPIOG->ODR |= 1<<Start_Floor;// �������� LED�� Ų��
    LCD_DisplayChar(5,6,0x30+Start_Floor);  // ���� Start_Floor->Final_Floor ���
    count=1;// count�� 1
  }
  else if(count==1)// �ƴϸ� count�� 1�̶��
  {
    swap=Final_Floor;// �켱 ������ ����� �������� �����ϰ�
    Final_Floor=3;// ���� ���� 3��
    GPIOG->ODR |= 1<<Final_Floor;// LED�� ������ ���� �ش��ϴ� ��ȣ�� Ų��
    if(Start_Floor!=Final_Floor)// ���� �������� �������� ���� �ʴٸ�?
    {
      LCD_DisplayChar(5,9,0x30+Final_Floor);  // ���� Start_Floor->Final_Floor ���
      count=0;//count�� 0
      b=1;// b=1�� �����Ѵ�.(b=1�̸� SW7���� ������ �����Ͽ���.
      
    }
    else// ���� ���ٸ�?(��ȭ�� ������Ѵ�.)
    {
      Final_Floor=swap;// ����Ǿ��� ���������� �ٽ� �ҷ��´�.
      count=1; // �״�� count�� 1
    }
  }
}
void Foured_Floor(void)
{
  GPIOG->ODR &= 0x0001;//����ġ 0���� �����ϰ� ��� OFF
  if(count==0)// ���� count�� 0�̶��
  {
    
    Start_Floor=4;// ������ :4��
    GPIOG->ODR |= 1<<Start_Floor;// �������� LED�� Ų��
    LCD_DisplayChar(5,6,0x30+Start_Floor); // ���� Start_Floor->Final_Floor ���
    count=1;// count�� 1
  }
  else if(count==1)// �ƴϸ� count�� 1�̶��
  {
    swap=Final_Floor;// �켱 ������ ����� �������� �����ϰ�
    Final_Floor=4;// ���� ���� 4��
    GPIOG->ODR |= 1<<Final_Floor;// LED�� ������ ���� �ش��ϴ� ��ȣ�� Ų��
    if(Start_Floor!=Final_Floor)// ���� �������� �������� ���� �ʴٸ�?
    {
      LCD_DisplayChar(5,9,0x30+Final_Floor); // ���� Start_Floor->Final_Floor ���
      count=0;//count�� 0
      b=1;// b=1�� �����Ѵ�.(b=1�̸� SW7���� ������ �����Ͽ���.
      
    }
    else// ���� ���ٸ�?(��ȭ�� ������Ѵ�.)
    {
      Final_Floor=swap;// ����Ǿ��� ���������� �ٽ� �ҷ��´�.
      count=1; // �״�� count�� 1
    }
  }
}
void Fifth_Floor(void)
{
  GPIOG->ODR &= 0x0001;//����ġ 0���� �����ϰ� ��� OFF
  if(count==0)// ���� count�� 0�̶��
  {
    Start_Floor=5;// ������ :5��
    GPIOG->ODR |= 1<<Start_Floor;// �������� LED�� Ų��
    LCD_DisplayChar(5,6,0x30+Start_Floor); // ���� Start_Floor->Final_Floor ���
    count=1;// count�� 1
  }
  else if(count==1)// �ƴϸ� count�� 1�̶��
  {
    swap=Final_Floor;// �켱 ������ ����� �������� �����ϰ�
    Final_Floor=5;// ���� ���� 6��
    GPIOG->ODR |= 1<<Final_Floor;// LED�� ������ ���� �ش��ϴ� ��ȣ�� Ų��
    if(Start_Floor!=Final_Floor)// ���� �������� �������� ���� �ʴٸ�?
    {
      LCD_DisplayChar(5,9,0x30+Final_Floor); // ���� Start_Floor->Final_Floor ���
      count=0;//count�� 0
      b=1;// b=1�� �����Ѵ�.(b=1�̸� SW7���� ������ �����Ͽ���.
      
    }
    else// ���� ���ٸ�?(��ȭ�� ������Ѵ�.)
    {
      Final_Floor=swap;// ����Ǿ��� ���������� �ٽ� �ҷ��´�.
      count=1; // �״�� count�� 1
    }
  }
}
void Sixed_Floor(void)
{
  GPIOG->ODR &= 0x0001;//����ġ 0���� �����ϰ� ��� OFF
  if(count==0)// ���� count�� 0�̶��
  {
    Start_Floor=6;// ������ :6��
    GPIOG->ODR |= 1<<Start_Floor;// �������� LED�� Ų��
    LCD_DisplayChar(5,6,0x30+Start_Floor);  // ���� Start_Floor->Final_Floor ���
    count=1;// count�� 1
  }
  else if(count==1)// �ƴϸ� count�� 1�̶��
  {
    swap=Final_Floor;// �켱 ������ ����� �������� �����ϰ�
    Final_Floor=6;// ���� ���� 6��
    GPIOG->ODR |= 1<<Final_Floor;// LED�� ������ ���� �ش��ϴ� ��ȣ�� Ų��
    if(Start_Floor!=Final_Floor)// ���� �������� �������� ���� �ʴٸ�?
    {
      LCD_DisplayChar(5,9,0x30+Final_Floor);  // ���� Start_Floor->Final_Floor ���
      count=0;//count�� 0
      b=1;// b=1�� �����Ѵ�.(b=1�̸� SW7���� ������ �����Ͽ���.
      
    }
    else// ���� ���ٸ�?(��ȭ�� ������Ѵ�.)
    {
      Final_Floor=swap;// ����Ǿ��� ���������� �ٽ� �ҷ��´�.
      count=1; // �״�� count�� 1
    }
  }
}
void Execute(void)//������
{
  DelayUS(1000);
  BEEP();
  LCD_SetTextColor(RGB_BLUE);// ���ڻ� : �Ķ���
  LCD_DisplayText(0,16,"EX");  // EX��� ���
  
  GPIOG->ODR &= 0x00FE;//LED 0���� OFF�ϰ� ������ LED�� ������ ���� 
  GPIOG->ODR |= 0x0080;//LED 7�� ON
  
  if(abs(Start_Floor-One_Elevator)<=abs(Start_Floor-Two_Elevator)) // Blue ���������� ���� ����   : �������� ���� ������������ ���̺��� �������� ������ ������������ ���� ���̰� ũ�ų� ������
  {
    if(One_Elevator<Start_Floor) //�������� �������� ������ (UP)
    {
      LCD_SetTextColor(RGB_BLUE);// ���ڻ� :  �Ķ���
      LCD_DisplayText(4,6,"L:U");  // L:U��� (Left : Up)
      LCD_SetBrushColor(RGB_BLUE);// �귯���� : �Ķ���
      LCD_DrawFillRect(15,32,10,72); // �簢�� ����
      LCD_SetBrushColor(RGB_YELLOW);// �귯���� : �����(����� ����: ������ ������)
      LCD_DrawFillRect(15,32,10,72-(One_Elevator*12));// (����� ����: ������ ������)
      for(;One_Elevator<=Start_Floor; One_Elevator++)//���� ���������Ͱ� ���������� ������ �� ���� ������ ����
      {
        Hold();//Hold�Լ� ���Ȯ��
        LCD_SetBrushColor(RGB_BLUE);// �귯���� : �Ķ���
        LCD_DrawFillRect(15,32,10,72); // �簢�� ����
        LCD_SetBrushColor(RGB_YELLOW);// �귯���� : �����
        LCD_DrawFillRect(15,32,10,72-(One_Elevator*12)); // �簢�� ����
        DelayMS(500);// ������ 0.5��
      }
      LCD_SetBrushColor(RGB_BLUE);// �귯���� : �Ķ���
      One_Elevator=Start_Floor;//�������� ���� ���������� ���� ����.
      LCD_DisplayText(4,6,"L:S");  // L:S ��� (Left : Stop)
      DelayMS(500);// ������ 0.5��
    }
    
    else if(One_Elevator>Start_Floor)//������ ���������� ������ (DOWN)
    {
      LCD_SetTextColor(RGB_BLUE);// ���ڻ� :  �Ķ���
      LCD_DisplayText(4,6,"L:D");   // L:D��� (Left : Douw)
      LCD_SetBrushColor(RGB_BLUE);// �귯���� : �Ķ���
      LCD_DrawFillRect(15,32,10,72); // �簢�� ����
      LCD_SetBrushColor(RGB_YELLOW);// �귯���� : �����
      LCD_DrawFillRect(15,32,10,72-(One_Elevator*12)); // (����� ����: ������ ������)
      for(;One_Elevator>=Start_Floor; One_Elevator--)//���� ���������Ͱ� ���������� ������ �� ���� ������ ����
      {
        Hold();//Hold�Լ� ���Ȯ��
        LCD_SetBrushColor(RGB_BLUE);// �귯���� : �Ķ���
        LCD_DrawFillRect(15,32,10,72); // �簢�� ����
        LCD_SetBrushColor(RGB_YELLOW);// �귯���� : �����
        LCD_DrawFillRect(15,32,10,72-(One_Elevator*12)); // (����� ����: ������ ������)
        DelayMS(500);//������ 0.5��
      }
      LCD_SetBrushColor(RGB_BLUE);// �귯���� : �Ķ���
      One_Elevator=Start_Floor;//�������� ���� ���������� ���� ����.
      LCD_DisplayText(4,6,"L:S"); // L:S ��� (Left : Stop)
      DelayMS(500);//������ 0.5��
    }
    else//���� ���ٸ�(��ȭ�� ����.)
    {
      LCD_SetBrushColor(RGB_BLUE);// ���ڻ� :  �Ķ���  
    }
    
    
    
    if(Start_Floor<Final_Floor)// �� ���� ���� ������ ���� ��(UP)
    {
      LCD_SetTextColor(RGB_BLUE);// ���ڻ� :  �Ķ���
      LCD_DisplayText(4,6,"L:U");  // L:U��� (Left : Up)
      LCD_SetBrushColor(RGB_BLUE);// �귯���� : �Ķ���
      LCD_DrawFillRect(15,32,10,72); // �簢�� ����
      LCD_SetBrushColor(RGB_YELLOW);// �귯���� : �����
      LCD_DrawFillRect(15,32,10,72-(Start_Floor*12));//(����� ����: ������ ������)
      for(;Start_Floor<=Final_Floor; Start_Floor++)//�������� ������������ �� ���� ������Ų��
      {
        Hold();//Hold�Լ� ���Ȯ��
        
        LCD_SetBrushColor(RGB_BLUE);// �귯���� : �Ķ���
        LCD_DrawFillRect(15,32,10,72); // �簢�� ����
        LCD_SetBrushColor(RGB_YELLOW);// �귯���� : �����
        LCD_DrawFillRect(15,32,10,72-(Start_Floor*12)); //(����� ����: ������ ������)
        DelayMS(500);//������ 0.5��
      }
    }
    else if(Start_Floor>Final_Floor)// �� ���� ���� ������ ������(DOWN)
    {
      LCD_SetTextColor(RGB_BLUE);// ���ڻ� : �Ķ���
      LCD_DisplayText(4,6,"L:D");  // L:D��� (Left : Down)
      LCD_SetBrushColor(RGB_BLUE);// �귯���� : �Ķ���
      LCD_DrawFillRect(15,32,10,72); // �簢�� ����
      LCD_SetBrushColor(RGB_YELLOW);// �귯���� : �����
      LCD_DrawFillRect(15,32,10,72-(Start_Floor*12)); // (����� ����: ������ ������)
      for(;Start_Floor>=Final_Floor; Start_Floor--)//�������� ������������ �� ���� ������Ų��
      {         
        Hold();//Hold �Լ� ��� Ȯ��
        LCD_SetBrushColor(RGB_BLUE);//�귯�� �� : �Ķ���
        LCD_DrawFillRect(15,32,10,72); // �簢�� ����
        LCD_SetBrushColor(RGB_YELLOW);//�귯�� �� : �����
        LCD_DrawFillRect(15,32,10,72-(Start_Floor*12)); //����� ����: ������ ������)
        DelayMS(500);//������ 0.5��
      }
    }
    One_Elevator=Final_Floor;// ���� ���������ʹ� �������̴�.
    LCD_SetTextColor(RGB_BLUE);// ���ڻ� : �Ķ���
    LCD_DisplayText(0,16,"FL");  // FL���
    LCD_DisplayText(4,6,"L:S");  //  L:S��� (Left : Stop)
    LCD_SetTextColor(RGB_BLACK);// ���ڻ� : 
    GPIOG->ODR &= 0x0000;//LED���� OFF
    GPIOG->ODR |= 0x0001;//LED0 ON
    BEEP3();// ������ Beep�� 3�� ���� ���
  }
  
  
  else if(abs(Start_Floor-One_Elevator)>abs(Start_Floor-Two_Elevator))// RED ���������� ���� ����   : �������� ���� ������������ ���̺��� �������� ������ ������������ ���� ���̰� ���� ��
  {
    if(Two_Elevator<Start_Floor) //�������� �������� ������ (UP)
    {
      LCD_SetTextColor(RGB_RED);// ���ڻ� :  ������
      LCD_DisplayText(4,6,"R:U");  // R:U��� (Right : Up)
      LCD_SetBrushColor(RGB_RED);//�귯�� �� : ������
      LCD_DrawFillRect(105,32,10,72); // �簢�� ����
      LCD_SetBrushColor(RGB_YELLOW);//�귯�� �� : �����
      LCD_DrawFillRect(105,32,10,72-(Two_Elevator*12)); //(����� ����: ������ ������)
      for(;Two_Elevator<=Start_Floor; Two_Elevator++)//������ ���������Ͱ� ���������� ������ �� ���� ������ ����
      {
        Hold();//Hold�Լ� ���
        LCD_SetBrushColor(RGB_RED);// �귯�� �� : ������
        LCD_DrawFillRect(105,32,10,72); // �簢�� ����
        LCD_SetBrushColor(RGB_YELLOW);//�귯�� �� : �����
        LCD_DrawFillRect(105,32,10,72-(Two_Elevator*12)); //(����� ����: ������ ������)
        DelayMS(500);//������ 0.5��
      }
      LCD_SetTextColor(RGB_RED);// ���ڻ� : ������/////////////////
      LCD_SetBrushColor(RGB_RED);//�귯�� �� : ������
      Two_Elevator=Start_Floor;// ������ ���������ʹ� �������� ��ġ
      LCD_DisplayText(4,6,"R:S");  // R:S��� (Right : Stop)
      DelayMS(500);// ������ 0.5��
    }
    else if(Two_Elevator>Start_Floor)//������ ���������� ������ (DOWN)
    {
      LCD_SetTextColor(RGB_RED);// ���ڻ� : ������
      LCD_DisplayText(4,6,"R:D"); // R:D��� (Right : Down)
      LCD_SetBrushColor(RGB_RED);//�귯�� �� : ������
      LCD_DrawFillRect(105,32,10,72); // �簢�� ����
      LCD_SetBrushColor(RGB_YELLOW);//�귯�� �� : �����
      LCD_DrawFillRect(105,32,10,72-(Two_Elevator*12)); //(����� ����: ������ ������)
      for(;Two_Elevator>=Start_Floor; Two_Elevator--)//���� �� ���������Ͱ� ���������� ������ �� ���� ������ ����
      {
        
        Hold();//Hold�Լ� ��� ���� Ȯ��
        LCD_SetBrushColor(RGB_RED);//�귯�� �� : ������
        LCD_DrawFillRect(105,32,10,72); // �簢�� ����
        LCD_SetBrushColor(RGB_YELLOW);//�귯�� �� : �����
        LCD_DrawFillRect(105,32,10,72-(Two_Elevator*12)); //(����� ����: ������ ������)
        DelayMS(500);// ������ 0.5��
      }
      LCD_SetBrushColor(RGB_RED);//�귯�� �� : ������
      Two_Elevator=Start_Floor;//������ ���������Ͱ� �������� ����
      LCD_SetTextColor(RGB_RED);// ���ڻ� : ������/////////////////////////////
      LCD_DisplayText(4,6,"R:S");  // R:S��� (Right : Stop)
      DelayMS(500);// ������ 0.5��
    }
    else//���� ���ٸ�(��ȭ�� ����.)
    {
      LCD_SetTextColor(RGB_RED);// ���ڻ� : ������
    }
    
    
    if(Start_Floor<Final_Floor)// �� ���� ���� ������ ���� ��(UP)
    {
      LCD_SetTextColor(RGB_RED);// ���ڻ� : ������
      LCD_DisplayText(4,6,"R:U");  // R:U��� (Right : Up)
      LCD_SetBrushColor(RGB_RED);//�귯�� �� : ������
      LCD_DrawFillRect(105,32,10,72); // �簢�� ����
      LCD_SetBrushColor(RGB_YELLOW);//�귯�� �� : �����
      LCD_DrawFillRect(105,32,10,72-(Start_Floor*12));  //(����� ����: ������ ������)
      for(;Start_Floor<=Final_Floor; Start_Floor++)//�������� �������� ���� �� ���� ������Ų��
      {
        Hold();//Hold �Լ� Ȯ��
        LCD_SetBrushColor(RGB_RED);// �귯���� : ������
        LCD_DrawFillRect(105,32,10,72); // �簢�� ����
        LCD_SetBrushColor(RGB_YELLOW);// �귯���� : �����
        LCD_DrawFillRect(105,32,10,72-(Start_Floor*12)); //(����� ����: ������ ������)
        DelayMS(500);// ������ 0.5��
      }
    }
    else if(Start_Floor>Final_Floor)// �� ���� ���� ������ ������(DOWN)
    {
      LCD_SetTextColor(RGB_RED);// ���ڻ� : ������
      LCD_DisplayText(4,6,"R:D");  // R:D��� (Right : Down)
      LCD_SetBrushColor(RGB_RED);//�귯���� : ������
      LCD_DrawFillRect(105,32,10,72); // �簢�� ����
      LCD_SetBrushColor(RGB_YELLOW);//�귯���� : �����
      LCD_DrawFillRect(105,32,10,72-(Start_Floor*12)); //(����� ����: ������ ������)
      for(;Start_Floor>=Final_Floor; Start_Floor--)//�������� �������� ���� �� ���� ���ҽ�Ų��
      {
        Hold();//Hold �Լ� Ȯ��
        LCD_SetBrushColor(RGB_RED);//�귯���� : ������
        LCD_DrawFillRect(105,32,10,72); // �簢�� ����
        LCD_SetBrushColor(RGB_YELLOW);//�귯���� : �����
        LCD_DrawFillRect(105,32,10,72-(Start_Floor*12)); //(����� ����: ������ ������)
        DelayMS(500);//������ 0.5��
      }
    }
    Two_Elevator=Final_Floor;// �ι�° ���������Ͱ� ������ ����
    LCD_SetTextColor(RGB_BLUE);// ���ڻ� : �Ķ���
    LCD_DisplayText(0,16,"FL");   // FL���
    LCD_SetTextColor(RGB_RED);// ���ڻ� : ������
    LCD_DisplayText(4,6,"R:S");  // R:S��� (Right : Stop)
    LCD_SetTextColor(RGB_BLACK);// ���ڻ� : ������
    GPIOG->ODR &= 0x0000;//LED ��� OFF
    GPIOG->ODR |= 0x0001;//LED 0�� ���
    BEEP3();// ������ Beep�� 3�� ���� ���
  }
  
}
void Time_Delay(void)
{
  DelayUS(1000);
  BEEP();    
  LCD_SetTextColor(RGB_BLUE);// ���ڻ� :  �Ķ���
  LCD_DisplayText(0,16,"HL");  // HL���
  GPIOG->ODR &= 0x007E;// LED 0�� 7�� OFF ������ ���� ���� ����
  for(int i=0; i<10; i++)
  {
    DelayMS(500);//0.5�ʾ� �����̸� 10��
    BEEP();//������ �ѹ��� ���
  }
  LCD_SetTextColor(RGB_BLUE);// ���ڻ� :  �Ķ���
  LCD_DisplayText(0,16,"EX"); // EX���
  GPIOG->ODR |= 0x0080;//LED 7�� ON
  
}
void Hold(void)
{        
  if(SW0_Flag)// 0�� ����ġ�� ������?
  {
    
    Time_Delay();//Time_Delay�Լ� ���
    SW0_Flag=0;//SW0�� �÷��׺����� 0�̴�.        
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
// ���� �����ڷ�
// ������ stm32f4xx.h�� �ִ� RCC���� �ֿ� ������ 
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