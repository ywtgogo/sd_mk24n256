#include "stc15w10x.h"
#include "intrins.h"
#define GIO_set(x) (1<<x)

#define GIO_setWeakOutput(x) do {P3M1 &= ~(1<<x);	P3M0 &= ~(1<<x);} while(0)
#define GIO_setStrongOutput(x) do {P3M1 &= ~(1<<x);	P3M0 |= (1<<x);} while(0)
#define GIO_setInput(x) do {P3M1 |= (1<<x); P3M0 &= ~(1<<x);} while(0)
#define LongDelay() 	do{unsigned char loop = 200;while(loop--)	Delay20ms();}while(0)
#define ClrWDT()	do{WDT_CONTR |= 0x10;}while(0)

sbit DC_Power = P3^2;
#define DC_Power_ON 1
#define DC_Power_OFF 0

sbit SYSPower_Enabele = P3^3;
#define SYS_POWER_ON 1
#define SYS_POWER_OFF 0

sbit SYSPower_Status = P3^5;
sbit PowerDown_By_K24 = P3^4;			//K24申请关机用管脚；K24将该管脚拉低100ms以上，将切断整个系统电源

sbit PowerDown_Status = P3^1;

sbit KEY = P3^0;
#define KEY_PRESS 0

bit KeyPress_Poweron =0;

void Delay20ms();
void sleep();

//-----------------------------------------------
//中断服务程序
void exint0() interrupt 0       //INT0外部电源检测
{
	EA = 0;EX0 = 0;
	
	if(DC_Power == DC_Power_ON)			//外部电源上电
	{
		Delay20ms();
		if(DC_Power == DC_Power_ON)
		{
			SYSPower_Enabele = SYS_POWER_ON;
			INT_CLKO |= 0x10;               //开启K24主动关机中断
		}
	}
	
	EX0 = 1;EA = 1;
}
void exint2() interrupt 10          //K24主动关机信号
{
	EA = 0;
  INT_CLKO &= 0xEF;               //关闭中断,此时系统会自动清除内部的中断标志
	
	if(PowerDown_By_K24 == 0)
	{
		Delay20ms();
		Delay20ms();
		Delay20ms();
		Delay20ms();
		Delay20ms();		//去抖动100ms，防止系统重启时管脚跌落
		if(PowerDown_By_K24 == 0)
		{
			SYSPower_Enabele = SYS_POWER_OFF;
		}
	}
	
	INT_CLKO |= 0x10;               //重新再开中断
	EA = 1;
}

void exint4() interrupt 16          //电源按键中断
{
	EA = 0;
	INT_CLKO &= 0xBF;		//关中断
	
	//SYSPower_Enabele = POWER_ON;
	if (SYSPower_Status == SYS_POWER_OFF)
		KeyPress_Poweron = 1;
	
	INT_CLKO |= 0x40;               //重新再开中断
	EA = 1;
}

void main()
{
	//P30，高阻态，按键输入
	GIO_setInput(0);
	//P31，强上拉输出，向K24系统发送关机信号
	GIO_setStrongOutput(1);
	PowerDown_Status = 1;
	//P32，高阻态，外部电源检测，有外部电源为高电平。上升沿中断
	GIO_setInput(2);
	//P33，推挽输出，系统电源开关，ON=1，OFF=0
	GIO_setStrongOutput(3);
	//P34，高阻态，下降沿唤醒。K24申请关闭电源时为0，正常为高电平
	GIO_setInput(4);
	//P35，高阻态，不需要中断。系统电源状态检测，系统电源已供电为高电平
	GIO_setInput(5);
	
	
	if( DC_Power == DC_Power_ON )
	{
		SYSPower_Enabele = SYS_POWER_ON;
		LongDelay();		//等待K24启动后稳定
	}
	

	
	while(1)
	{
		if(KEY == KEY_PRESS)		//电源按键动作
		{
			if (SYSPower_Status == SYS_POWER_ON)			//开机状态下，长按7秒进行关机
			{
				unsigned int loop = 300;
				do
				{
					Delay20ms();
					if(KEY == KEY_PRESS)
					{
						--loop;
						if(loop < 200)
						{
							PowerDown_Status = 0;
						}
						if (loop == 0)
						{
							SYSPower_Enabele = SYS_POWER_OFF;
							ClrWDT();
							while (KEY == KEY_PRESS)	ClrWDT();
						}
						else
						{
							
						}
					}
					else			//未完成长按键7S的动作，中间按键被松掉
					{
						PowerDown_Status = 1;
						loop = 0;
					}
				}while(loop);
			}
			else //关机状态下准备开机
			{
				if(KeyPress_Poweron)	//开机按键动作，执行开机程序
				{
					KeyPress_Poweron = 0;
					Delay20ms();
					if(KEY == KEY_PRESS)
					{
						SYSPower_Enabele = SYS_POWER_ON;
						INT_CLKO |= 0x10;               //开启K24主动关机中断
						PowerDown_Status = 1;
					}
				}
			}
		}
		
		ClrWDT();
		INT_CLKO |= 0x10;				//使能外部关机信号中断
		INT_CLKO |= 0x40;       //使能开关机按键中断
		IT0 = 0;
		EX0 = 1;                //使能外部电源检测中断
		sleep();
	}		
}


void sleep()
{
	KeyPress_Poweron = 0;		//开机按键标志位取消
//	INT_CLKO |= 0x40;       //(EX4 = 1)使能开关机按键中断
//	INT_CLKO |= 0x10;				//开K24关机信号唤醒
//	EX0 = 1;                //使能外部电源检测中断
	
	EA = 1;
	_nop_();
	_nop_();
	PCON |= 0x02;           //将STOP(PCON.1)置1,MCU将进入掉电模式
	_nop_();                //此时CPU无时钟,不执行指令,且所欲外设停止工作
	_nop_();                //外部中断信号和外部复位信号可以终止掉电模式
	_nop_();
	_nop_();
}


void Delay20ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	i = 1;
	j = 216;
	k = 35;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
	ClrWDT();
}
