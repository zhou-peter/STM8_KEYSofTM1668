#include "stm8s.h"
#include "delay.h"
#include "Pin_Allocation.h"
#include "TM1638.h"  
#include "G_Var.h"

unsigned char IsKeyDownRec[16]={0};
unsigned char tab[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,
                           0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};

void TM1638_Write(unsigned char	DATA)			//写数据函数
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		GPIO_WriteLow(TMCLK);
		if(DATA&0X01)
			GPIO_WriteHigh(TMDIO);
		else
			GPIO_WriteLow(TMDIO);
		DATA>>=1;
		GPIO_WriteHigh(TMCLK);
	}
}
unsigned char TM1638_Read(void)					//读数据函数
{
  unsigned char i;
  unsigned char temp=0;
  //GPIO_WriteHigh(TMDIO);	//设置为输入
  
  for(i=0;i<8;i++)
  {
    temp>>=1;
    GPIO_WriteLow(TMCLK);
    BitStatus bs;
    bs = GPIO_ReadInputPin(TMDIO);
    if(bs)
      temp|=0x80;
    GPIO_WriteHigh(TMCLK);
    
  }
  
  return temp;
}
void Write_COM(unsigned char cmd)		//发送命令字
{
	GPIO_WriteLow(TMSTB);
	TM1638_Write(cmd);
	GPIO_WriteHigh(TMSTB);
}
void Read_key(void)
{
  unsigned char c[4],i,key_value=0;

  
  GPIO_WriteLow(TMSTB);
  TM1638_Write(0x42);		           //读键扫数据 命令
  GPIO_Init(TMDIO, GPIO_MODE_IN_PU_NO_IT);//TMDIO设置为输入
  for(i=0;i<4;i++){		
    c[i]=TM1638_Read();
    
  }
  GPIO_Init(TMDIO, GPIO_MODE_OUT_PP_HIGH_FAST);//TMDIO设置为输出
  GPIO_WriteHigh(TMSTB);					           //4个字节数据合成一个字节
  for(i=0;i<4;i++)
    key_value|=c[i]<<i;
  for(i=0;i<8;i++){
    if((0x01<<i)&(key_value)){
      IsKeyDownRec[i]=1;
    }
  }
  
}
void Write_DATA(unsigned char add,unsigned char DATA)		//指定地址写入数据
{
	Write_COM(0x44);
	GPIO_WriteLow(TMSTB);
	TM1638_Write(0xc0|add);
	TM1638_Write(DATA);
	GPIO_WriteHigh(TMSTB);
}

void Write_oneLED(unsigned char num,unsigned char flag)	//单独控制一个LED函数，num为需要控制的led序号，flag为0时熄灭，不为0时点亮
{
	if(flag)
		Write_DATA(2*num+1,1);
	else
		Write_DATA(2*num+1,0);
}  	
void Write_allLED(unsigned char LED_flag)					//控制全部LED函数，LED_flag表示各个LED状态
{
	unsigned char i;
	for(i=0;i<8;i++)
		{
			if(LED_flag&(1<<i))
				Write_DATA(2*i+1,3);
				//Write_DATA(2*i+1,1);
			else
				Write_DATA(2*i+1,0);
		}
}

//TM1638初始化函数
void init_TM1638(void)
{
	unsigned char i;
        GPIO_Init(TMSTB, GPIO_MODE_OUT_PP_HIGH_FAST);
        GPIO_Init(TMCLK, GPIO_MODE_OUT_PP_HIGH_FAST);
        GPIO_Init(TMDIO, GPIO_MODE_OUT_PP_HIGH_FAST);
        Write_COM(0x03);
	Write_COM(0x8f);       //亮度 (0x88-0x8f)8级亮度可调  (小于等于0x87，则灯熄灭)
	Write_COM(0x40);       //采用地址自动加1
	GPIO_WriteLow(TMSTB);		           //
	TM1638_Write(0xc0);    //设置起始地址

	for(i=0;i<16;i++)	   //传送16个字节的数据
		TM1638_Write(0x00);
	GPIO_WriteHigh(TMSTB);
}