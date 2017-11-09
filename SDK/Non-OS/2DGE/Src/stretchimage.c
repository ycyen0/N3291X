/******************************************************************************
 *
 *  Copyright (c) 2014 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond N3291x Graphics Low-Level Library
 *
 *  FILENAME
 *      STRETCHIMAGE.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This module is used to scale a bitmap from source surface 
 *      to destination surface.        
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
#include "w55fa95_gfx.h"
#include "global.h"

#include "w55fa95_reg.h"
#include "wberrcode.h"


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxStretchImage()
//
//  DESCRIPTION
//      This function is used to scale a selected bitmap rectangle
//      from source surface to a specified rectangle of destination surface. 
//
//  INPUTS
//      src_rect:   source rectangle
//      dest_rect:  destination rectangle 
//      
//  OUTPUTS
//      None
//
//  RETURN
//      0:      success
//      others: error 
//
///////////////////////////////////////////////////////////////////////////////
INT gfxStretchImage(GFX_RECT_T src_rect, GFX_RECT_T dest_rect)
{
    UINT32 sx, sy, dx, dy, src_width, src_height, dest_width, dest_height;
    UINT32 cmd32, src_pitch, dest_pitch, pitch, dest_start, src_start, dimension;
    UINT32 data32, alpha;
    UINT32 tile_ctl;

    if ((_gfx_ucROP == 0x00) || (_gfx_ucROP == 0xff))
    {
        if (_gfx_ucROP == 0x00)
            return gfxFillSolidRect(dest_rect, 0x000000);
        
        if (_gfx_ucROP == 0xff)
            return gfxFillSolidRect(dest_rect, 0xffffff);    
    }
        
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;

#if 0 // can't call this because it is memory buffer    
    if (! gfxIsValidRect(src_rect)) return ERR_GFX_INVALID_RECT;
#endif
    
    gfxWaitEngineReady();
    
#if 0    
    //
    // add reset FIFO
    //
    temp32 = inpw(REG_GE_MISC);
    outpw(REG_GE_MISC, temp32 | 0x00000040); // FIFO reset
    outpw(REG_GE_MISC, temp32);     
#endif

    sx = (UINT32)src_rect.fC.nLeft;
    sy = (UINT32)src_rect.fC.nTop;
    src_width = (UINT32)(src_rect.fC.nRight - src_rect.fC.nLeft + 1);
    src_height = (UINT32)(src_rect.fC.nBottom - src_rect.fC.nTop + 1);
    
    dx = (UINT32)dest_rect.fC.nLeft;
    dy = (UINT32)dest_rect.fC.nTop;
    dest_width = (UINT32)(dest_rect.fC.nRight - dest_rect.fC.nLeft + 1);
    dest_height = (UINT32)(dest_rect.fC.nBottom - dest_rect.fC.nTop + 1);    

#if 0
    cmd32 = 0x00030000 | ((UINT32)_gfx_ucROP << 24);
    
    if (_gfx_nPatternType == GFX_MONO_PATTERN)
    {
        cmd32 |= 0x00000010;
        outpw(REG_2D_GEFC, _gfx_uPatternForeColor);
        outpw(REG_2D_GEBC, _gfx_uPatternBackColor); 
    }
#else
    cmd32 = 0xcc030000;
#endif
        
    outpw(REG_2D_GECMD, cmd32);

    src_pitch = (UINT32)(_gfx_nSrcPitch / _gfx_nByte); // pitch in pixels
    dest_pitch = (UINT32)(_gfx_nDestPitch / _gfx_nByte); // pitch in pixels

    pitch = dest_pitch << 16 | src_pitch; // pitch in pixel
    outpw(REG_2D_GESDP, pitch);

    outpw(REG_2D_GEXYSOA, _gfx_uSrcStartAddr);
  
    src_start = sy << 16 | sx;
    outpw(REG_2D_GESSXYL, src_start);
  
    dest_start = dy << 16 | dx;
    outpw(REG_2D_GEDSXYL, dest_start);
  
    dimension = src_height << 16 | src_width;
    outpw(REG_2D_GEDIXYL, dimension);  
  
    if (_gfx_bClipEnabled)
    {
        cmd32 |= 0x00000200;
        outpw(REG_2D_GECMD, cmd32);
        outpw(REG_2D_GECBTL, _gfx_uClipTL);
        outpw(REG_2D_GECBBR, _gfx_uClipBR);
    }  

#if 0
    if (_gfx_bAlphaEnabled)
    {
        //
        // NOTE:
        //   Hardware doesn't allow pattern with alpha blending.
        //
        #if 0
        cmd32 = 0xcc430000; // only support SRCCOPY
        cmd32 |= 0x00200000;
        #else
        cmd32 = 0xcc230000; 
        #endif 
        outpw(REG_2D_GECMD, cmd32);
    
        data32 = inpw(REG_2D_GEMC) & 0x0000ffff;
        alpha = _gfx_uAlphaKs << 8 | (256 - _gfx_uAlphaKs);
        data32 |= (alpha << 16);
    
        outpw(REG_2D_GEMC, data32);
    }
#endif     
        
    if (_gfx_bColorKeyEnabled)
    {
        cmd32 |= 0x00008000; // color transparency 
        outpw(REG_2D_GECMD, cmd32);
        outpw(REG_2D_GETC, _gfx_uColorKey);
        outpw(REG_2D_GETCM, _gfx_uColorKeyMask);
    }
  
    /* 
    ** The tile height and width are changed to CR0008 bit 15 to b0.
    */
    tile_ctl = (((_gfx_uTileY-1) << 16) | (_gfx_uTileX-1));
    outpw(REG_2D_TILEXY, tile_ctl);     
   
#if 0    
    if (tile_ctl != 0) // not 1x1
    {
        cmd32 |= 0x0000400; // b10 controls the tile option
        outpw(REG_2D_GECMD, cmd32);
    }
#endif 


    //
    // set to 0xCC if alpha is enabled 
    //
    if (_gfx_bAlphaEnabled)
    {
        cmd32 = (cmd32 & 0x001fffff) | 0xcc200000; // force to 0xCC 
        //cmd32 = 0xcc230000; // 
 
        outpw(REG_2D_GECMD, cmd32);
    
        data32 = inpw(REG_2D_GEMC) & 0x0000ffff;
        alpha = _gfx_uAlphaKs << 8 | (256 - _gfx_uAlphaKs);
        data32 |= (alpha << 16);
    
        outpw(REG_2D_GEMC, data32);
    }
    
    //
    // set scaling factor 
    //
    outpw(REG_2D_VSF, dest_height << 16 | src_height);
    outpw(REG_2D_HSF, dest_width << 16 | src_width); 

     
    outpw(REG_2D_GETRIG, 1);     

    return GFX_OK;
}

