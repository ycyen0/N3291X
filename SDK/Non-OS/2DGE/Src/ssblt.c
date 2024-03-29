/******************************************************************************
 *
 *  Copyright (c) 2014 by Nuvoton Technology Corp. All rights reserved.       
 *  Nuvoton N3291x 2D Graphics Engine Low-Level Library
 *
 *  FILENAME
 *      SSBLT.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to handle the screen-to-screen BLT operation.       
 * 
 *  HISTORY
 *      2014/12/01  Created.
 *
 *  REMARK
 *      None
 *                                                             
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wbio.h"
#include "wblib.h"
#include "w55fa95_gfx.h"
#include "global.h"

#include "w55fa95_reg.h"
#include "wberrcode.h"


#define PN  1   // Quadrant 1
#define NN  2   // Quadrant 2
#define NP  3   // Quadrant 3
#define PP  4   // Quadrant 4


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxScreenToScreenBlt()
//
//  DESCRIPTION
//      This function is used to do the screen-to-screen BLT.
//
//  INPUTS
//      src_rect:   source rectangle to be copied to the destination address
//      dest_pnt:   destination address (X/Y addressing)
//      
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error 
//
//  REMARK:
//      The hardware has problem with some ROP. The library must 
//      specially take care of those ROP code. 
//      Engine will not be ready for ROP = 0x00 or 0xff.
//
///////////////////////////////////////////////////////////////////////////////
INT gfxScreenToScreenBlt(GFX_RECT_T src_rect, GFX_PNT_T dest_pnt)
{
    INT direction;
    UINT32 srcx, srcy, destx, desty, width, height;
    UINT32 cmd32, dest_pitch, pitch, dest_start, src_start, dimension;
    UINT32 data32, alpha;
    GFX_RECT_T dest_rect;

    if ((_gfx_ucROP == 0x00) || (_gfx_ucROP == 0xff))
    {
        dest_rect.fC.nLeft = dest_pnt.nX;
        dest_rect.fC.nRight = dest_rect.fC.nLeft + (src_rect.fC.nRight - src_rect.fC.nLeft);
        dest_rect.fC.nTop = dest_pnt.nY;
        dest_rect.fC.nBottom = dest_rect.fC.nTop + (src_rect.fC.nBottom - src_rect.fC.nTop);
    
        if (_gfx_ucROP == 0x00)
            return gfxFillSolidRect(dest_rect, 0x000000);
        
        if (_gfx_ucROP == 0xff)
            return gfxFillSolidRect(dest_rect, 0xffffff);    
    }
    
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;
    
    if (! gfxIsValidRect(src_rect)) return ERR_GFX_INVALID_RECT;

    gfxWaitEngineReady();
    
    srcx = (UINT32)src_rect.fC.nLeft;
    srcy = (UINT32)src_rect.fC.nTop;
    destx = (UINT32)dest_pnt.nX;
    desty = (UINT32)dest_pnt.nY;
    width = (UINT32)(src_rect.fC.nRight - src_rect.fC.nLeft + 1);
    height = (UINT32)(src_rect.fC.nBottom - src_rect.fC.nTop + 1);
    
    cmd32 = 0x00430000 | ((UINT32)_gfx_ucROP << 24);

    /* normalization */
    
    if (srcx > destx) // +X
    {
        if (srcy > desty) // +Y
        {
            direction = PP; // 000
        } 
        else //-Y
        {
            direction= PN; // 100
            cmd32 |= 0x08; 
            srcy = srcy + height - 1;
            desty = desty + height - 1;
        }
    }      
    else // -X
    {     
        if (srcy > desty) // +Y
        {
            direction = NP;
            cmd32 |= 0x04; // 010
            srcx = srcx + width - 1;
            destx = destx + width - 1;
        }  
        else //-Y
        {
            direction = NN;
            cmd32 |= 0xc; // 110
            srcx = srcx + width - 1;
            destx = destx + width - 1;
            srcy = srcy + height - 1;
            desty = desty + height - 1;
        }
    }     

    if (_gfx_nPatternType == GFX_MONO_PATTERN)
    {
        cmd32 |= 0x00000010;
        outpw(REG_2D_GEFC, _gfx_uPatternForeColor);
        outpw(REG_2D_GEBC, _gfx_uPatternBackColor); 
    }
    
    outpw(REG_2D_GECMD, cmd32); 
  
    dest_pitch = (UINT32)(_gfx_nDestPitch / _gfx_nByte); // pitch in pixels
    pitch = dest_pitch << 16 | dest_pitch; 
    outpw(REG_2D_GESDP, pitch);

    src_start = srcy << 16 | srcx;
    outpw(REG_2D_GESSXYL, src_start);
  
    dest_start = desty << 16 | destx;
    outpw(REG_2D_GEDSXYL, dest_start);
  
    dimension = height << 16 | width;
    outpw(REG_2D_GEDIXYL, dimension);  
  
    //
    // force to use the same starting address
    //
    outpw(REG_2D_GEXYSOA, _gfx_uDestStartAddr);
  
    if (_gfx_bClipEnabled)
    {
        cmd32 |= 0x00000200;
        outpw(REG_2D_GECMD, cmd32);
        outpw(REG_2D_GECBTL, _gfx_uClipTL);
        outpw(REG_2D_GECBBR, _gfx_uClipBR);
    }  

    /* 
    ** Hardware can support destination transparency as well.
    ** Current implementation only supports source transparency.
    */
    if (_gfx_nDrawMode == GFX_SRC_TRANSPARENT)
    {
        if (! _gfx_bColorKeyEnabled)
            return ERR_GFX_COLOR_KEY_NOT_INITIALIZED;
            
        cmd32 |= 0x00008000; // source color transparency 
        outpw(REG_2D_GECMD, cmd32);
        outpw(REG_2D_GETC, _gfx_uColorKey);
        outpw(REG_2D_GETCM, _gfx_uColorKeyMask);
    }  
  
    if (_gfx_bAlphaEnabled)
    {
        cmd32 |= 0x00200000;
        outpw(REG_2D_GECMD, cmd32);
    
        data32 = inpw(REG_2D_GEMC) & 0x0000ffff;
        alpha = (_gfx_uAlphaKs << 8) | (256 - _gfx_uAlphaKs);
        data32 |= (alpha << 16);
    
        outpw(REG_2D_GEMC, data32);
    }
  
    outpw(REG_2D_GETRIG, 1); 

    return 0;
}


