/***************************************************************************
 *                                                                         *
 * Copyright (c) 2008 Nuvoton Technolog. All rights reserved.              *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include "wblib.h"
#include "RTC.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
UINT32 volatile g_u32TICK = 0;
BOOL volatile g_bAlarm = FALSE;
VOID Smpl_RTC_Powerdown_Wakeup(VOID);
VOID Smpl_RTC_Powerdown_Wakeup_Relative(VOID);
VOID Smpl_RTC_PowerOff_Control(UINT32 u32Mode);

#define HW_POWER_OFF	0
#define SW_POWER_OFF	1

/*---------------------------------------------------------------------------------------------------------*/
/* RTC Tick Callback function                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
VOID RTC_TickISR(VOID)
{
	RTC_TIME_DATA_T sCurTime;
	
	/* Get the currnet time */
	RTC_Read(RTC_CURRENT_TIME, &sCurTime);		
	
	sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);				
	RTC_EnableClock(TRUE);
	g_u32TICK++;
}

/*---------------------------------------------------------------------------------------------------------*/
/* RTC Alarm Callback function                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
VOID RTC_AlarmISR(VOID)
{
	
	sysprintf("   Abolute Alarm!!\n");
	
	g_bAlarm = TRUE; 
}

VOID RTC_Releative_AlarmISR(VOID)
{
	
	sysprintf("   Relative Alarm!!\n");
	
	g_bAlarm = TRUE; 
}

int main()
{
	WB_UART_T uart;
	UINT32 u32Item, u32ExtFreq;
	RTC_TIME_DATA_T sInitTime;
	UINT32 u32ClockSource;
		
	u32ExtFreq = sysGetExternalClock();
	sysUartPort(1);
	uart.uiFreq = u32ExtFreq;
	uart.uiBaudrate = 115200;
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
	sysInitializeUART(&uart);

	/* RTC Initialize */
	RTC_Init();	
	
	/* Time Setting */
	sInitTime.u32Year = 2010;
	sInitTime.u32cMonth = 11;	
	sInitTime.u32cDay = 25;
	sInitTime.u32cHour = 13;
	sInitTime.u32cMinute = 20;
	sInitTime.u32cSecond = 0;
	sInitTime.u32cDayOfWeek = RTC_FRIDAY;
	sInitTime.u8cClockDisplay = RTC_CLOCK_24;			

	/* Initialization the RTC timer */
	if(RTC_Open(&sInitTime) !=E_RTC_SUCCESS)
		sysprintf("Open Fail!!\n");
			
	sysSetLocalInterrupt(ENABLE_IRQ);	
	
	/* Clock Test Mode PAD Control */
	outp32(REG_GPBFUN, inp32(REG_GPBFUN) | MF_GPB0);
	
	/* Clock Test Mode to Monitor HCLK */
	outp32(REG_CLK_TREG, 0x85);

	sysprintf("Do RTC Calibration \n");	
	RTC_Ioctl(0,RTC_IOC_SET_FREQUENCY, 0,0);
	
	do
	{    	
		sysprintf("=======================================\n");
		sysprintf("             RTC Demo Code	          \n");		
		sysprintf("=======================================\n");	
		RTC_Ioctl(0,RTC_IOC_GET_CLOCK_SOURCE, (UINT32)&u32ClockSource,0);	
		if(u32ClockSource == RTC_INTERANL)
			sysprintf("         (Clock from Internal) 	      \n");  
		else
			sysprintf("         (Clock from External) 	      \n");  
		sysprintf("=======================================\n");	
		sysprintf("Select Demo Item:\n");
		sysprintf("0. Time Display Test\n");
		sysprintf("1. Alarm Test (Absolute)\n");
		sysprintf("2. Alarm Test (Relative)\n");
		sysprintf("3. Power down Wakeup Test (Absolute)\n");
		sysprintf("4. Power down Wakeup Test (Relative)\n");
		sysprintf("5. S/W Force to Power Off Test\n");
		sysprintf("6. S/W Power Off (Normal Case) Control Flow Test\n");
		sysprintf("7. H/W Power Off (System Crash) Control Flow Test\n");			
		sysprintf("8. Change RTC Clock Source\n");
		sysprintf("9. Do Calibration\n");
		sysprintf("Q. Exit\n");
		u32Item = sysGetChar();			
		
		switch(u32Item)
		{
			case '0':
			{
				RTC_TICK_T sTick;
				
				sysprintf("\n0. RTC Time Display Test (Exit after 5 seconds)\n");
					
				/* Set Tick property */
				sTick.ucMode = RTC_TICK_1_SEC;
				sTick.pfnTickCallBack = RTC_TickISR;		
				
				/* Set Tick setting */
				RTC_Ioctl(0,RTC_IOC_SET_TICK_MODE, (UINT32)&sTick,0);

				/* Enable RTC Tick Interrupt and install tick call back function */
				RTC_Ioctl(0,RTC_IOC_ENABLE_INT, (UINT32)RTC_TICK_INT,0);	
				
				g_u32TICK = 0;
				
				RTC_EnableClock(TRUE);
				
				while(g_u32TICK < 5);
				
				/* Disable RTC Tick Interrupt */
				RTC_Ioctl(0,RTC_IOC_DISABLE_INT, (UINT32)RTC_TICK_INT,0);							
				break;
			}
			case '1':
			{
				RTC_TIME_DATA_T sCurTime;	
				
				sysprintf("\n1. RTC Alarm Test (Absolute)-> Alarm after 10 seconds\n");
					
				g_bAlarm = FALSE;	
				
				/* Get the currnet time */
				RTC_Read(RTC_CURRENT_TIME, &sCurTime);
				
				/* Set Alarm call back function */
				sCurTime.pfnAlarmCallBack = RTC_AlarmISR;

				sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);
				
				/* The alarm time setting */	
				sCurTime.u32cSecond = sCurTime.u32cSecond + 10; 	
				
				if(sCurTime.u32cSecond >= 60)
				{
					sCurTime.u32cSecond = sCurTime.u32cSecond - 60;		
					
					sCurTime.u32cMinute++; 	
					if(sCurTime.u32cMinute >= 60)
					{
						sCurTime.u32cMinute = sCurTime.u32cMinute - 60;
						
						sCurTime.u32cHour++; 
						if(sCurTime.u32cHour >= 24)
						{
							sCurTime.u32cHour = 0;
							sCurTime.u32cDay++;
						}			
					}				
				}					
							
				/* Set the alarm time (Install the call back function and enable the alarm interrupt)*/
				RTC_Write(RTC_ALARM_TIME,&sCurTime);
				
				RTC_EnableClock(TRUE);
						
				while(!g_bAlarm);
				
				RTC_EnableClock(FALSE);				

				/* Get the currnet time */
				RTC_Read(RTC_CURRENT_TIME, &sCurTime);				
				
				sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);
							
				break;				
			}
			case '2':
			{
				RTC_TIME_DATA_T sCurTime;	
				
				sysprintf("\n2. RTC Alarm Test (Relative) -> Alarm after 10 seconds\n");
					
				g_bAlarm = FALSE;	
				
				/* Get the currnet time */
				RTC_Read(RTC_CURRENT_TIME, &sCurTime);
				
				sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);								
				
				/* Enable RTC Relative Alarm Interrupt and install call back function */
				RTC_Ioctl(0,RTC_IOC_SET_RELEATIVE_ALARM, 10, (UINT32)RTC_Releative_AlarmISR);	
	
				RTC_EnableClock(TRUE);
	
				while(!g_bAlarm);			
			
				RTC_EnableClock(FALSE);				

				/* Get the currnet time */
				RTC_Read(RTC_CURRENT_TIME, &sCurTime);				
	
				sysprintf("   Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32cMonth,sCurTime.u32cDay,sCurTime.u32cHour,sCurTime.u32cMinute,sCurTime.u32cSecond);
											
				break;				
			}			
			case '3':
				sysprintf("\n3. Power down Wakeup Test (Absolute)\n");
		
				Smpl_RTC_Powerdown_Wakeup();	
				break;
			case '4':
				sysprintf("\n4. Power down Wakeup Test (Relative)\n");
		
				Smpl_RTC_Powerdown_Wakeup_Relative();	
				break;	
			case '5':			
				sysprintf("\n5. S/W Force to Power Off Test\n");	
				sysprintf("   Force to Power Off imediatedly\n");							
				RTC_Ioctl(0, RTC_IOC_SET_POWER_OFF, 0, 0);
				while(1);
				break;											
			case '6':
				sysprintf("\n6. S/W Power Off (Normal Case) Control Flow Test\n");
				Smpl_RTC_PowerOff_Control(SW_POWER_OFF);
				break;
			case '7':
				sysprintf("\n7. H/W Power Off (System Crash!) Control Flow Test\n");
				Smpl_RTC_PowerOff_Control(HW_POWER_OFF);
				break;	
			case '8':
			{			
				sysprintf("Change RTC Clock\n");	
				u32ClockSource++;
				if(u32ClockSource % 2 )
					u32ClockSource = RTC_INTERANL;
				else
					u32ClockSource = RTC_EXTERNAL;
				RTC_Ioctl(0,RTC_IOC_SET_CLOCK_SOURCE, u32ClockSource,0);	
				
				RTC_Ioctl(0,RTC_IOC_GET_CLOCK_SOURCE, (UINT32)&u32ClockSource,0);	
				if(u32ClockSource == RTC_INTERANL)
					sysprintf("->Clock from Internal now\n");  
				else
					sysprintf("->Clock from External now\n");  
					
				break;
			}		
			case '9':
			{	
				sysprintf("Do RTC Calibration\n");	
				RTC_Ioctl(0,RTC_IOC_SET_FREQUENCY, 0,0);
				break;
			}				
			default:
				sysprintf("Wrong Item\n");
				break;			
		}
		
	}while((u32Item!= 'q') || (u32Item!= 'Q'));	
			
	
	/* Disable RTC  */		
	RTC_Close();
	
	/* Disable I & F bit */ 
	sysSetLocalInterrupt(DISABLE_IRQ);		


}  