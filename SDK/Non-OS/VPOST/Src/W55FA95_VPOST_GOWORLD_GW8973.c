/***************************************************************************
 *                                                                         *
 * Copyright (c) 2007 - 2009 Nuvoton Technology Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     NUC930_VPOST_GOWORLD_GW8973.c
 *
 * VERSION
 *     0.1 
 *
 * DESCRIPTION
 *
 *
 *
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *
 *
 *     
 * HISTORY
 *     2009.03.16		Created by Shu-Ming Fan
 *
 *
 * REMARK
 *     None
 *
 *
 **************************************************************************/
#include "stdio.h"
#include "stdlib.h"
//#include "NUC930_VPOST_Regs.h"
//#include "w55fa93_vpost.h"
#include "w55fa95_vpost.h"

extern void LCDDelay(unsigned int nCount);

#if defined(HAVE_GOWORLD_GW8973)

static UINT32 g_nScreenWidth;
static UINT32 g_nScreenHeight;

#define _VERIFY_REAL_CHIP_
//#define _FPGA_

#ifdef _FPGA_
typedef enum 
{
	eEXT 	= 0,
	eX32K 	= 1,
	eAPLL  	= 2,
	eUPLL  	= 3
}E_CLK;


static UINT32 GetPLLOutputKhz(
	E_CLK eSysPll,
	UINT32 u32FinKHz
	)
{
	UINT32 u32Freq, u32PllCntlReg;
	UINT32 NF, NR, NO;
	
	UINT8 au8Map[4] = {1, 2, 2, 4};
	if(eSysPll==eSYS_APLL)
		u32PllCntlReg = inp32(REG_APLLCON);
	else if(eSysPll==eSYS_UPLL)	
		u32PllCntlReg = inp32(REG_UPLLCON);		
	
	NF = (u32PllCntlReg&FB_DV)+2;
	NR = ((u32PllCntlReg & IN_DV)>>9)+2;
	NO = au8Map[((u32PllCntlReg&OUT_DV)>>14)];
//	sysprintf("PLL regster = 0x%x\n", u32PllCntlReg);	
//	sysprintf("NF = %d\n", NF);
//	sysprintf("NR = %d\n", NR);
//	sysprintf("NOr = %d\n", NO);
		
	u32Freq = u32FinKHz*NF/NR/NO;
//	sysprintf("PLL Freq = %d\n", u32Freq);
	return u32Freq;
}
#endif

static void BacklightControl(int OnOff)
{	
	// GPA[11] set OUTPUT mode  => control the backlight
	outpw(REG_GPIOA_OMD, (inpw(REG_GPIOA_OMD) & 0x0000FFFF)| 0x00000800);
	if(OnOff==TRUE) {
		// GPA[11] turn on the backlight
		outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FFFF)| 0x00000800);
	} else {
		// GPA[11] diable backlight
		outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FFFF) & 0xFFFFF7FF);
	}
}

void GOWORLD_GW8973_Init(void)
{



}

static INT Clock_Control(void)
{
}

INT vpostLCMInit_GOWORLD_GW8973(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
{
	volatile S_DRVVPOST_SYNCLCM_WINDOW sWindow = {480,272,480};	
	volatile S_DRVVPOST_SYNCLCM_HTIMING sHTiming = {2,0x27,(UINT8)0x07};
	volatile S_DRVVPOST_SYNCLCM_VTIMING sVTiming = {2,0x8,0x09};
//	volatile S_DRVVPOST_SYNCLCM_POLARITY sPolarity = {FALSE,FALSE,TRUE,FALSE};
	volatile S_DRVVPOST_SYNCLCM_POLARITY sPolarity = {TRUE,TRUE,FALSE,FALSE};

	UINT32 nBytesPixel, u32PLLclk, u32ClockDivider;

//#define OPT_24BIT_MODE

	// VPOST clock control
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | VPOST_CKE | HCLK4_CKE);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | VPOSTRST);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~VPOSTRST);	
	
