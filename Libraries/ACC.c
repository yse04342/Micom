//////////////////////////////////////////////////////////////////////////
// ���ӵ��������� SPI����� application function ����
//  SPI1_Send(uint8_t byte): 1 byte �����͸� ���ӵ������� ���� 
//  ACC_Init(): ���ӵ����� �ʱ�ȭ, CTRL1,2,4�� �ʱⰪ ����/�Է�
//  SPI1_GetData(UINT8 *pBuf) : �����κ��� ���ӵ��� X,Y,Z�� �о��, pBuf[]�� return
//  SPI1_Process(UINT16 *pBuf) : ���ӵ��� �а� 16bit ���·� ��ȯ,  pBuf[]�� return 
//  SPI1_CalData(UINT8 *pBuf, UINT16 *pTxBuf) : ���ӵ����� 16bit ���·� ��ȯ,
///////////////////////////////////////////////////////////////////////////
#define __ACC_C__
#include "ACC.h"
#undef  __ACC_C__

UINT8 SPI1_GetData(UINT8 *pBuf);
void SPI1_CalData(UINT8 *pBuf, UINT16 *pTxBuf);

#define DUMMY   0x00		// �ǹ̾��� ������, ���Ÿ��� ���� �����ϴ� ������ 

uint8_t SPI1_Send(uint8_t byte)		// 1 byte �����͸� ���ӵ������� ���� 
{
        // Polling: Loop while DR register in not empty 
        while ((SPI1->SR & (1<<1)) == RESET);		// SR.TXE = 1 ?	
        
        // Send a byte through the SPI1 peripheral(ACC sensor)
        SPI1->DR = byte;

        // Polling: Wait to receive a byte 
        while ((SPI1->SR & (1<<0)) == RESET);		// SR.RXNE = 1 ?	 // ������ �ȿ����� ��� ��ٸ�..
	
        // Return the byte read from the SPI bus (ACC sensor) 
        return(SPI1->DR);
}

/////// ACC_Init(void)	////////////////////////////////////////
// CTRL1 �ʱⰪ �Է�
// 	HR : high resolution (Value:1)
// 	ODR[2:0] : 800Hz (Value:110) Output Data Rate
// 	ZEN : Z-axis enabled (Value:1)
// 	YEN : Y-axis enabled (Value:1)
// 	XEN : X-axis enabled (Value:1)
// CTRL2 �ʱⰪ �Է�
//	DFC[1:0] : ODR 9 (Value:10) 
//	   (HighPass cutoff freqency ���:ODR���� ���õ� ���ļ�)
// CTRL4 �ʱⰪ �Է�
//	BW[2:1] : 400Hz (Value:00)  
// 	FS : +/-2g (Value:00)
// 	BW_SCALE_ODR : automatically(Value:0)
// 	IF_ADD_INC : enabled (Value:1)
//	I2C_DISABLE : Disabled (Value:1)
//	SIM : 4-wire interface (Value:0)
////////////////////////////////////////////////////////////////////
void ACC_Init(void)	
{
        SET_ACC_NSS1();         // NSS : H

	// Control Reg 1 
        CLR_ACC_NSS1();	        // NSS : L (enable: ��� ����)
        SPI1_Send(CMD_WRITE | CMD_REG_CTRL1);	// Master -> Slave(sensor)
						// writing a Command to CTRL1(20h) // 20������ �ڿ��ִ� �����͸� ���ڴ�.
        SPI1_Send(HR_HIGH_RESOLUTION		// Command
                | ODR_800HZ
                | BDU_CONTINUOUS_UPDATE
                //| ZEN_Z_ENABLE  
              //  | YEN_Y_ENABLE
                | XEN_X_ENABLE);
        SET_ACC_NSS1();         // NSS : H
        UTIL_DelayUS(1);
  
	// Control Reg 2 
        CLR_ACC_NSS1();	       // NSS : L (enable: ��� ����)
        SPI1_Send(CMD_WRITE | CMD_REG_CTRL2);	// Master -> Slave(sensor)
                                              	// writing a Command to CTRL2(21h)   
        SPI1_Send(ZERO                        	// command
                | DFC_ODR_9
		| FDS_FILTER_BYPASSED
		| HPIS1_FILTER_BYPASSED  
		| HPIS2_FILTER_BYPASSED);
        SET_ACC_NSS1();
        UTIL_DelayUS(1);

	// Control Reg 3   
        CLR_ACC_NSS1();	
        SPI1_Send(CMD_WRITE | CMD_REG_CTRL3);  
        SPI1_Send(0x00);
        SET_ACC_NSS1();
        UTIL_DelayUS(1);

	// Control Reg 4   
        CLR_ACC_NSS1();	
        SPI1_Send(CMD_WRITE | CMD_REG_CTRL4); 	// Master -> Slave(sensor)
                                             	// writing a Command to CTRL4(24h) 
        SPI1_Send(BW_400HZ 
		| FS_4G
		| BW_SCALE_ODR_AUTOMATICALLY
		| IF_ADD_INC_ENABLE  
		| I2C_DISABLE
		| SIM_4WIRE);
        SET_ACC_NSS1();
        UTIL_DelayUS(1);

	// Control Reg 5   
        CLR_ACC_NSS1();	
        SPI1_Send(CMD_WRITE | CMD_REG_CTRL5);  
        SPI1_Send(0x00);
        SET_ACC_NSS1();
        UTIL_DelayUS(1);
  
	// Control Reg 6 
        CLR_ACC_NSS1();	
        SPI1_Send(CMD_WRITE | CMD_REG_CTRL6);  
        SPI1_Send(0x00);
        SET_ACC_NSS1();
  
	// Control Reg 7 
        CLR_ACC_NSS1();	
        SPI1_Send(CMD_WRITE | CMD_REG_CTRL7);  
        SPI1_Send(0x00);
        SET_ACC_NSS1();
  
        UTIL_DelayUS(100);
}

