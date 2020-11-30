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
uint8_t	SW0_Flag, SW1_Flag,SW5_Flag,SW6_Flag, SW7_Flag,SW0_Flag1,SW1_Flag1,SW2_Flag1,SW3_Flag1,SW4_Flag1; // �տ� �÷��� ������ ���ͷ�Ʈ ���� �÷��� ����, �÷��� �ڿ� 1�� �������� ������ġ ���� �÷��� ����.
uint8_t count=0; // (ī��Ʈ = 0 : ó�� ����) (ī��Ʈ = 1 : ������ ���� )  (ī��Ʈ =2 : �Ķ��� ����)
int u=5,t=5, x=5,y=5,RedplayerW=0, BlueplayerW=0,Turn=0; //u,t�� �������� (x,y)��ǥ�̸� x,y�� �Ķ����� (x,y)��ǥ�̴�.
int i[10][10] = {0}; // �ߺ� ���� �迭

int main(void)
{
  _GPIO_Init(); // GPIO (LED & SW) �ʱ�ȭ
  LCD_Init();	// LCD ��� �ʱ�ȭ
  _EXTI_Init();  //���ͷ�Ʈ
  UART5_Init();
  Fram_Init();        // FRAM �ʱ�ȭ  //�ݵ�� �����;���
  Fram_Status_Config(); // Fram_Satus_config();�ݵ�� �����;���
  DelayMS(100);
  
  DisplayInitScreen();	// LCD �ʱ�ȭ��
  // BEEP();
  GPIOG->ODR = 0xFF00;	// LED �ʱⰪ: LED0~7 Off
  //�츮 Fram�� ����� 0���� �ʱ�ȭ �Ǿ�����
  //Fram_Write(�ּ�(0~8191),������(1Byte)
  //Fram_Read (�ּ�) -  Return(1Byte)
  //LCD_DisplayChar(1,0,Fram_Read(100)+0x30); //FRAM 100���� ����� data(1byte) �о� 
  
  while(1)
  { 
    if(SW0_Flag) // ����ġ 0���� ������ ��
    {
      if(Turn==0 && (count==0  || count==2))  //ó�� �����ϰų� �������� ������ ���� ��
      {
        GPIOG->ODR = 0xFF00;	// LED �ʱⰪ: LED0~7 Off
        GPIOG->ODR |= 0x0001; // LED 0 ON
        LCD_SetTextColor(RGB_RED);// ���ڻ� : Red
        LCD_DrawText(2,116,"*"); //������
        LCD_DrawText(145,116," "); //������
        LCD_SetBrushColor(RGB_RED);
        Serial_PutString("1");
        Turn=1; //���� ���� ����
        count=1;//���� ���� ����
      }
      SW0_Flag = 0;
      SW1_Flag = 0; // 1���� 6���� 0���� ��� ���ִ� ������ �߰��� ������ ���� �����ϱ� �����̴�.
      SW6_Flag = 0;
    }
    else if(SW7_Flag)
    {
      if(Turn==0 && (count==0  || count==1)) //ó�� �����ϰų� �Ķ����� ������ ���� ��
      {
        GPIOG->ODR = 0xFF00;	// LED �ʱⰪ: LED0~7 Off
        GPIOG->ODR |= 0x0080; // LED 7 ON
        LCD_SetTextColor(RGB_BLUE);// ���ڻ� : BLUE
        LCD_DrawText(145,116,"*"); //�Ķ���
        LCD_DrawText(2,116," "); //�Ķ���
        LCD_SetBrushColor(RGB_BLUE);
        Serial_PutString("2");
        Turn=3;// �Ķ����� ����
        count=2;// �Ķ����� ����
      }
      SW7_Flag = 0;
      SW1_Flag = 0; // 1���� 6���� 0���� ��� ���ִ� ������ �߰��� ������ ���� �����ϱ� �����̴�.
      SW6_Flag = 0;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////�Ķ��� ������ ���� ���� ///////////////////////////////////////////////////////////////////////
    
    joystic(); // ���̽�ƽ ����
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    if(SW1_Flag && Turn == 0 && count==1) // �������� ��⿡�� ����ϰ� ����ġ 1���� ��������
    {
      LCD_SetTextColor(RGB_RED);
      RedplayerW++; // �̱�Ƚ�� ����
      if(RedplayerW>9) // ���� �̱�Ƚ���� 9���� Ŀ���� �Ǹ�?
      {
        RedplayerW=0; //0���� �ʱ�ȭ
      }
      Turn=0; // �ʱ���� �ǵ���
      count=0; // �ʱ���� �ǵ���
      Fram_Write(300,RedplayerW);  // �������� 301������ ����
      RedplayerW=Fram_Read(300);// 301���� ���� �о�´�.
      LCD_DrawChar(58,116,RedplayerW+0x30); //������
      DelayMS(500); //5�� ���
      u=y=x=t=5; // �ʱ� ��ǥ���� ������ �Ķ��� ��� 5,5�� �ʱ�ȭ
      DisplayInitScreen(); // �ʱ�ȭ��
      for(int j=0; j<10; j++)
        for(int k=0; k<10; k++)
          i[j][k] = 0; // �ߺ� ���� �迭 �ʱ�ȭ
      SW1_Flag = 0; 
      
    }
    
    else if(SW6_Flag && Turn == 0 && count==2)  //�Ķ����� ��⿡�� ����ϰ� ����ġ 6���� ������ ��
    {
      LCD_SetTextColor(RGB_BLUE);// ���ڻ� : BLUE
      BlueplayerW++;// �̱�Ƚ�� ����
      if(BlueplayerW>9)// ���� �̱�Ƚ���� 9���� Ŀ���� �Ǹ�?
      {
        BlueplayerW=0;//0���� �ʱ�ȭ
      }
      
      Turn=0; // �ʱ���� �ǵ���
      count=0; // �ʱ���� �ǵ���
      Fram_Write(301,BlueplayerW); // �������� 301������ ����
      BlueplayerW=Fram_Read(301);// 301���� ���� �о�´�.
      LCD_DrawChar(88,116,BlueplayerW+0x30); //������
      DelayMS(500);
      u=y=x=t=5; // �ʱ� ��ǥ���� ������ �Ķ��� ��� 5,5�� �ʱ�ȭ
      DisplayInitScreen();// �ʱ�ȭ��
      for(int j=0; j<10; j++)
        for(int k=0; k<10; k++)
          i[j][k] = 0; // �ߺ� ���� �迭 �ʱ�ȭ
      SW6_Flag = 0;
    }
  }
}



void _EXTI_Init(void)
{
  
  RCC->AHB1ENR 	|= 0x0080;	// RCC_AHB1ENR GPIOH Enable
  RCC->APB2ENR 	|= 0x4000;	// Enable System Configuration Controller Clock
  GPIOH->MODER 	&= 0x0000FFFF;	// GPIOH PIN8~PIN15 Input mode (reset state)				 
  
  SYSCFG->EXTICR[2] |= 0x0077; 	// EXTI8�� ���� �ҽ� �Է��� GPIOH�� ���� (EXTICR3) (reset value: 0x0000)//PH8->EXTI8�� �ٲ��ִ� ����
  EXTI->FTSR |= 0x0300;		// Falling Trigger Enable  (EXTI8:PH8)//�������� ����
  EXTI->IMR  |= 0x0300;  	// EXTI8,9 ���ͷ�Ʈ mask (Interrupt Enable) //IMR �� ����Ѵ�.
  NVIC->ISER[0] |= ( 1<<23  );   // Enable Interrupt EXTI8,9 Vector table Position ���� //ISER[0] ~ [7]���� �ִ�. [0]�� (0������ 31������ ����Ѵ�. 32�� [1]�� �ִ�.) 1������ 32bit
  //NVIC->ISER[0] |= ( 0x00800000  );
  
  SYSCFG->EXTICR[3] |= 0x7770; 	//EXIT15,14�� ���� �ҽ� �Է��� GPIOH�� ����// PH15->EXIT15	
  EXTI->FTSR |= 0xE000;		// Falling Trigger Enable  (15,14,13��)//�������� ����
  EXTI->IMR  |= 0xE000;  	// EXTI15,14 ���ͷ�Ʈ mask (Interrupt Enable) //IMR �� ����Ѵ�.// 0xE000 =0b 1110 0000 0000 0000
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
  GPIOH->MODER	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
  GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state
  
  // Buzzer (GPIO F) ���� 
  RCC->AHB1ENR    |=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
  GPIOF->MODER    |=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
  GPIOF->OTYPER 	&= ~0x00000200;	// GPIOF 9 : Push-pull  	
  GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
  
  //Joy Stick SW(PORT I) ����
  RCC->AHB1ENR 	|= 0x0100;	// RCC_AHB1ENR GPIOI Enable
  GPIOI->MODER   &= ~0x000FFC00;	// GPIOI 5~9 : Input mode (reset state)
  GPIOI->PUPDR    &= ~0x000FFC00;	// GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)
  
}	
void EXTI9_5_IRQHandler(void)		// EXTI 5~9 ���ͷ�Ʈ �ڵ鷯 //��� �̹� ������־ �׳� ���� �ȴ�.
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
void EXTI15_10_IRQHandler(void)		// EXTI 15~10 ���ͷ�Ʈ �ڵ鷯 //��� �̹� ������־ �׳� ���� �ȴ�.
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
void DisplayInitScreen(void) // �ʱ����
{
  GPIOG->ODR = 0xFF00;	// LED �ʱⰪ: LED0~7 Off
  LCD_Clear(RGB_YELLOW);	// ȭ�� Ŭ����
  LCD_SetFont(&Gulim8);	// ��Ʈ : ���� 8
  
  LCD_SetBackColor(RGB_YELLOW);// ���ڹ��� : Green
  LCD_SetTextColor(RGB_BLACK);// ���ڻ� : Black
  LCD_DisplayText(0,1,"Mecha-OMOK");  // Title
  LCD_SetPenColor(RGB_BLACK);
  
  RedplayerW=Fram_Read(300); //������ ���Ƚ���� 300������ ����
  BlueplayerW=Fram_Read(301);//�Ķ��� ���Ƚ���� 301������ ����
  for(int i=1; i<=10; i++)//10X10�� ��
  {
    LCD_DrawHorLine(35,19+i*9,81); // ���μ� 10�� : ���� 9
    LCD_DrawVerLine(26+i*9,28,81); // ���μ� 10�� : ���� 9
  }
  LCD_DrawText(20,15,"0");
  LCD_DrawText(20,65,"5");
  LCD_DrawText(20,105,"9");
  LCD_DrawText(75,15,"5");
  LCD_DrawText(115,15,"9"); // ĭ�� ǥ��
  LCD_DrawText(70,116,"VS");
  
  LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Blue
  LCD_DrawText(105,116,"( , )");//�Ķ���
  Fram_Write(301,BlueplayerW); // 301������ �Ķ� ���� ���Ƚ���� �о�´�
  LCD_DrawChar(88,116,Fram_Read(301)+0x30); //�Ķ���
  LCD_DrawChar(111,116,0x30+u);//�Ķ���
  LCD_DrawChar(128,116,0x30+t);//�Ķ���
  LCD_DrawText(145,116," "); //������
  
  LCD_SetTextColor(RGB_RED);// ���ڻ� : Red
  LCD_DrawText(12,116,"( , )"); //������
  Fram_Write(300,RedplayerW);
  LCD_DrawChar(58,116,Fram_Read(300)+0x30); //������
  LCD_DrawChar(18,116,0x30+u);//������
  LCD_DrawChar(36,116,0x30+t);//������
  LCD_DrawText(2,116," "); //������
  LCD_SetBrushColor(RGB_BLACK);//������
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


void joystic(void) // ���� ����ġ �Լ�
{
  switch(JOY_Scan())
  {
  case NAVI_LEFT: 	// NAVI_LEFT
    Serial_PutString("l"); 
    if(Turn==1) // ���������� ���� ��
    {
      
      LCD_SetTextColor(RGB_RED);// ���ڻ� : Red
      LCD_SetBrushColor(RGB_RED);
      
      if(u<=0) // ���� �������� x�� ���� 0���� �۴ٸ�?
      {     
        u=0;
        LCD_DrawChar(18,116,0x30+u);//������
      }
      else //�׷��� �ʴٸ�?
      {      
        u--; // ��ǥ���� 1��ŭ ���ָ� �ȴ�.
        BEEP();
        LCD_DrawChar(18,116,0x30+u);//������   
      }   
    }
    
    else if(Turn==3) //�Ķ������� ���� ��
    {
      LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Blue
      LCD_SetBrushColor(RGB_BLUE);
      
      if(x<=0)// ���� �Ķ����� x�� ���� 0���� �۴ٸ�?
      {     
        x=0;
        LCD_DrawChar(111,116,0x30+x);//�Ķ���
      }
      else //�׷��� �ʴٸ�?
      {     
        BEEP();
        x--;// ��ǥ���� 1��ŭ ���ָ� �ȴ�.
        LCD_DrawChar(111,116,0x30+x);//�Ķ���
      }
      
    }
    
    break;
    
  case NAVI_RIGHT: 	// NAVI_RIGHT
    Serial_PutString("r");
    if(Turn==1) // ���������� ���� ��
    {
      LCD_SetTextColor(RGB_RED);// ���ڻ� : Red
      LCD_SetBrushColor(RGB_RED);
      if(u>=9)// ���� �������� x�� ���� 9���� ũ�ٸ�?
      {     
        u=9;
        LCD_DrawChar(18,116,0x30+u);//������
      }
      else //�׷��� �ʴٸ�?
      {     
        BEEP();
        u++;// ��ǥ���� 1��ŭ �����ָ� �ȴ�.
        LCD_DrawChar(18,116,0x30+u);//������
      }
    }
    else if(Turn==3)
    {
      LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Blue
      LCD_SetBrushColor(RGB_BLUE);
      if(x>=9)// ���� �Ķ����� x�� ���� 9���� ũ�ٸ�?
      {     
        x=9;
        LCD_DrawChar(111,116,0x30+x);//�Ķ���
        
      }
      else //�׷��� �ʴٸ�?
      {     
        BEEP();
        x++;// ��ǥ���� 1��ŭ �����ָ� �ȴ�.
        LCD_DrawChar(111,116,0x30+x);//�Ķ���
      }
      
    }
    break;
    
  case NAVI_UP: 	// NAVI_UP
    Serial_PutString("u");
    if(Turn==1) // ���������� ���� ��
    {
      LCD_SetTextColor(RGB_RED);// ���ڻ� : Red
      LCD_SetBrushColor(RGB_RED);
      if(t<=0)// ���� �������� y�� ���� 0���� �۴ٸ�?
      {
        t=0; 
        LCD_DrawChar(36,116,0x30+t); //������
      }
      else//�׷��� �ʴٸ�? 
      {     
        BEEP();
        t--;// ��ǥ���� 1��ŭ ���ָ� �ȴ�.
        LCD_DrawChar(36,116,0x30+t); //������
      }
    }
    else if(Turn==3)
    {
      LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Blue
      LCD_SetBrushColor(RGB_BLUE);
      if(y<=0)// ���� �Ķ����� y�� ���� 0���� �۴ٸ�?
      {     
        y=0;
        LCD_DrawChar(128,116,0x30+y);//�Ķ���
      }
      else 
      {     
        BEEP();
        y--;// ��ǥ���� 1��ŭ ���ָ� �ȴ�.
        LCD_DrawChar(128,116,0x30+y);//�Ķ���
      }
      
    }
    
    break;
    
  case NAVI_DOWN: 	// NAVI_UP
    Serial_PutString("d");
    if(Turn==1) // ���������� ���� ��
    {                        
      LCD_SetTextColor(RGB_RED);// ���ڻ� : Red
      LCD_SetBrushColor(RGB_RED);
      if(t>=9)// ���� �������� y�� ���� 9���� ũ�ٸ�?
      { 
        t=9;
        LCD_DrawChar(36,116,0x30+t); //������
      }
      else //�׷��� �ʴٸ�?
      {     
        BEEP();
        t++;// ��ǥ���� 1��ŭ �����ָ� �ȴ�. 
        LCD_DrawChar(36,116,0x30+t); //������
      }
    }
    else if(Turn==3)
    {
      LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Blue
      LCD_SetBrushColor(RGB_BLUE);
      if(y>=9)// ���� �Ķ����� y�� ���� 9���� ũ�ٸ�?
      {     
        y=9;
        LCD_DrawChar(128,116,0x30+y);//�Ķ���
      }
      else 
      {     
        BEEP();
        y++;/// ��ǥ���� 1��ŭ �����ָ� �ȴ�.
        LCD_DrawChar(128,116,0x30+y);//�Ķ���
      }
    }
    
    break;
    
  case NAVI_PUSH:
    Serial_PutString("p");
    if(Turn==1) //���� ���̿��ٸ�?
    {
      if(i[u][t]==1) //�ߺ� �� �̶��?
      {
        Turn=1; // ������(���� ���� ����) ����
        count=1;
      }
      
      else
      {
        BEEP();
        DelayMS(100);
        BEEP();
        DelayMS(100);
        BEEP();///////////////////////////////������ ���� �︮��//////////////////////////////
        LCD_DrawFillRect(32+u*9,21+t*9,9,9);//������
        i[u][t]=1; // ���̴� ���� �ߺ��� �����ϱ� ���� ���
        Turn=0; // 
        count=1;
      }      
    }
    
    else if(Turn==3) // �Ķ� �� �̿��ٸ� ?
    {
      if(i[x][y]==1) //������(���� ���� ����) ����
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
        BEEP();///////////////////////////////������ ���� �︮��//////////////////////////////
        LCD_DrawFillRect(32+x*9,21+y*9,9,9);//�Ķ���
        i[x][y]=1;// ���ε� ����
        Turn=0; //�ٽ� �� ����
        count=2; // �Ķ����� ����
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
  // UART5->CR1 	|= USART_CR1_TXEIE;	//  0x0020, TXEIE interrupt Enable              // �̰� 1�̸� �ܺ� ���ͷ�Ʈ �ٷι߻� ����? 
  
  NVIC->ISER[1]	|= (1 << (53-32)); 	// Enable Interrupt UART5 (NVIC 37��)
  UART5->CR1 	|= USART_CR1_UE;	//  0x2000, UART5 Enable
}

void UART5_IRQHandler(void)
{       
  
  if ( (UART5->SR & USART_SR_RXNE) )
  {
    uint16_t ch;
    
    ch = (uint16_t)(UART5->DR & (uint16_t)0x01FF);	// ���ŵ� ���� ����  0000000111111111
    //SerialPutChar(ch);     // �����ֱ� ����                                            0000000000110001
    if(ch == 'l') // 0x6C = l
    {
      //LCD_SetTextColor(RGB_BLUE);
      if(Turn==1) // ���������� ���� ��
      {
        LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Red
        LCD_SetBrushColor(RGB_BLUE);
        if(u<=0) // ���� �������� x�� ���� 0���� �۴ٸ�?
        {     
          u=0;
          LCD_DrawChar(111,116,0x30+u);//�Ķ���
        }
        else //�׷��� �ʴٸ�?
        {     
          u--; // ��ǥ���� 1��ŭ ���ָ� �ȴ�.
          BEEP();
          LCD_DrawChar(111,116,0x30+u);//�Ķ���
        }
      }
      else if(Turn==3) //�Ķ������� ���� ��
      {
        LCD_SetTextColor(RGB_RED);// ���ڻ� : Blue
        LCD_SetBrushColor(RGB_RED);
        if(x<=0)// ���� �Ķ����� x�� ���� 0���� �۴ٸ�?
        {     
          x=0;
          LCD_DrawChar(18,116,0x30+x);//������
        }
        else //�׷��� �ʴٸ�?
        {     
          BEEP();
          x--;// ��ǥ���� 1��ŭ ���ָ� �ȴ�.
          LCD_DrawChar(18,116,0x30+x);//������
        }
      }
    }
    
    
    
    if(ch == 'r')  // 0x72 ='r'
    {
      //LCD_SetTextColor(RGB_BLUE);
      if(Turn==1) // ���������� ���� ��
      {
        LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Red
        LCD_SetBrushColor(RGB_BLUE);
        if(u>=9) 
        {     
          u=9;
          LCD_DrawChar(111,116,0x30+u);
        }
        else //�׷��� �ʴٸ�?
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
        else //�׷��� �ʴٸ�?
        {     
          BEEP();
          x++;
          LCD_DrawChar(18,116,0x30+x);
        }
      }
    }
    
    
    if(ch == 'u')  // 0x75 = 'u'
    {
      if(Turn==1) // ���������� ���� ��
      {                        
        LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Red
        LCD_SetBrushColor(RGB_BLUE);
        if(t<=0)// ���� �������� y�� ���� 9���� ũ�ٸ�?
        { 
          t=0;
          LCD_DrawChar(128,116,0x30+t); //������
        }
        else //�׷��� �ʴٸ�?
        {     
          BEEP();
          t--;// ��ǥ���� 1��ŭ �����ָ� �ȴ�. 
          LCD_DrawChar(128,116,0x30+t); //������
        }
      }
      else if(Turn==3)
      {
        LCD_SetTextColor(RGB_RED);// ���ڻ� : Blue
        LCD_SetBrushColor(RGB_RED);
        if(y<=0)// ���� �Ķ����� y�� ���� 9���� ũ�ٸ�?
        {     
          y=0;
          LCD_DrawChar(36,116,0x30+y);//�Ķ���
        }
        else 
        {     
          BEEP();
          y--;/// ��ǥ���� 1��ŭ �����ָ� �ȴ�.
          LCD_DrawChar(36,116,0x30+y);//�Ķ���
        }
      }
    }
    if(ch == 'd') //0x64= 'd'
    {
      if(Turn==1) // ���������� ���� ��
      {
        LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Red
        LCD_SetBrushColor(RGB_BLUE);
        if(t>=9)// ���� �������� y�� ���� 0���� �۴ٸ�?
        {
          t=9; 
          LCD_DrawChar(128,116,0x30+t);//�Ķ���
        }
        else//�׷��� �ʴٸ�? 
        {     
          BEEP();
          t++;// ��ǥ���� 1��ŭ ���ָ� �ȴ�.
          LCD_DrawChar(128,116,0x30+t);//�Ķ���
        }
      }
      else if(Turn==3)
      {
        LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Blue
        LCD_SetBrushColor(RGB_BLUE);
        if(y>=9)// ���� �Ķ����� y�� ���� 0���� �۴ٸ�?
        {     
          y=9;
          LCD_DrawChar(36,116,0x30+y);//�Ķ���
        }
        else 
        {     
          BEEP();
          y++;// ��ǥ���� 1��ŭ ���ָ� �ȴ�.
          LCD_DrawChar(36,116,0x30+y);//�Ķ���
        }
        
        
      }
    }
    
    if(ch == 'p') //ox70 = 'p'
    {
      if(Turn==1) //���� ���̿��ٸ�?
      {
        if(i[u][t]==1) //�ߺ� �� �̶��?
        {
          Turn=1; // ������(���� ���� ����) ����
          count=1;
        }
        
        else
        {
          BEEP();
          DelayMS(100);
          BEEP();
          DelayMS(100);
          BEEP();///////////////////////////////������ ���� �︮��//////////////////////////////
          LCD_SetBrushColor(RGB_BLUE);
          LCD_DrawFillRect(32+u*9,21+t*9,9,9);//������
          i[u][t]=1; // ���̴� ���� �ߺ��� �����ϱ� ���� ���
          Turn=0; // 
          count=1;
        }      
      }
      
      else if(Turn==3) // �Ķ� �� �̿��ٸ� ?
      {
        if(i[x][y]==1) //������(���� ���� ����) ����
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
          BEEP();///////////////////////////////������ ���� �︮��//////////////////////////////
          LCD_SetBrushColor(RGB_RED);
          LCD_DrawFillRect(32+x*9,21+y*9,9,9);//�Ķ���
          i[x][y]=1;// ���ε� ����
          Turn=0; //�ٽ� �� ����
          count=2; // �Ķ����� ����
        }
      }
    }
    
    else if(ch == 0x31)
    {
      GPIOG->ODR = 0xFF00;	// LED �ʱⰪ: LED0~7 Off
      GPIOG->ODR |= 0x0001; // LED 0 ON
      LCD_SetTextColor(RGB_BLUE);// ���ڻ� : Red
      LCD_DrawText(2,116," "); //������
      LCD_DrawText(145,116,"*"); //������
      LCD_SetBrushColor(RGB_BLUE);
      Turn=1; //���� ���� ����
      count=1;//���� ���� ����
    }
    else if(ch == 0x32)
    {
      GPIOG->ODR = 0xFF00;	// LED �ʱⰪ: LED0~7 Off
      GPIOG->ODR |= 0x0080; // LED 0 ON
      LCD_SetTextColor(RGB_RED);// ���ڻ� : Red
      LCD_DrawText(2,116,"*"); //������
      LCD_DrawText(145,116,""); //������
      LCD_SetBrushColor(RGB_RED);
      Turn=3;// �Ķ����� ����
      count=2;// �Ķ����� ����
    }                
    
  }
  
  // DR �� ������ SR.RXNE bit(flag bit)�� clear �ȴ� 
}
void SerialPutChar(uint8_t Ch) // 1���� ������ �Լ�
{
  while((UART5->SR & USART_SR_TXE) == RESET); //  USART_SR_TXE:0x0080, �۽� ������ ���±��� ���   0 �̸� �� 
  
  UART5->DR = (Ch & 0x01FF);	// ����    0000 0001 1111 1111    0x0001   
  //         0000 0000 0000 0001    0x0031
  //           0000 0000 0000 0001
}

void Serial_PutString(char *str) // �������� ������ �Լ�
{
  while (*str != '\0') // ���Ṯ�ڰ� ������ ������ ����, ���Ṯ�ڰ� �����Ŀ��� ������ �޸� ���� �߻����ɼ� ����.
  {
    SerialPutChar(*str);	// �����Ͱ� ����Ű�� ���� �����͸� �۽�
    str++; 			// ������ ��ġ ����
  }
}

void USART_BRR_Configuration(uint32_t USART_BaudRate) //��żӵ� ����
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
