/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved.  *
 *                                                              *
 ****************************************************************/
 
#include "string.h"
#include "stdlib.h"

#include "w55fa95_reg.h"
#include "w55fa95_spu.h"
#include "spu.h"

extern const N_table[32];
extern const sp_Digi_table[11];
extern const sp_Ana_table[11];
extern const ear_Digi_table[11];
extern const ear_Ana_table[11];

//#define DBG_PRINTF(...)
//#define DBG_PRINTF printf

/* buffer */
UINT8	*_pucPlayAudioBuff;

__align(256) UINT8 playbuffer[FRAG_SIZE];

void spuSwitchVolume(int uDirection)		// 1: for speaker -> earphone; 0: for earphone -> speaker
{
	int currentVolume, ii, jj;
	int ucVol_R, ucVol_L;
	int r_ucVol_R, r_ucVol_L;		

#ifdef __KLE_ONLY
    if (uDirection==0) 		// earphone --> speaker
    {
		ucVol_R = DrvSPU_ReadDACReg(0x0A);			    	
		for (ii= 0; ii<11; ii++)
		{
			if(ucVol_R >= ear_Digi_table[ii])
			{
				ucVol_R = 10 - ii;
				break;
			}			
		}		
	
		ucVol_L = DrvSPU_ReadDACReg(0x09);
		for (ii= 0; ii<11; ii++)
		{
			if(ucVol_L >= ear_Digi_table[ii])
			{
				ucVol_L = 10 - ii;
				break;
			}			
		}		
		
		r_ucVol_L = sp_Digi_table[10-ucVol_L];
		r_ucVol_R = sp_Digi_table[10-ucVol_R];
		DrvSPU_WriteDACReg(0x09, (UINT8)r_ucVol_L);
		DrvSPU_WriteDACReg(0x0A, (UINT8)r_ucVol_R);	

		r_ucVol_L = sp_Ana_table[10-ucVol_L];
		r_ucVol_R = sp_Ana_table[10-ucVol_R];
		DrvSPU_WriteDACReg(0x00, (UINT8)r_ucVol_L);
		DrvSPU_WriteDACReg(0x01, (UINT8)r_ucVol_R);	
	}
	else				// speaker --> earphone
	{
		ucVol_R = DrvSPU_ReadDACReg(0x0A);				
		for (ii= 0; ii<11; ii++)
		{
			if(ucVol_R >= sp_Digi_table[ii])
			{
				ucVol_R = 10 - ii;
				break;
			}			
		}		
	
		ucVol_L = DrvSPU_ReadDACReg(0x09);
		for (ii= 0; ii<11; ii++)
		{
			if(ucVol_L >= sp_Digi_table[ii])
			{
				ucVol_L = 10 - ii;
				break;
			}			
		}		
		
		r_ucVol_L = ear_Digi_table[10-ucVol_L];
		r_ucVol_R = ear_Digi_table[10-ucVol_R];
		DrvSPU_WriteDACReg(0x09, (UINT8)r_ucVol_L);
		DrvSPU_WriteDACReg(0x0A, (UINT8)r_ucVol_R);	

		r_ucVol_L = ear_Ana_table[10-ucVol_L];
		r_ucVol_R = ear_Ana_table[10-ucVol_R];
		DrvSPU_WriteDACReg(0x00, (UINT8)r_ucVol_L);
		DrvSPU_WriteDACReg(0x01, (UINT8)r_ucVol_R);	
	}		    	

#else
	jj = 0xff;
	currentVolume = DrvSPU_ReadDACReg(0x0A);		
	
	for (ii=0; ii<32; ii++)
	{
		if (currentVolume >= N_table[ii])
		{
			jj = ii;
			break;
		}
	}
	
	if (jj==0xff)
		jj = 0;
			
	switch(uDirection)
	{
		case 0:			// for earphone -> speaker
			DrvSPU_WriteDACReg(0x00, 1);		// -2dB
			DrvSPU_WriteDACReg(0x01, 1);
			jj++;
			if (jj==32)
				jj = 31;	

			break;		
		
		case 1:			// for speaker -> earphone
		default:
			DrvSPU_WriteDACReg(0x00, 3);		// -6dB
			DrvSPU_WriteDACReg(0x01, 3);
			if (jj!=0)
				jj--;;	
	
			break;
	}		

	DrvSPU_WriteDACReg(0x09, N_table[jj]);
	DrvSPU_WriteDACReg(0x0A, N_table[jj]);
#endif	
}

