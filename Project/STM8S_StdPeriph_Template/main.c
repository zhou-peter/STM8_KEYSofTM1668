/**
******************************************************************************
* @file    Project/main.c 
* @author  MCD Application Team
* @version V2.2.0
* @date    30-September-2014
* @brief   Main program body
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
*
* Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
* You may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
*        http://www.st.com/software_license_agreement_liberty_v2
*
* Unless required by applicable law or agreed to in writing, software 
* distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************************
*/ 


/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "delay.h"
#include "TM1638.h" 
/* Private defines -----------------------------------------------------------*/

#define BUTTON_FILTER_TIME         5  
#define BUTTON_LONG_TIME           50                /* 持续1秒，认为长按事件 */
#define Key_Num  8




u8 KeyDownCode[]= {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f};
//按键按下的键值
u8 KeyUpCode[]={0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f};
//按键弹起的键值
u8 KeyLongCode[]={0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff};
//按键长按的键值
unsigned char num[8]={0,0,1,1,2,2,3,3};		//各个数码管显示的值

typedef struct  
{  
  /* 下面是一个函数指针，指向判断按键手否按下的函数 */  
  
  
  unsigned char  which_key;
  //unsigned char  (*IsKeyDownFunc)(unsigned char which_key); /* 按键按下的判断函数,1表示按下 */    
  unsigned char  Count;                        /* 滤波器计数器 */  
  unsigned char  FilterTime;                /* 滤波时间(最大255,表示2550ms) */  
  unsigned short LongCount;                /* 长按计数器 */  
  unsigned short LongTime;                /* 按键按下持续时间, 0表示不检测长按 */  
  unsigned char  State;                        /* 按键当前状态（按下还是弹起） */  
  unsigned char  KeyCodeUp;                /* 按键弹起的键值代码, 0表示不检测按键弹起 */  
  unsigned char  KeyCodeDown;        /* 按键按下的键值代码, 0表示不检测按键按下 */  
  unsigned char  KeyCodeLong;        /* 按键长按的键值代码, 0表示不检测长按 */  
  unsigned char  RepeatSpeed;        /* 连续按键周期 */  
  unsigned char  RepeatCount;        /* 连续按键计数器 */  
}BUTTON_T; 

BUTTON_T Key[16];




/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static void UART_Config(void);

void Pannelkey_Polling(void);
void Button_Detect(BUTTON_T *_pBtn);
unsigned char IsKey_Down(unsigned char which_key);

void Key_Init(void);
void IsKeyDownDetect(void);

void key_display(u8 i);
//
//u8 KeyValue,COLUMN_VALUE1,COLUMN_VALUE2,COLUMN_VALUE3,COLUMN_VALUE4,column_value;
//u8 ROW_VALUE1,ROW_VALUE2,ROW_VALUE3,ROW_VALUE4,row_value;

/* 按键滤波时间50ms, 单位10ms 
*只有连续检测到50ms状态不变才认为有效，包括弹起和按下两种事件 
*/  


/* 
每个按键对应1个全局的结构体变量。 
其成员变量是实现滤波和多种按键状态所必须的 
*/  

void main(void)
{
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
  CLK_HSICmd(ENABLE);
  
  delay_init(16);
  Key_Init();
  UART_Config();
  
  init_TM1638();	                           //初始化TM1638
  for(u8 i=0;i<8;i++)
    Write_DATA(i<<1,tab[num[i]]);	               //初始化寄存器
  
  while(1){
    
    Pannelkey_Polling();
  }
  
  /* Infinite loop */
  
}