#ifdef _VERIFY_REAL_CHIP_
	u32PLLclk = sysGetPLLOutputHz(eSYS_UPLL, sysGetExternalClock());
	u32ClockDivider = u32PLLclk / 9000000;
	
	u32ClockDivider /= 2;
	u32ClockDivider--;
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N0) | 1);						
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N1) | ((u32ClockDivider & 0xFF) << 8));						
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_S);
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) | (3<<3));		// VPOST clock from UPLL		
#else
//	u32PLLclk = sysGetPLLOutputKhz(eSYS_UPLL, 27000);	// CLK_IN = 27 MHz
//	u32PLLclk = sysGetPLLOutputKhz(eSYS_UPLL, 12000);	// CLK_IN = 12 MHz
	u32PLLclk = GetPLLOutputKhz(eUPLL, 12000);			// CLK_IN = 12 MHz
	u32ClockDivider = u32PLLclk / 9000;
	
	u32ClockDivider /= 2;
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N0) | 1);						
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N1) | ((u32ClockDivider & 0xFF) << 8));						
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_S);
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) | (3<<3));		// VPOST clock from UPLL		
#endif

	vpostVAStopTrigger();	

	// Enable VPOST function pins
#ifdef	OPT_24BIT_MODE		
	vpostSetDataBusPin(eDRVVPOST_DATA_24BITS);
#else
	vpostSetDataBusPin(eDRVVPOST_DATA_16BITS);
//	vpostSetDataBusPin(eDRVVPOST_DATA_18BITS);	
#endif	
		  
	// LCD image source select
	vpostSetLCM_ImageSource(eDRVVPOST_FRAME_BUFFER);
	
	// configure LCD interface
	vpostSetLCM_TypeSelect(eDRVVPOST_HIGH_RESOLUTINO_SYNC);

	// configure LCD timing sync or async with TV timing	
	vpostsetLCM_TimingType(eDRVVPOST_ASYNC_TV);
	
    // Configure Parallel LCD interface (16/18/24-bit data bus)
#ifdef	OPT_24BIT_MODE		
    vpostSetParalelSyncLCM_Interface(eDRVVPOST_PRGB_24BITS);
#else    
    vpostSetParalelSyncLCM_Interface(eDRVVPOST_PRGB_16BITS);
//    vpostSetParalelSyncLCM_Interface(eDRVVPOST_PRGB_18BITS);    
#endif    
    
//	GOWORLD_GW8973_Init();
	
    // set Horizontal scanning line timing for Syn type LCD 
    vpostSetSyncLCM_HTiming(&sHTiming);

	// set Vertical scanning line timing for Syn type LCD   
    vpostSetSyncLCM_VTiming(&sVTiming);
	
	// set both "active pixel per line" and "active lines per screen" for Syn type LCD   
	vpostSetSyncLCM_ImageWindow(&sWindow);

  	// set Hsync/Vsync/Vden/Pclk poalrity
	vpostSetSyncLCM_SignalPolarity(&sPolarity);  	
    
#if 0
	vpostSetFrameBuffer_BaseAddress(pFramebuf);
#else    
    // set frambuffer base address
    if(pFramebuf != NULL) {
		vpostAllocVABufferFromAP(pFramebuf);
	} else {
    	if( vpostAllocVABuffer(plcdformatex, nBytesPixel)==FALSE)
    		return ERR_NULL_BUF;
    }
#endif    
	
	// set frame buffer data format
	vpostSetFrameBuffer_DataType(plcdformatex->ucVASrcFormat);
	
	vpostSetYUVEndianSelect(eDRVVPOST_YUV_LITTLE_ENDIAN);
	
	// enable LCD controller
	vpostVAStartTrigger();
	
//	BacklightControl(TRUE);			
	return 0;
}

INT32 vpostLCMDeinit_GOWORLD_GW8973(VOID)
{
	vpostVAStopTrigger();
	vpostFreeVABuffer();
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) & ~VPOST_CKE);	
	return 0;
}
#endif    //HAVE_GOWORLD_GW8973