/////////////////////////////////////////////////////
// �����κ��� ���ӵ��� X,Y,Z�� �о��, PBuf[]�� return
// X��: pBuf[1], pBuf[0]
// Y��: pBuf[3], pBuf[2]
// Z��: pBuf[5], pBuf[4]
UINT8 SPI1_GetData(UINT8 *pBuf)
{
	// Get ACC_X
        CLR_ACC_NSS1();  
        SPI1_Send(CMD_READ | CMD_REG_OUT_X_L);	// Master <- Slave(sensor) //CMD_REG_OUT_X_L= 28��
                                             	// reading a data from OUT_X
        pBuf[0] = SPI1_Send(DUMMY);		// ���ӵ� X ���� 16��Ʈ�̹Ƿ� �ΰ��� dummy�� ���� // �о���
        pBuf[1] = SPI1_Send(DUMMY);
        SET_ACC_NSS1();
        UTIL_DelayUS(1);
  
	// Get ACC_Y
        CLR_ACC_NSS1();  
        SPI1_Send(CMD_READ | CMD_REG_OUT_Y_L);	// Master <- Slave(sensor)
                                             	// reading a data from OUT_Y
        pBuf[2] = SPI1_Send(DUMMY);		// ���ӵ� Y ���� 16��Ʈ�̹Ƿ� �ΰ��� dummy�� ����
        pBuf[3] = SPI1_Send(DUMMY);
        SET_ACC_NSS1();
        UTIL_DelayUS(1);
  
	// Get ACC_Z
        CLR_ACC_NSS1();  
        SPI1_Send(CMD_READ | CMD_REG_OUT_Z_L);	// Master <- Slave(sensor)
                                             	// reading a data from OUT_Z
        pBuf[4] = SPI1_Send(DUMMY);		// ���ӵ� Z ���� 16��Ʈ�̹Ƿ� �ΰ��� dummy�� ����
        pBuf[5] = SPI1_Send(DUMMY);
        SET_ACC_NSS1();
        UTIL_DelayUS(1);  
  
        return true;
}

//   ���ӵ��� �а� 16bit ���·� ��ȯ,  pBuf[]�� return 
void SPI1_Process(UINT16 *pBuf)
{
        UINT8 buffer[3];
  
        SPI1_GetData(&buffer[0]);		// Get ACC value(X,Y,Z)
  
        SPI1_CalData(&buffer[0],&pBuf[0]);	// 8 bit data --> 16 bit data
}

// ���ӵ����� 16bit ���·� ��ȯ 
// �Է� pBuf[5..0]: 8bit 6���� ��(X_H,X_L, Y_H,Y_L,  Z_H,Z_L)
// ��� pTxBuf[2..0] : 16bit 3���� �� (X, Y, Z)
void SPI1_CalData(UINT8 *pBuf, UINT16 *pTxBuf) 
{
        UINT16 TempUniX0, TempUniY0, TempUniZ0;

        int16 TempX0, TempY0, TempZ0;

        TempX0 = (int16)(((int16)pBuf[1]<<8) | ((int16)pBuf[0]&0xFF));
        TempY0 = (int16)(((int16)pBuf[3]<<8) | ((int16)pBuf[2]&0xFF));
        TempZ0 = (int16)(((int16)pBuf[5]<<8) | ((int16)pBuf[4]&0xFF));
  
        pTxBuf[0] = abs(TempX0);
        pTxBuf[1] = abs(TempY0);
        pTxBuf[2] = abs(TempZ0);
}

