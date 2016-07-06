#include<reg52.h>
#include<intrins.h>

#define uchar unsigned char	//宏定义常用数据类型，方便书写
#define uint unsigned int
#define ulong unsigned long

sbit yq=P3^4;	//单片机P34口接L298的in1
sbit yh=P3^5;	//单片机P35口接L298的in2
sbit zq=P3^6;	//单片机P36口接L298的in3
sbit zh=P3^7;	//单片机P37口接L298的in4
sbit RX=P3^2;	//单片机P32口接超声波模块ECH0
sbit TX=P3^3;	//单片机P33口接超声波模块TRIG
sbit pwm=P0^3;	//单片机P03口输出PWM信号，控制舵机

uchar zd;

/******************延时函数**************************************/
void delay(uint z)
{
	uint x,y;
	for(x=z;x>0;x--)
		for(y=120;y>0;y--);
}

/******************电机控制函数**************************************/
void gostraight(uint t)	//小车前进
{
	zq=1;
	yq=1;
	zh=0;
	yh=0;
	delay(t);
}

void back(uint t)	//小车后退
{
	zq=0;
	yq=0;
	zh=1;
	yh=1;
	delay(t);		  
}

 void turnleft(uint t)	//小车左转
{
	zq=0;
	yq=1;
	zh=1;
	yh=0;
	delay(t);
}
		  
void turnright(uint t)	//小车右转
{
	zq=1;
	yq=0;
	zh=0;
	yh=1;
	delay(t);
}

void stop(uint t)	//小车停止   
{
	zq=0;
	yq=0;
	zh=0;
	yh=0;
	delay(t);		  
}

/******************超声波模块检测函数**************************************/
uint time=0;
uint timer=0;
uchar posit=0;
ulong S=0;				    
bit flag=0;
unsigned char const	positon[3]={0xdf,0xef,0xf7};
uchar disbuff[4]={0,0,0,0};

ulong Conut(void)
{
	ulong c;
	time=TH0*256+TL0;
	TH0=0;
	TL0=0;
	
	S=(time*1.7)/100;	//算出来是CM
	if((S>=700)||flag==1)	//超出测量范围显示“-”
	{	 
		flag=0;
		disbuff[0]=10;	   //“-”
		disbuff[1]=10;	   //“-”
		disbuff[2]=10;	   //“-”
	}
	else
	{
		disbuff[0]=S%1000/100;
		disbuff[1]=S%1000%100/10;
		disbuff[2]=S%1000%10 %10;
	}
	
	c=S;
	return c;
}

void init(void)
{
	TMOD=0x11;	//设T0为方式1，GATE=1；
	TH0=0;
	TL0=0;          
	TH1=0xf8;	//2MS定时
	TL1=0x30;
	ET0=1;	//允许T0中断
	ET1=1;	//允许T1中断
	TR1=1;	//开启定时器
	EA=1;	//开启总中断
}

ulong distance(void)
{
	ulong d;
	zd=0;
	init();
	delay(10);
	while(!RX);	//当RX为零时等待
	TR0=1;	//开启计数
	while(RX);	//当RX为1计数并等待
	TR0=0;	//关闭计数
	d=Conut();	//计算
	return d;
}

/******************舵机控制函数**************************************/
uint pwm_value=1580;	//初值为1.5ms

void InitTimer(void)
{
	TMOD=0x11;	//开定时器0,1
	TH0=-20000/256;	//定时20MS,20MS为一个周期
	TL0=-20000%256;
	TH1=-1500/256;	//定时1.5MS,这时舵机处于0度
	TL1=-1500%256;
	EA=1;	//开总断
	TR0=1;	//开定时器0
	ET0=1;
	TR1=1;	//开定时器1
	ET1=1;
}

void timer0(void) interrupt 1	//定时器0中断函数
{
	if(zd==0)
	{
		flag=1;	//中断溢出标志
	}
    else
	{
		pwm=1;
		TH0=-20000/256;
		TL0=-20000%256;
		TR1=1;
	}
}

void timer1(void) interrupt 3	//定时器1中断函数
{
	if(zd==0)
	{
		TH1=0xf8;
		TL1=0x30;
		timer++;
		if(timer>=400)
		{
			timer=0;
			TX=1;	//800MS  启动一次模块
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_();
			_nop_(); 
			_nop_(); 
			_nop_(); 
			_nop_();
			TX=0;
		} 
	}
	else
	{
		pwm=0;
		TH1=-pwm_value/256;
		TL1=-pwm_value%256;
		TR1=0;
	}
}

void duojistop(void)	//舵机回正中位置
{
	uchar i;
	zd=1;
	InitTimer();
	delay(100);
	for(i=0;i<10;i++)
	{
		pwm_value=1580;	//改变舵机角度
		delay(100);
	}	
}

void duojileft(void)	//舵机左转
{
	uchar i;
	zd=1;
	InitTimer();
	delay(100);
	for(i=0;i<10;i++)
	{
		pwm_value=2000;
		delay(100);
	}
}

void duojiright(void)	//舵机右转
{
	uchar i;
	zd=1;
	InitTimer();
	delay(100);
	for(i=0;i<10;i++)
	{
		pwm_value=1160;
		delay(100);
	}
}

/******************主函数**************************************/
int main()
{	   
	ulong d,dz,dy;
	duojistop();
	delay(1000);
	while(1)
	{
		d=distance();
		if(d>45)
		{
			gostraight(1);
		}
		else if(d>20) 
		{
			stop(1);
			duojileft();
			dz=distance();
			dz=distance();

			if(dz>45)
			{
				duojistop();
				turnleft(500);
				gostraight(1);
			}
			else
			{
				duojiright();
				dy=distance();
				dy=distance();
				if(dy>45)
				{
					duojistop();
					turnright(500);
					gostraight(1);
				}
				else
				{
					duojistop();
					turnright(2000);
					gostraight(1);
				}
			}
		}
		else if(d>10) 
		{
			stop(1);
			back(75);
			stop(1);
			duojileft();
			dz=distance();
			dz=distance();
	
			if(dz>45)
			{
				duojistop();
				turnleft(500);
				gostraight(1);
			}
			else
			{
				duojiright();
				dy=distance();
				dy=distance();
				if(dy>45)
				{
					duojistop();
					turnright(500);
					gostraight(1);
				}
				else
				{
					duojistop();
					turnright(2000);
					gostraight(1);
				}
			}
		}
		else
		{
			stop(1);
			back(150);
			stop(1);
			duojileft();
			dz=distance();
			dz=distance();

			if(dz>45)
			{
				duojistop();
				turnleft(500);
				gostraight(1);
			}
			else
			{
				duojiright();
				dy=distance();
				dy=distance();
				if(dy>45)
				{
					duojistop();
					turnright(500);
					gostraight(1);
				}
				else
				{
					duojistop();
					turnright(2000);
					gostraight(1);
				}
			}
		}
	}	
	return 0;	 
}		