void spuSetSampleRate(UINT32 u32SampleRate)
{	
	DrvSPU_SetSampleRate(u32SampleRate);
}

void spuSetBaseAddress(UINT32 u32BaseAddress)
{
	DrvSPU_SetBaseAddress(0, u32BaseAddress);	
	DrvSPU_SetBaseAddress(1, u32BaseAddress);	
}

UINT32 spuGetBaseAddress(void)
{
	UINT32 u32Address;
	
	DrvSPU_GetBaseAddress(0, &u32Address);
	return u32Address;
}

void spuSetThresholdAddress(UINT32 u32TH1Address)
{
	DrvSPU_SetThresholdAddress(0, u32TH1Address);	
	DrvSPU_SetThresholdAddress(1, u32TH1Address);	
}

UINT32 spuGetThresholdAddress(void)
{
	UINT32 u32Address;
	
	DrvSPU_GetThresholdAddress(0,&u32Address);
	return u32Address;	
}

void spuSetEndAddress(UINT32 u32TH2Address)
{
	DrvSPU_SetEndAddress(0, u32TH2Address);	
	DrvSPU_SetEndAddress(1, u32TH2Address);
}

UINT32 spuGetEndAddress(void)
{
	UINT32 u32Address;
	
	DrvSPU_GetEndAddress(0,&u32Address);
	return u32Address;	
}

UINT32 spuGetCurrentAddress(void)
{
	UINT32 u32Address;
	
	DrvSPU_GetCurrentAddress(0,&u32Address);
	return u32Address;		
}

void spuSetStereo(void)
{
	DrvSPU_SetSrcType(0, eDRVSPU_STEREO_PCM16_LEFT);		// Stereo PCM16 left	
	DrvSPU_SetSrcType(1, eDRVSPU_STEREO_PCM16_RIGHT);		// Stereo PCM16 right	
}

void spuSetMono(void)
{
	DrvSPU_SetSrcType(0, eDRVSPU_MONO_PCM16);		// Mono PCM16	
}

BOOL spuIsMono(void)
{
	UINT16 u16SrcType;
	
	DrvSPU_GetSrcType(0, &u16SrcType);

	if (u16SrcType == eDRVSPU_MONO_PCM16)
		return TRUE;
		
	return FALSE;				
}

void spuSetVolume(UINT16 u16CHRVolume, UINT16 u16CHLVolume)
{
#ifdef __KLE_ONLY
	volatile int ucLeftVol, ucRightVol;
	volatile int r_ucLeftVol, r_ucRightVol;	

	if(u16CHRVolume >=100)
		u16CHRVolume = 100;

	if(u16CHLVolume >=100)
		u16CHLVolume = 100;
		
	ucLeftVol = u16CHLVolume/10;
	ucRightVol = u16CHRVolume/10;

    if (inp32(REG_GPIOG_PIN) & 0x0080) 		// GPIOG_7, headset plugged in
    {			
		r_ucLeftVol = ear_Digi_table[10-ucLeftVol];
		r_ucRightVol = ear_Digi_table[10-ucRightVol];
		DrvSPU_WriteDACReg(0x09, (UINT8)r_ucLeftVol);
		DrvSPU_WriteDACReg(0x0A, (UINT8)r_ucRightVol);	

		r_ucLeftVol = ear_Ana_table[10-ucLeftVol];
		r_ucRightVol = ear_Ana_table[10-ucRightVol];
		DrvSPU_WriteDACReg(0x00, (UINT8)r_ucLeftVol);
		DrvSPU_WriteDACReg(0x01, (UINT8)r_ucRightVol);	
	}
	else					// headset plugged out
	{
		r_ucLeftVol = sp_Digi_table[10-ucLeftVol];
		r_ucRightVol = sp_Digi_table[10-ucRightVol];
		DrvSPU_WriteDACReg(0x09, (UINT8)r_ucLeftVol);
		DrvSPU_WriteDACReg(0x0A, (UINT8)r_ucRightVol);	

		r_ucLeftVol = sp_Ana_table[10-ucLeftVol];
		r_ucRightVol = sp_Ana_table[10-ucRightVol];
		DrvSPU_WriteDACReg(0x00, (UINT8)r_ucLeftVol);
		DrvSPU_WriteDACReg(0x01, (UINT8)r_ucRightVol);	
	}		
#else	

	if(u16CHRVolume >=100)
		u16CHRVolume = 100;

	if(u16CHLVolume >=100)
		u16CHLVolume = 100;

//	sysprint("u16CHRVolume = 0x%x, u16CHLVolume = 0x%x !!!", u16CHRVolume, u16CHLVolume);
	u16CHRVolume = spuNormalizeVolume(u16CHRVolume);	
	DrvSPU_WriteDACReg(0x0A, (u16CHRVolume & 0xFF));	
	u16CHLVolume = spuNormalizeVolume(u16CHLVolume);		
	DrvSPU_WriteDACReg(0x09, (u16CHLVolume & 0xFF));
//	sysprint("Normalized u16CHRVolume = 0x%x, u16CHLVolume = 0x%x !!!", u16CHRVolume, u16CHLVolume);	
#endif
}

