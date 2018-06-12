#ifndef __G_Var_H
#define __G_Var_H

extern unsigned char tab[];
extern unsigned char IsKeyDownRec[16];
void Write_allLED(unsigned char LED_flag);
void init_TM1638(void);
void Write_oneLED(unsigned char num,unsigned char flag);
void Write_DATA(unsigned char add,unsigned char DATA);
void Read_key(void);
void Write_COM(unsigned char cmd);
unsigned char TM1638_Read(void)	;
void TM1638_Write(unsigned char	DATA);


#endif

