#include<LPC17xx.H>
unsigned char sw;
unsigned lat1[20]; 
unsigned lon1[20];
unsigned char lat[]="Latitude:";
unsigned char lon[]="Longitude:";
unsigned char msg2[]="Vehicle Monitoring & Remote locking system";
unsigned char msg3[]="Vehicle started";
unsigned char msg4[]="Vehicle stopped";
unsigned char msg5[]="Tracking...";
unsigned char gpsdata[200];
void delay(unsigned long int x);
void uart0_init();
void uart3_init();
void gsm_init();
void gps_init();
void gsm_sendsms();
void gsm_receivesms();
void gsm_sendlocation();
void gsm_confirm();
void lcdwrt(unsigned char y);
int main()
{
	unsigned char cmd[]={0x38,0x0E,0x06,0x01,0x80};
	unsigned char i;
	SystemInit();
	uart0_init();
	uart3_init();
	gsm_init();
	LPC_GPIO0->FIOMASK3=0xF7;		//switch
	LPC_GPIO0->FIODIR3=0xF7;
	LPC_GPIO0->FIOMASK2=0xBF;		//relay
	LPC_GPIO0->FIODIR2=0x40;
	LPC_GPIO2->FIOMASK0=0x00;		//LCD
	LPC_GPIO2->FIODIR0=0xFF;
	LPC_GPIO1->FIOMASK1=0xF8;
	LPC_GPIO1->FIODIR1=0x07;
	LPC_GPIO1->FIOCLR1=0x02;		//RW=0
	LPC_GPIO1->FIOCLR1=0x01;		//RS=0
	for(i=0;i<5;i++)
	{
		lcdwrt(cmd[i]);
	}
	LPC_GPIO1->FIOSET1=0x01;		//RS=1
	for(i=0;msg2[i]!='\0';i++)
	{
		lcdwrt(msg2[i]);
	}
	while(1)
	{
		sw=((LPC_GPIO0->FIOPIN3&0x08)>>3);
		if(sw==1)
		{
			LPC_GPIO0->FIOSET2=0x40;
			LPC_GPIO1->FIOSET1=0x01;		//RS=1
			for(i=0;msg3[i]!='\0';i++)
			{
				lcdwrt(msg3[i]);
			}
			gsm_sendsms();
			gsm_receivesms();
		}
		else
		{
			LPC_GPIO0->FIOCLR2=0x40;
		}
	}
}
void delay(unsigned long int x)
{
	unsigned long int i;
	for(i=0;i<x;i++);
}
void uart0_init()
{
	LPC_PINCON->PINSEL0|=0x00000050;
	LPC_SC->PCLKSEL0|=0x00000000;
	LPC_UART0->LCR=0x83;
	LPC_UART0->DLM=0x00;
	LPC_UART0->DLL=0x75;
	LPC_UART0->FDR=0x00000010;
	LPC_UART0->LCR=0x03;
}
void uart3_init()
{
	LPC_SC->PCONP|=0x02000000;
	LPC_PINCON->PINSEL9|=0x0F000000;
	LPC_UART3->LCR=0x83;
	LPC_UART3->DLM=0x00;
	LPC_UART3->DLL=0x75;
	LPC_UART3->FDR=0x00000010;
	LPC_UART3->LCR=0x03;
}
void gsm_init()
{
	unsigned char i;
	unsigned char cmd1[]="AT\r\n";
	unsigned char cmd2[]="AT+CREG?\r\n";
	unsigned char cmd3[]="AT+CMGF=1\r\n";
	for(i=0;cmd1[i]!='\0';i++)
	{
		LPC_UART0->THR=cmd1[i];
		while((LPC_UART0->LSR&0x20)!=0x20);
	}
	delay(0x60000);
	for(i=0;cmd2[i]!='\0';i++)
	{
		LPC_UART0->THR=cmd2[i];
		while((LPC_UART0->LSR&0x20)!=0x20);
	}
	delay(0x60000);
	for(i=0;cmd3[i]!='\0';i++)
	{
		LPC_UART0->THR=cmd3[i];
		while((LPC_UART0->LSR&0x20)!=0x20);
	}
	delay(0x60000);
}
void gsm_sendsms()
{
	unsigned char i;
	unsigned char cmd4[]="AT+CMGS=\"9110264143\"\r\n";
	unsigned char msg1[]="VEHICLE STARTED";
	for(i=0;cmd4[i]!='\0';i++)
	{
		LPC_UART0->THR=cmd4[i];
		while((LPC_UART0->LSR&0x20)!=0x20);
	}
	delay(0x60000);
	for(i=0;msg1[i]!='\0';i++)
	{
		LPC_UART0->THR=msg1[i];
		while((LPC_UART0->LSR&0x20)!=0x20);
	}
	LPC_UART0->THR=0x1A;
	while((LPC_UART0->LSR&0x20)!=0x20);
	delay(0x60000);
}
void gps_init()
{
	unsigned char i;
	unsigned char val;
	while(1)
	do
	{
		while((LPC_UART3->LSR&0x01)!=0x01)
		val=LPC_UART3->RBR;
	} while (val!='$');
	for(i=0;i<100;i++)
	{
		while((LPC_UART3->LSR&0x01)!=0x01)
		gpsdata[i]=LPC_UART3->RBR;
	}
	if(gpsdata[2]=='R')
	{
		if(gpsdata[17]=='A')
			{
				for(i=19;i<=29;i++)
				{
					lat1[i]=gpsdata[i];
					while((LPC_UART0->LSR&0x20)!=0x20);
				}
				for(i=31;i<=42;i++)
					{
						lon1[i]=gpsdata[i];
						while((LPC_UART0->LSR&0x20)!=0x20);
					}
			}
		else;
	}
	else;
}
void gsm_receivesms()
{
	unsigned char rxval[10];
	unsigned char i;
	while((LPC_UART0->LSR&0x01)!=0x01)
		rxval[i]=LPC_UART0->RBR;
	if (rxval[0]=='S')
	{
		if (rxval[1]=='T')
		{
			if (rxval[2]=='O')
			{
				if (rxval[3]=='P')
				{
					LPC_GPIO0->FIOCLR2=0x40;
					gsm_confirm();
					LPC_GPIO1->FIOSET1=0x01;	//RS=1
					for(i=0;msg4[i]!='\0';i++)
					{
						lcdwrt(msg4[i]);
					}
				}
			}
		}
	}
	if (rxval[0]=='T')
	{
		if (rxval[1]=='R')
		{
			if (rxval[2]=='A')
			{
				if (rxval[3]=='K')
				{
					LPC_GPIO1->FIOSET1=0x01;	//RS=1
					for(i=0;msg5[i]!='\0';i++)
					{
						lcdwrt(msg5[i]);
					}
					gps_init();
					gsm_sendlocation();
				}
			}
		}
	}
	
}
void gsm_sendlocation()
{
	unsigned char i;
	unsigned char cmd4[]="AT+CMGS=\"9110264143\"\r\n";
	for(i=0;cmd4[i]!='\0';i++)
	{
		LPC_UART0->THR=cmd4[i];
		while((LPC_UART0->LSR&0x20)!=0x20);
	}
	delay(0x60000);
	for(i=0;lat[i]!='\0';i++)
	{
		LPC_UART0->THR=lat[i];
		while((LPC_UART0->LSR&0x20)!=0x20);
	}
	delay(0x60000);
	for(i=0;lat1[i]!='\0';i++)
	{
		LPC_UART0->THR=lat1[i];
		while((LPC_UART0->LSR&0x20)!=0x20);
	}
	delay(0x60000);
	for(i=0;lon[i]!='\0';i++)
	{
		LPC_UART0->THR=lon[i];
		while((LPC_UART0->LSR&0x20)!=0x20);
	}
	delay(0x60000);
	for(i=0;lon1[i]!='\0';i++)
	{
		LPC_UART0->THR=lon1[i];
		while((LPC_UART0->LSR&0x20)!=0x20);
	}
	delay(0x60000);
	LPC_UART0->THR=0x1A;
	while((LPC_UART0->LSR&0x20)!=0x20);
	delay(0x60000);
}
void lcdwrt(unsigned char y)
{
	LPC_GPIO2->FIOPIN0=y;
	LPC_GPIO1->FIOSET1=0x04;	//EN=1
	delay(0x10000);
	LPC_GPIO1->FIOCLR1=0x04;	//EN=0
	delay(0x10000);
}
void gsm_confirm()
{
	unsigned char i;
	unsigned char cmd4[]="AT+CMGS=\"9110264143\"\r\n";
	unsigned char msg4[]="Vehicle Stopped";
	for(i=0;cmd4[i]!='\0';i++)
	{
		LPC_UART0->THR=cmd4[i];
		while((LPC_UART0->LSR&0x20)!=0x20);
	}
	delay(0x60000);
	for(i=0;msg4[i]!='\0';i++)
	{
		LPC_UART0->THR=msg4[i];
		while((LPC_UART0->LSR&0x20)!=0x20);
	}
	LPC_UART0->THR=0x1A;
	while((LPC_UART0->LSR&0x20)!=0x20);
	delay(0x60000);
}