static UINT8 spuGetVolume(UINT8 u8Channel)
{
	UINT8 u8Volume;
	
	DrvSPU_GetChannelVolume(u8Channel,&u8Volume);
	return u8Volume;
}

void spuSetPowerDown(UINT16 u16PowerDown)
{
	UINT8 u8Ret;
	
	u8Ret = u16PowerDown & 0xFF;
	DrvSPU_WriteDACReg(0x05, u8Ret);	
}

UINT16 spuGetPowerDown(void)
{
	UINT8 u8Ret;

	u8Ret = DrvSPU_ReadDACReg(0x05);
	return u8Ret;
}

UINT8 DacOnOffLevel;

//must use this function before calling spuStartPlay()
VOID spuDacOn(UINT8 level)
{
	DacOnOffLevel = level;	

	outp32(REG_SPU_CLK_PAR, inp32(REG_SPU_CLK_PAR) & ~DAC_RST);
	sysDelay(1);		
	outp32(REG_SPU_CLK_PAR, inp32(REG_SPU_CLK_PAR) |  DAC_RST);	
	sysDelay(2);			


	DrvSPU_WriteDACReg(0x00, 0x1F);
	
	DrvSPU_WriteDACReg(0x01, 0x1F);	

	DrvSPU_WriteDACReg(0x07, 0x01);	

	DrvSPU_WriteDACReg(0x09, 0x64);	

	DrvSPU_WriteDACReg(0x0A, 0x64);	

	DrvSPU_WriteDACReg(0x05, 0xDF);	

	//sysDelay(100);
/*
	DrvSPU_WriteDACReg(0x05, 0x13);	

	DrvSPU_WriteDACReg(0x05, 0x10);	

	DrvSPU_WriteDACReg(0x00, 0x00);	

	DrvSPU_WriteDACReg(0x01, 0x00);	

	DrvSPU_WriteDACReg(0x08, 0x01);	
*/
}

//must use this function after calling spuStopPlay()
VOID spuDacOff(VOID)
{
	DrvSPU_WriteDACReg(0x00, 0x1F);	

	DrvSPU_WriteDACReg(0x01, 0x1F);	

	sysDelay(50);

	DrvSPU_WriteDACReg(0x05, 0xFF);	

}

VOID spuSetDacSlaveMode(VOID)
{
	DrvSPU_WriteDACReg(0x05, 0x13);	

	DrvSPU_WriteDACReg(0x05, 0x10);	

	DrvSPU_WriteDACReg(0x00, 0x00);	

	DrvSPU_WriteDACReg(0x01, 0x00);	

	DrvSPU_WriteDACReg(0x08, 0x01);
	
#if 1
    	DrvSPU_WriteDACReg(0x07, 0x05);	// "Stereo -> Mono -> Stereo" can do software reset
    	
    	DrvSPU_WriteDACReg(0x07, 0x01);		

#endif     	
	
}

VOID spuStartPlay(PFN_DRVSPU_CB_FUNC *fnCallBack, UINT8 *data)
{	
	DrvSPU_EnableInt(eDRVSPU_CHANNEL_0, DRVSPU_THADDRESS_INT, (PFN_DRVSPU_CB_FUNC*) fnCallBack);
	DrvSPU_EnableInt(eDRVSPU_CHANNEL_0, DRVSPU_ENDADDRESS_INT, (PFN_DRVSPU_CB_FUNC*) fnCallBack);	
	
	memcpy(playbuffer, data, FRAG_SIZE);

	DrvSPU_StartPlay();	
}