/* 
********************************************************************************************************* 
*        函 数 名: bsp_DetectButton 
*        功能说明: 检测一个按键。非阻塞状态，必须被周期性的调用。 
*        形    参：按键结构变量指针 
*        返 回 值: 无 
********************************************************************************************************* 
*/  
void Button_Detect(BUTTON_T *_pBtn)  
{  
  if (IsKeyDownRec[_pBtn->which_key])  
  {  
    IsKeyDownRec[_pBtn->which_key]=0;
    if (_pBtn->Count < _pBtn->FilterTime)  
    {  
      _pBtn->Count = _pBtn->FilterTime;  
    }  
    else if(_pBtn->Count < 2 * _pBtn->FilterTime)  
    {  
      _pBtn->Count++;  
    }  
    else  
    {  
      if (_pBtn->State == 0)  
      {  
        _pBtn->State = 1;  
        
        /* 发送按钮按下的消息 */  
        if (_pBtn->KeyCodeDown > 0)  
        {  
          /* 键值放入按键FIFO */ 
          key_display(_pBtn->which_key);
//          UART1_SendData8(_pBtn->KeyCodeDown);// 记录按键按下标志，等待释放  
//          while( UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET );
        }  
      }  
      
      if (_pBtn->LongTime > 0)  
      {  
        if (_pBtn->LongCount < _pBtn->LongTime)  
        {  
          /* 发送按钮持续按下的消息 */  
          if (++_pBtn->LongCount == _pBtn->LongTime)  
          {  
            /* 键值放入按键FIFO */  
//            UART1_SendData8(_pBtn->KeyCodeLong);          
//            while( UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET );
          }  
        }  
        else  
        {  
          if (_pBtn->RepeatSpeed > 0)  
          {  
            if (++_pBtn->RepeatCount >= _pBtn->RepeatSpeed)  
            {  
              _pBtn->RepeatCount = 0;  
              /* 常按键后，每隔10ms发送1个按键 */  
//              UART1_SendData8(_pBtn->KeyCodeDown);          
//              while( UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET );
            }  
          }  
        }  
      }  
    }  
  }  
  else  
  {  
    if(_pBtn->Count > _pBtn->FilterTime)  
    {  
      _pBtn->Count = _pBtn->FilterTime;  
    }  
    else if(_pBtn->Count != 0)  
    {  
      _pBtn->Count--;  
    }  
    else  
    {  
      if (_pBtn->State == 1)  
      {  
        _pBtn->State = 0;  
        
        /* 发送按钮弹起的消息 */  
        if (_pBtn->KeyCodeUp > 0) /*按键释放*/  
        {  
          /* 键值放入按键FIFO */  
//          UART1_SendData8(_pBtn->KeyCodeUp);          
//          while( UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET );
        }  
      }  
    }  
    
    _pBtn->LongCount = 0;  
    _pBtn->RepeatCount = 0;  
  }  
}  
//功能说明: 检测所有按键。10MS 调用一次  
void Pannelkey_Polling(void)  
{ 
  Read_key();
  for(u8 i=0;i<8;i++)
  {
    Button_Detect(&Key[i]);                /*Key_1 键 */  
    
  }
  delay_ms(10);
} 
void key_display(u8 i)
{
  if(i<8)
  {
    num[i]++;
    if(num[i]>15)
      num[i]=0;
    Write_DATA(i*2,tab[num[i]]);
    Write_allLED(1<<i);
  }
  
}
static void UART_Config(void)
{
  /* Deinitializes the UART1 and UART3 peripheral */
  UART1_DeInit();
  // UART3_DeInit();
  /* UART1 and UART3 configuration -------------------------------------------------*/
  /* UART1 and UART3 configured as follow:
  - BaudRate = 9600 baud  
  - Word Length = 8 Bits
  - One Stop Bit
  - No parity
  - Receive and transmit enabled
  - UART1 Clock disabled
  */
  /* Configure the UART1 */
  UART1_Init((u32)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
             UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
  
  /* Enable UART1 Transmit interrupt*/
  UART1_ITConfig(UART1_IT_RXNE_OR, ENABLE);
 
  enableInterrupts(); 
  UART1_Cmd(ENABLE);
  UART1_SendData8(0x00);
}


void Key_Init()
{
  for(u8 i=0;i<8;i++){
    
    Key[i].which_key=i;
    /* 初始化USER按键变量，支持按下、弹起、长按 */  
    //Key[i].IsKeyDownFunc = IsKey_Down;       /* 判断按键按下的函数 */  
    Key[i].FilterTime = BUTTON_FILTER_TIME;      /* 按键滤波时间 */  
    Key[i].LongTime = BUTTON_LONG_TIME;          /* 长按时间 */  
    Key[i].Count = Key[i].FilterTime / 2;    /* 计数器设置为滤波时间的一半 */  
    Key[i].State = 0;                            /* 按键缺省状态，0为未按下 */  
    Key[i].KeyCodeDown = KeyDownCode[i];         /* 按键按下的键值代码 */  
    Key[i].KeyCodeUp =KeyUpCode[i];              /* 按键弹起的键值代码 */  
    Key[i].KeyCodeLong = KeyLongCode[i];         /* 按键被持续按下的键值代码 */  
    Key[i].RepeatSpeed = 0;                      /* 按键连发的速度，0表示不支持连发 */  
    Key[i].RepeatCount = 0;                      /* 连发计数器 */    

  }
}


//unsigned char IsKeyDown(unsigned char which_key)
//{
//  //if()
//  return 0;
//}











#ifdef USE_FULL_ASSERT

/**
* @brief  Reports the name of the source file and the source line number
*   where the assert_param error has occurred.
* @param file: pointer to the source file name
* @param line: assert_param error line source number
* @retval : None
*/
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