VOID spuStopPlay(VOID)
{
	int ii;     
	
	for (ii=0; ii<32; ii++)
	{
		DrvSPU_DisableInt(ii, DRVSPU_ENDADDRESS_INT); 
		DrvSPU_DisableInt(ii, DRVSPU_THADDRESS_INT);
	}
	
	DrvSPU_StopPlay();
	sysDisableInterrupt(IRQ_SPU);
}

VOID spuIoctl(UINT32 cmd, UINT32 arg0, UINT32 arg1)
{
	switch(cmd)
	{		
		case SPU_IOCTL_SET_VOLUME:
			spuSetVolume(arg0, arg1);
			break;
			
		case SPU_IOCTL_SET_MONO:
			spuSetMono();
			break;
			
		
		case SPU_IOCTL_SET_STEREO:
			spuSetStereo();
			break;			
			
		case SPU_IOCTL_GET_FRAG_SIZE:
			*((UINT32 *)arg0) = FRAG_SIZE;
			break;
			
		default:
			break;
	}		
}
static void delay(UINT32 kk)
{
	UINT32 ii, jj;
	
	for(ii=0; ii < kk; ii++)
	{
		for(jj=0; jj < 0x10; jj++);	
	}
}

VOID spuOpen(UINT32 u32SampleRate)
{	
	_pucPlayAudioBuff = (UINT8 *)((UINT32)playbuffer | 0x80000000);
	memset(_pucPlayAudioBuff, 0, FRAG_SIZE);

#if 0
	// reset SPU DAC
	outp32(REG_SPU_CLK_PAR, inp32(REG_SPU_CLK_PAR) & ~DAC_RST);		
	delay(10000);
	outp32(REG_SPU_CLK_PAR, inp32(REG_SPU_CLK_PAR) | DAC_RST);			
	delay(1000);	
//	outp32(REG_SPU_CLK_PAR, inp32(REG_SPU_CLK_PAR) & ~DAC_RST);		
//	delay(1000);
//	outp32(REG_SPU_CLK_PAR, inp32(REG_SPU_CLK_PAR) | DAC_RST);			
#endif

	outp32(REG_AHBCLK, inp32(REG_AHBCLK) | ADO_CKE | SPU_CKE | HCLK4_CKE);			// enable SPU engine clock 

	spuStopPlay();	// SPU must be disabled before to enable again	

	// 1.Check I/O pins. If I/O pins are used by other IPs, return error code.
	// 2.Enable IP��s clock
	// 3.Reset IP			
	
	DrvSPU_Open();	
	
	DrvSPU_SetPAN(0, 0x1f1f);
	DrvSPU_SetPAN(1, 0x1f1f);
	
	DrvSPU_SetChannelVolume(0, 0x4F);
	DrvSPU_SetChannelVolume(1, 0x4F);
	
	DrvSPU_ChannelOpen(0);	
	DrvSPU_ChannelOpen(1);	

	DrvSPU_SetSampleRate(u32SampleRate);

	DrvSPU_SetSrcType(0, eDRVSPU_STEREO_PCM16_LEFT);	
	DrvSPU_SetSrcType(1, eDRVSPU_STEREO_PCM16_RIGHT);

	DrvSPU_SetDFA(0, 0x400);
	DrvSPU_SetDFA(1, 0x400);

	spuSetBaseAddress((UINT32)_pucPlayAudioBuff);
	spuSetThresholdAddress((UINT32)_pucPlayAudioBuff + HALF_FRAG_SIZE);	
	spuSetEndAddress((UINT32)_pucPlayAudioBuff + FRAG_SIZE);

	sysInstallISR(IRQ_LEVEL_1, IRQ_SPU, (PVOID)DrvSPU_IntHandler);	
	sysSetLocalInterrupt(ENABLE_IRQ);
	sysEnableInterrupt(IRQ_SPU);
}

VOID spuClose (VOID)
{
	sysDisableInterrupt(IRQ_SPU);
	DrvSPU_Close();
}

VOID spuEqOpen (E_DRVSPU_EQ_BAND eEqBand, E_DRVSPU_EQ_GAIN eEqGain)
{
	DrvSPU_EqOpen(eEqBand, eEqGain);
}

VOID spuEqClose (VOID)
{
	DrvSPU_EqClose();
}



