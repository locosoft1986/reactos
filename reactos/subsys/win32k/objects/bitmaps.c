/*
 *  ReactOS W32 Subsystem
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 ReactOS Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/* $Id: bitmaps.c,v 1.46 2003/12/04 21:35:11 fireball Exp $ */
#undef WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <win32k/color.h>
#include <win32k/gdiobj.h>
#include <win32k/bitmaps.h>
#include <win32k/brush.h>
//#include <win32k/debug.h>
#include "../eng/handle.h"
#include <include/inteng.h>
#include <include/eng.h>
#include <include/error.h>
#include <include/surface.h>
#include <include/palette.h>

#define NDEBUG
#include <win32k/debug1.h>

BOOL STDCALL NtGdiBitBlt(HDC  hDCDest,
                 INT  XDest,
                 INT  YDest,
                 INT  Width,
                 INT  Height,
                 HDC  hDCSrc,
                 INT  XSrc,
                 INT  YSrc,
                 DWORD  ROP)
{
  PDC DCDest = NULL;
  PDC DCSrc  = NULL;
  PSURFOBJ SurfDest, SurfSrc;
  PSURFGDI SurfGDIDest, SurfGDISrc;
  RECTL DestRect;
  POINTL SourcePoint;
  BOOL Status;
  PPALGDI PalDestGDI, PalSourceGDI;
  PXLATEOBJ XlateObj = NULL;
  HPALETTE SourcePalette, DestPalette;
  ULONG SourceMode, DestMode;
  PBRUSHOBJ BrushObj;
  BOOL UsesSource = ((ROP & 0xCC0000) >> 2) != (ROP & 0x330000);
  BOOL UsesPattern = TRUE;//((ROP & 0xF00000) >> 4) != (ROP & 0x0F0000);  

  DCDest = DC_LockDc(hDCDest);
  if (NULL == DCDest)
    {
      DPRINT1("Invalid destination dc handle (0x%08x) passed to NtGdiBitBlt\n", hDCDest);
      SetLastWin32Error(ERROR_INVALID_HANDLE);
      return FALSE;
    }

  if (UsesSource)
    {
      if (hDCSrc != hDCDest)
        {
          DCSrc = DC_LockDc(hDCSrc);
          if (NULL == DCSrc)
            {
              DC_UnlockDc(hDCDest);
              DPRINT1("Invalid source dc handle (0x%08x) passed to NtGdiBitBlt\n", hDCSrc);
              SetLastWin32Error(ERROR_INVALID_HANDLE);
              return FALSE;
            }
        }
      else
        {
           DCSrc = DCDest;
        }
    }
  else
    {
      DCSrc = NULL;
    }

  /* Offset the destination and source by the origin of their DCs. */
  XDest += DCDest->w.DCOrgX;
  YDest += DCDest->w.DCOrgY;
  if (UsesSource)
    {
      XSrc += DCSrc->w.DCOrgX;
      YSrc += DCSrc->w.DCOrgY;
    }

  DestRect.left   = XDest;
  DestRect.top    = YDest;
  DestRect.right  = XDest+Width;
  DestRect.bottom = YDest+Height;

  SourcePoint.x = XSrc;
  SourcePoint.y = YSrc;

  /* Determine surfaces to be used in the bitblt */
  SurfDest = (PSURFOBJ)AccessUserObject((ULONG)DCDest->Surface);
  SurfGDIDest = (PSURFGDI)AccessInternalObjectFromUserObject(SurfDest);
  if (UsesSource)
    {
      SurfSrc  = (PSURFOBJ)AccessUserObject((ULONG)DCSrc->Surface);
      SurfGDISrc  = (PSURFGDI)AccessInternalObjectFromUserObject(SurfSrc);
    }
  else
    {
      SurfSrc  = NULL;
      SurfGDISrc  = NULL;
    }

  if (UsesPattern)
    {
      BrushObj = BRUSHOBJ_LockBrush(DCDest->w.hBrush);
      if (NULL == BrushObj)
        {
          if (UsesSource && hDCSrc != hDCDest)
            {
              DC_UnlockDc(hDCSrc);
            }
          DC_UnlockDc(hDCDest);
          SetLastWin32Error(ERROR_INVALID_HANDLE);
          return FALSE;
        }
    }
  else
    {
      BrushObj = NULL;
    }

  if (DCDest->w.hPalette != 0)
    {
      DestPalette = DCDest->w.hPalette;
    }
  else
    {
      DestPalette = NtGdiGetStockObject(DEFAULT_PALETTE);
    }

  if (UsesSource && DCSrc->w.hPalette != 0)
    {
      SourcePalette = DCSrc->w.hPalette;
    }
  else
    {
      SourcePalette = NtGdiGetStockObject(DEFAULT_PALETTE);
    }

  PalSourceGDI = PALETTE_LockPalette(SourcePalette);
  if (NULL == PalSourceGDI)
    {
      if (UsesSource && hDCSrc != hDCDest)
        {
          DC_UnlockDc(hDCSrc);
        }
      DC_UnlockDc(hDCDest);
      SetLastWin32Error(ERROR_INVALID_HANDLE);
      return FALSE;
    }
  SourceMode = PalSourceGDI->Mode;
  PALETTE_UnlockPalette(SourcePalette);

  if (DestPalette == SourcePalette)
    {
      DestMode = SourceMode;
    }
  else
    {
      PalDestGDI = PALETTE_LockPalette(DestPalette);
      if (NULL == PalDestGDI)
        {
          if (UsesSource && hDCSrc != hDCDest)
            {
              DC_UnlockDc(hDCSrc);
            }
          DC_UnlockDc(hDCDest);
          SetLastWin32Error(ERROR_INVALID_HANDLE);
          return FALSE;
        }
      DestMode = PalDestGDI->Mode;
      PALETTE_UnlockPalette(DestPalette);
    }

  XlateObj = (PXLATEOBJ)IntEngCreateXlate(DestMode, SourceMode, DestPalette, SourcePalette);
  if (NULL == XlateObj)
    {
      if (UsesSource && hDCSrc != hDCDest)
        {
          DC_UnlockDc(hDCSrc);
        }
      DC_UnlockDc(hDCDest);
      SetLastWin32Error(ERROR_NO_SYSTEM_RESOURCES);
      return FALSE;
    }

  /* Perform the bitblt operation */
  Status = IntEngBitBlt(SurfDest, SurfSrc, NULL, DCDest->CombinedClip, XlateObj,
                        &DestRect, &SourcePoint, NULL, BrushObj, NULL, ROP);

  EngDeleteXlate(XlateObj);
  if (UsesPattern)
    {
      BRUSHOBJ_UnlockBrush(DCDest->w.hBrush);
    }
  if (UsesSource && hDCSrc != hDCDest)
    {
      DC_UnlockDc(hDCSrc);
    }
  DC_UnlockDc(hDCDest);

  return Status;
}

HBITMAP STDCALL NtGdiCreateBitmap(INT  Width,
                          INT  Height,
                          UINT  Planes,
                          UINT  BitsPerPel,
                          CONST VOID *Bits)
{
  PBITMAPOBJ  bmp;
  HBITMAP  hBitmap;

  Planes = (BYTE) Planes;
  BitsPerPel = (BYTE) BitsPerPel;

  /* Check parameters */
  if (!Height || !Width)
  {
    return 0;
  }
  if (Planes != 1)
  {
    UNIMPLEMENTED;
    return  0;
  }
  if (Height < 0)
  {
    Height = -Height;
  }
  if (Width < 0)
  {
    Width = -Width;
  }

  /* Create the BITMAPOBJ */
  hBitmap = BITMAPOBJ_AllocBitmap ();
  if (!hBitmap)
  {
	DPRINT("NtGdiCreateBitmap: BITMAPOBJ_AllocBitmap returned 0\n");
    return 0;
  }

  bmp = BITMAPOBJ_LockBitmap( hBitmap );

  DPRINT("NtGdiCreateBitmap:%dx%d, %d (%d BPP) colors returning %08x\n", Width, Height,
         1 << (Planes * BitsPerPel), BitsPerPel, bmp);

  bmp->size.cx = Width;
  bmp->size.cy = Height;
  bmp->bitmap.bmType = 0;
  bmp->bitmap.bmWidth = Width;
  bmp->bitmap.bmHeight = Height;
  bmp->bitmap.bmPlanes = Planes;
  bmp->bitmap.bmBitsPixel = BitsPerPel;
  bmp->bitmap.bmWidthBytes = BITMAPOBJ_GetWidthBytes (Width, BitsPerPel);
  bmp->bitmap.bmBits = NULL;
  bmp->DDBitmap = NULL;
  bmp->dib = NULL;

  // Allocate memory for bitmap bits
  bmp->bitmap.bmBits = ExAllocatePool(PagedPool, bmp->bitmap.bmWidthBytes * bmp->bitmap.bmHeight);

  BITMAPOBJ_UnlockBitmap( hBitmap );

  if (Bits) /* Set bitmap bits */
  {
    NtGdiSetBitmapBits(hBitmap, Height * bmp->bitmap.bmWidthBytes, Bits);
  }

  return  hBitmap;
}

BOOL FASTCALL Bitmap_InternalDelete( PBITMAPOBJ pBmp )
{
  ASSERT( pBmp );

  if (NULL != pBmp->bitmap.bmBits)
    {
      if (NULL != pBmp->dib)
	{
	  if (NULL == pBmp->dib->dshSection)
	    {
	      EngFreeUserMem(pBmp->bitmap.bmBits);
	    }
	  else
	    {
	      /* This is a file-mapped section */
	      UNIMPLEMENTED;
	    }
	}
      else
	{
	  ExFreePool(pBmp->bitmap.bmBits);
	}
    }

  return TRUE;
}


HBITMAP STDCALL NtGdiCreateCompatibleBitmap(HDC hDC,
                                    INT  Width,
                                    INT  Height)
{
  HBITMAP  hbmpRet;
  PDC  dc;

  hbmpRet = 0;
  dc = DC_LockDc(hDC);

  DPRINT("NtGdiCreateCompatibleBitmap(%04x,%d,%d, bpp:%d) = \n", hDC, Width, Height, dc->w.bitsPerPixel);

  if (!dc)
  {
    return 0;
  }
  if ((Width >= 0x10000) || (Height >= 0x10000))
  {
    DPRINT("got bad width %d or height %d, please look for reason\n", Width, Height);
  }
  else
  {
    /* MS doc says if width or height is 0, return 1-by-1 pixel, monochrome bitmap */
    if (!Width || !Height)
    {
      hbmpRet = NtGdiCreateBitmap (1, 1, 1, 1, NULL);
    }
    else
    {
      hbmpRet = NtGdiCreateBitmap(Width, Height, 1, dc->w.bitsPerPixel, NULL);
    }
  }
  DPRINT ("\t\t%04x\n", hbmpRet);
  DC_UnlockDc( hDC );
  return hbmpRet;
}

HBITMAP STDCALL NtGdiCreateBitmapIndirect(CONST BITMAP  *BM)
{
  return NtGdiCreateBitmap (BM->bmWidth,
                           BM->bmHeight,
                           BM->bmPlanes,
                           BM->bmBitsPixel,
                           BM->bmBits);
}

HBITMAP STDCALL NtGdiCreateDiscardableBitmap(HDC  hDC,
                                     INT  Width,
                                     INT  Height)
{
  /* FIXME: this probably should do something else */
  return  NtGdiCreateCompatibleBitmap(hDC, Width, Height);
}

BOOL STDCALL NtGdiExtFloodFill(HDC  hDC,
                      INT  XStart,
                      INT  YStart,
                      COLORREF  Color,
                      UINT  FillType)
{
  UNIMPLEMENTED;
}

BOOL STDCALL NtGdiFloodFill(HDC  hDC,
                    INT  XStart,
                    INT  YStart,
                    COLORREF  Fill)
{
  UNIMPLEMENTED;
}

BOOL STDCALL NtGdiGetBitmapDimensionEx(HBITMAP  hBitmap,
                               LPSIZE  Dimension)
{
  PBITMAPOBJ  bmp;

  bmp = BITMAPOBJ_LockBitmap(hBitmap);
  if (bmp == NULL)
  {
    return FALSE;
  }

  *Dimension = bmp->size;

  BITMAPOBJ_UnlockBitmap(hBitmap);

  return  TRUE;
}

COLORREF STDCALL NtGdiGetPixel(HDC  hDC,
                       INT  XPos,
                       INT  YPos)
{
  PDC      dc = NULL;
  COLORREF cr = (COLORREF) 0;

  dc = DC_LockDc (hDC);
  if (NULL == dc)
  {
    return (COLORREF) CLR_INVALID;
  }
  //FIXME: get actual pixel RGB value
  DC_UnlockDc (hDC);
  return cr;
}

/***********************************************************************
 * MaskBlt
 * Ported from WINE by sedwards 11-4-03
 * 
 * Someone thought it would be faster to do it here and then switch back 
 * to GDI32. I dunno. Write a test and let me know. 
 */

static inline BYTE SwapROP3_SrcDst(BYTE bRop3)
{
    return (bRop3 & 0x99) | ((bRop3 & 0x22) << 1) | ((bRop3 & 0x44) >> 1);
}

#define FRGND_ROP3(ROP4)	((ROP4) & 0x00FFFFFF)
#define BKGND_ROP3(ROP4)	(ROP3Table[(SwapROP3_SrcDst((ROP4)>>24)) & 0xFF])
#define DSTCOPY 		0x00AA0029
#define DSTERASE		0x00220326 /* dest = dest & (~src) : DSna */

BOOL STDCALL NtGdiMaskBlt(HDC hdcDest, INT nXDest, INT nYDest,
                        INT nWidth, INT nHeight, HDC hdcSrc,
			INT nXSrc, INT nYSrc, HBITMAP hbmMask,
			INT xMask, INT yMask, DWORD dwRop)
{
    HBITMAP hOldMaskBitmap, hBitmap2, hOldBitmap2, hBitmap3, hOldBitmap3;
    HDC hDCMask, hDC1, hDC2;
    static const DWORD ROP3Table[256] = 
    {
        0x00000042, 0x00010289,
        0x00020C89, 0x000300AA,
        0x00040C88, 0x000500A9,
        0x00060865, 0x000702C5,
        0x00080F08, 0x00090245,
        0x000A0329, 0x000B0B2A,
        0x000C0324, 0x000D0B25,
        0x000E08A5, 0x000F0001,
        0x00100C85, 0x001100A6,
        0x00120868, 0x001302C8,
        0x00140869, 0x001502C9,
        0x00165CCA, 0x00171D54,
        0x00180D59, 0x00191CC8,
        0x001A06C5, 0x001B0768,
        0x001C06CA, 0x001D0766,
        0x001E01A5, 0x001F0385,
        0x00200F09, 0x00210248,
        0x00220326, 0x00230B24,
        0x00240D55, 0x00251CC5,
        0x002606C8, 0x00271868,
        0x00280369, 0x002916CA,
        0x002A0CC9, 0x002B1D58,
        0x002C0784, 0x002D060A,
        0x002E064A, 0x002F0E2A,
        0x0030032A, 0x00310B28,
        0x00320688, 0x00330008,
        0x003406C4, 0x00351864,
        0x003601A8, 0x00370388,
        0x0038078A, 0x00390604,
        0x003A0644, 0x003B0E24,
        0x003C004A, 0x003D18A4,
        0x003E1B24, 0x003F00EA,
        0x00400F0A, 0x00410249,
        0x00420D5D, 0x00431CC4,
        0x00440328, 0x00450B29,
        0x004606C6, 0x0047076A,
        0x00480368, 0x004916C5,
        0x004A0789, 0x004B0605,
        0x004C0CC8, 0x004D1954,
        0x004E0645, 0x004F0E25,
        0x00500325, 0x00510B26,
        0x005206C9, 0x00530764,
        0x005408A9, 0x00550009,
        0x005601A9, 0x00570389,
        0x00580785, 0x00590609,
        0x005A0049, 0x005B18A9,
        0x005C0649, 0x005D0E29,
        0x005E1B29, 0x005F00E9,
        0x00600365, 0x006116C6,
        0x00620786, 0x00630608,
        0x00640788, 0x00650606,
        0x00660046, 0x006718A8,
        0x006858A6, 0x00690145,
        0x006A01E9, 0x006B178A,
        0x006C01E8, 0x006D1785,
        0x006E1E28, 0x006F0C65,
        0x00700CC5, 0x00711D5C,
        0x00720648, 0x00730E28,
        0x00740646, 0x00750E26,
        0x00761B28, 0x007700E6,
        0x007801E5, 0x00791786,
        0x007A1E29, 0x007B0C68,
        0x007C1E24, 0x007D0C69,
        0x007E0955, 0x007F03C9,
        0x008003E9, 0x00810975,
        0x00820C49, 0x00831E04,
        0x00840C48, 0x00851E05,
        0x008617A6, 0x008701C5,
        0x008800C6, 0x00891B08,
        0x008A0E06, 0x008B0666,
        0x008C0E08, 0x008D0668,
        0x008E1D7C, 0x008F0CE5,
        0x00900C45, 0x00911E08,
        0x009217A9, 0x009301C4,
        0x009417AA, 0x009501C9,
        0x00960169, 0x0097588A,
        0x00981888, 0x00990066,
        0x009A0709, 0x009B07A8,
        0x009C0704, 0x009D07A6,
        0x009E16E6, 0x009F0345,
        0x00A000C9, 0x00A11B05,
        0x00A20E09, 0x00A30669,
        0x00A41885, 0x00A50065,
        0x00A60706, 0x00A707A5,
        0x00A803A9, 0x00A90189,
        0x00AA0029, 0x00AB0889,
        0x00AC0744, 0x00AD06E9,
        0x00AE0B06, 0x00AF0229,
        0x00B00E05, 0x00B10665,
        0x00B21974, 0x00B30CE8,
        0x00B4070A, 0x00B507A9,
        0x00B616E9, 0x00B70348,
        0x00B8074A, 0x00B906E6,
        0x00BA0B09, 0x00BB0226,
        0x00BC1CE4, 0x00BD0D7D,
        0x00BE0269, 0x00BF08C9,
        0x00C000CA, 0x00C11B04,
        0x00C21884, 0x00C3006A,
        0x00C40E04, 0x00C50664,
        0x00C60708, 0x00C707AA,
        0x00C803A8, 0x00C90184,
        0x00CA0749, 0x00CB06E4,
        0x00CC0020, 0x00CD0888,
        0x00CE0B08, 0x00CF0224,
        0x00D00E0A, 0x00D1066A,
        0x00D20705, 0x00D307A4,
        0x00D41D78, 0x00D50CE9,
        0x00D616EA, 0x00D70349,
        0x00D80745, 0x00D906E8,
        0x00DA1CE9, 0x00DB0D75,
        0x00DC0B04, 0x00DD0228,
        0x00DE0268, 0x00DF08C8,
        0x00E003A5, 0x00E10185,
        0x00E20746, 0x00E306EA,
        0x00E40748, 0x00E506E5,
        0x00E61CE8, 0x00E70D79,
        0x00E81D74, 0x00E95CE6,
        0x00EA02E9, 0x00EB0849,
        0x00EC02E8, 0x00ED0848,
        0x00EE0086, 0x00EF0A08,
        0x00F00021, 0x00F10885,
        0x00F20B05, 0x00F3022A,
        0x00F40B0A, 0x00F50225,
        0x00F60265, 0x00F708C5,
        0x00F802E5, 0x00F90845,
        0x00FA0089, 0x00FB0A09,
        0x00FC008A, 0x00FD0A0A,
        0x00FE02A9, 0x00FF0062,
    };

    if (!hbmMask)
	return NtGdiBitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, FRGND_ROP3(dwRop));

    /* 1. make mask bitmap's dc */
    hDCMask = NtGdiCreateCompatableDC(hdcDest);
    hOldMaskBitmap = (HBITMAP)NtGdiSelectObject(hDCMask, hbmMask);

    /* 2. make masked Background bitmap */

    /* 2.1 make bitmap */
    hDC1 = NtGdiCreateCompatableDC(hdcDest);
    hBitmap2 = NtGdiCreateCompatibleBitmap(hdcDest, nWidth, nHeight);
    hOldBitmap2 = (HBITMAP)NtGdiSelectObject(hDC1, hBitmap2);

    /* 2.2 draw dest bitmap and mask */
    NtGdiBitBlt(hDC1, 0, 0, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, SRCCOPY);
    NtGdiBitBlt(hDC1, 0, 0, nWidth, nHeight, hdcDest, nXDest, nYDest, BKGND_ROP3(dwRop));
    NtGdiBitBlt(hDC1, 0, 0, nWidth, nHeight, hDCMask, xMask, yMask, DSTERASE);

    /* 3. make masked Foreground bitmap */

    /* 3.1 make bitmap */
    hDC2 = NtGdiCreateCompatableDC(hdcDest);
    hBitmap3 = NtGdiCreateCompatibleBitmap(hdcDest, nWidth, nHeight);
    hOldBitmap3 = (HBITMAP)NtGdiSelectObject(hDC2, hBitmap3);

    /* 3.2 draw src bitmap and mask */
    NtGdiBitBlt(hDC2, 0, 0, nWidth, nHeight, hdcDest, nXDest, nYDest, SRCCOPY);
    NtGdiBitBlt(hDC2, 0, 0, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, FRGND_ROP3(dwRop));
    NtGdiBitBlt(hDC2, 0, 0, nWidth, nHeight, hDCMask, xMask, yMask, SRCAND);

    /* 4. combine two bitmap and copy it to hdcDest */
    NtGdiBitBlt(hDC1, 0, 0, nWidth, nHeight, hDC2, 0, 0, SRCPAINT);
    NtGdiBitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hDC1, 0, 0, SRCCOPY);

    /* 5. restore all object */
    NtGdiSelectObject(hDCMask, hOldMaskBitmap);
    NtGdiSelectObject(hDC1, hOldBitmap2);
    NtGdiSelectObject(hDC2, hOldBitmap3);

    /* 6. delete all temp object */
    NtGdiDeleteObject(hBitmap2);
    NtGdiDeleteObject(hBitmap3);

    NtGdiDeleteDC(hDC1);
    NtGdiDeleteDC(hDC2);
    NtGdiDeleteDC(hDCMask);

    return TRUE;
}

BOOL STDCALL NtGdiPlgBlt(HDC  hDCDest,
                CONST POINT  *Point,
                HDC  hDCSrc,
                INT  XSrc,
                INT  YSrc,
                INT  Width,
                INT  Height,
                HBITMAP  hMaskBitmap,
                INT  xMask,
                INT  yMask)
{
  UNIMPLEMENTED;
}

LONG STDCALL NtGdiSetBitmapBits(HBITMAP  hBitmap,
                        DWORD  Bytes,
                        CONST VOID *Bits)
{
  LONG height, ret;
  PBITMAPOBJ bmp;

  bmp = BITMAPOBJ_LockBitmap(hBitmap);
  if (bmp == NULL || Bits == NULL)
  {
    return 0;
  }

  if (Bytes < 0)
  {
    DPRINT ("(%ld): Negative number of bytes passed???\n", Bytes );
    Bytes = -Bytes;
  }

  /* Only get entire lines */
  height = Bytes / bmp->bitmap.bmWidthBytes;
  if (height > bmp->bitmap.bmHeight)
  {
    height = bmp->bitmap.bmHeight;
  }
  Bytes = height * bmp->bitmap.bmWidthBytes;
  DPRINT ("(%08x, bytes:%ld, bits:%p) %dx%d %d colors fetched height: %ld\n",
          hBitmap,
          Bytes,
          Bits,
          bmp->bitmap.bmWidth,
          bmp->bitmap.bmHeight,
          1 << bmp->bitmap.bmBitsPixel,
          height);

#if 0
  /* FIXME: call DDI specific function here if available  */
  if(bmp->DDBitmap)
  {
    DPRINT ("Calling device specific BitmapBits\n");
    if (bmp->DDBitmap->funcs->pBitmapBits)
    {
      ret = bmp->DDBitmap->funcs->pBitmapBits(hBitmap, (void *) Bits, Bytes, DDB_SET);
    }
    else
    {
      DPRINT ("BitmapBits == NULL??\n");
      ret = 0;
    }
  }
  else
#endif
    {
      /* FIXME: Alloc enough for entire bitmap */
      if (bmp->bitmap.bmBits == NULL)
      {
        bmp->bitmap.bmBits = ExAllocatePool (PagedPool, Bytes);
      }
      if(!bmp->bitmap.bmBits)
      {
        DPRINT ("Unable to allocate bit buffer\n");
        ret = 0;
      }
      else
      {
        memcpy(bmp->bitmap.bmBits, Bits, Bytes);
        ret = Bytes;
      }
    }

  BITMAPOBJ_UnlockBitmap(hBitmap);

  return ret;
}

BOOL STDCALL NtGdiSetBitmapDimensionEx(HBITMAP  hBitmap,
                               INT  Width,
                               INT  Height,
                               LPSIZE  Size)
{
  PBITMAPOBJ  bmp;

  bmp = BITMAPOBJ_LockBitmap(hBitmap);
  if (bmp == NULL)
  {
    return FALSE;
  }

  if (Size)
  {
    *Size = bmp->size;
  }
  bmp->size.cx = Width;
  bmp->size.cy = Height;

  BITMAPOBJ_UnlockBitmap (hBitmap);

  return TRUE;
}

COLORREF STDCALL NtGdiSetPixel(HDC  hDC,
                       INT  X,
                       INT  Y,
                       COLORREF  Color)
{
   COLORREF cr = NtGdiGetPixel(hDC,X,Y);
   if(cr != CLR_INVALID && NtGdiSetPixelV(hDC,X,Y,Color))
   {
      return(cr);
   }
   return ((COLORREF) -1);
}

BOOL STDCALL NtGdiSetPixelV(HDC  hDC,
                    INT  X,
                    INT  Y,
                    COLORREF  Color)
{
  HBRUSH NewBrush = NtGdiCreateSolidBrush(Color);
  HGDIOBJ OldBrush;
  
  if (NewBrush == NULL)
    return(FALSE);
  OldBrush = NtGdiSelectObject(hDC, NewBrush);
  if (OldBrush == NULL)
    return(FALSE);
  NtGdiPatBlt(hDC, X, Y, 1, 1, PATCOPY);
  NtGdiSelectObject(hDC, OldBrush);
  NtGdiDeleteObject(NewBrush);
  return TRUE;
}

BOOL STDCALL NtGdiStretchBlt(HDC  hDCDest,
                     INT  XOriginDest,
                     INT  YOriginDest,
                     INT  WidthDest,
                     INT  HeightDest,
                     HDC  hDCSrc,
                     INT  XOriginSrc,
                     INT  YOriginSrc,
                     INT  WidthSrc,
                     INT  HeightSrc,
                     DWORD  ROP)
{
  PDC DCDest = NULL;
  PDC DCSrc  = NULL;
  PSURFOBJ SurfDest, SurfSrc;
  PSURFGDI SurfGDIDest, SurfGDISrc;
  RECTL DestRect;
  POINTL SourcePoint;
  BOOL Status;
  PPALGDI PalDestGDI, PalSourceGDI;
  PXLATEOBJ XlateObj = NULL;
  HPALETTE SourcePalette, DestPalette;
  ULONG SourceMode, DestMode;
  PBRUSHOBJ BrushObj;
  BOOL UsesSource = ((ROP & 0xCC0000) >> 2) != (ROP & 0x330000);
  BOOL UsesPattern = ((ROP & 0xF00000) >> 4) != (ROP & 0x0F0000);  

  DCDest = DC_LockDc(hDCDest);
  if (NULL == DCDest)
    {
      DPRINT1("Invalid destination dc handle (0x%08x) passed to NtGdiStretchBlt\n", hDCDest);
      SetLastWin32Error(ERROR_INVALID_HANDLE);
      return FALSE;
    }

  if (UsesSource)
    {
      if (hDCSrc != hDCDest)
        {
          DCSrc = DC_LockDc(hDCSrc);
          if (NULL == DCSrc)
            {
              DC_UnlockDc(hDCDest);
              DPRINT1("Invalid source dc handle (0x%08x) passed to NtGdiStretchBlt\n", hDCSrc);
              SetLastWin32Error(ERROR_INVALID_HANDLE);
              return FALSE;
            }
        }
      else
        {
           DCSrc = DCDest;
        }
    }
  else
    {
      DCSrc = NULL;
    }

  /* Offset the destination and source by the origin of their DCs. */
  XOriginDest += DCDest->w.DCOrgX;
  YOriginDest += DCDest->w.DCOrgY;
  if (UsesSource)
    {
      XOriginSrc += DCSrc->w.DCOrgX;
      YOriginSrc += DCSrc->w.DCOrgY;
    }

  DestRect.left   = XOriginDest;
  DestRect.top    = YOriginDest;
  DestRect.right  = XOriginDest+WidthDest;
  DestRect.bottom = YOriginDest+HeightDest;

  SourcePoint.x = XOriginSrc;
  SourcePoint.y = YOriginSrc;

  /* Determine surfaces to be used in the bitblt */
  SurfDest = (PSURFOBJ)AccessUserObject((ULONG)DCDest->Surface);
  SurfGDIDest = (PSURFGDI)AccessInternalObjectFromUserObject(SurfDest);
  if (UsesSource)
    {
      SurfSrc  = (PSURFOBJ)AccessUserObject((ULONG)DCSrc->Surface);
      SurfGDISrc  = (PSURFGDI)AccessInternalObjectFromUserObject(SurfSrc);
    }
  else
    {
      SurfSrc  = NULL;
      SurfGDISrc  = NULL;
    }

  if (UsesPattern)
    {
      BrushObj = BRUSHOBJ_LockBrush(DCDest->w.hBrush);
      if (NULL == BrushObj)
        {
          if (UsesSource && hDCSrc != hDCDest)
            {
              DC_UnlockDc(hDCSrc);
            }
          DC_UnlockDc(hDCDest);
          SetLastWin32Error(ERROR_INVALID_HANDLE);
          return FALSE;
        }
    }
  else
    {
      BrushObj = NULL;
    }

  if (DCDest->w.hPalette != 0)
    {
      DestPalette = DCDest->w.hPalette;
    }
  else
    {
      DestPalette = NtGdiGetStockObject(DEFAULT_PALETTE);
    }

  if (UsesSource && DCSrc->w.hPalette != 0)
    {
      SourcePalette = DCSrc->w.hPalette;
    }
  else
    {
      SourcePalette = NtGdiGetStockObject(DEFAULT_PALETTE);
    }

  PalSourceGDI = PALETTE_LockPalette(SourcePalette);
  if (NULL == PalSourceGDI)
    {
      if (UsesSource && hDCSrc != hDCDest)
        {
          DC_UnlockDc(hDCSrc);
        }
      DC_UnlockDc(hDCDest);
      SetLastWin32Error(ERROR_INVALID_HANDLE);
      return FALSE;
    }
  SourceMode = PalSourceGDI->Mode;
  PALETTE_UnlockPalette(SourcePalette);

  if (DestPalette == SourcePalette)
    {
      DestMode = SourceMode;
    }
  else
    {
      PalDestGDI = PALETTE_LockPalette(DestPalette);
      if (NULL == PalDestGDI)
        {
          if (UsesSource && hDCSrc != hDCDest)
            {
              DC_UnlockDc(hDCSrc);
            }
          DC_UnlockDc(hDCDest);
          SetLastWin32Error(ERROR_INVALID_HANDLE);
          return FALSE;
        }
      DestMode = PalDestGDI->Mode;
      PALETTE_UnlockPalette(DestPalette);
    }

  XlateObj = (PXLATEOBJ)IntEngCreateXlate(DestMode, SourceMode, DestPalette, SourcePalette);
  if (NULL == XlateObj)
    {
      if (UsesSource && hDCSrc != hDCDest)
        {
          DC_UnlockDc(hDCSrc);
        }
      DC_UnlockDc(hDCDest);
      SetLastWin32Error(ERROR_NO_SYSTEM_RESOURCES);
      return FALSE;
    }

  /* Perform the bitblt operation */
//  Status = IntEngBitBlt(SurfDest, SurfSrc, NULL, DCDest->CombinedClip, XlateObj,
  //                      &DestRect, &SourcePoint, NULL, BrushObj, NULL, ROP);
             
  //Status = IntEngStretchBlt(SurfDest, SurfSrc, NULL, DCDest->CombinedClip,
  //                    XlateObj, &DestRect, &SourceRect, NULL, NULL, NULL, ROP);
  Status = FALSE;
             

  EngDeleteXlate(XlateObj);
  if (UsesPattern)
    {
      BRUSHOBJ_UnlockBrush(DCDest->w.hBrush);
    }
  if (UsesSource && hDCSrc != hDCDest)
    {
      DC_UnlockDc(hDCSrc);
    }
  DC_UnlockDc(hDCDest);

  return Status;
}

/*  Internal Functions  */

INT FASTCALL
BITMAPOBJ_GetWidthBytes (INT bmWidth, INT bpp)
{
#if 0
  switch(bpp)
  {
    case 1:
      return 2 * ((bmWidth+15) >> 4);

    case 24:
      bmWidth *= 3; /* fall through */
    case 8:
      return bmWidth + (bmWidth & 1);

    case 32:
      return bmWidth * 4;

    case 16:
    case 15:
      return bmWidth * 2;

    case 4:
      return 2 * ((bmWidth+3) >> 2);

    default:
      DPRINT ("stub");
  }

  return -1;
#endif

  return ((bmWidth * bpp + 31) & ~31) >> 3;
}

HBITMAP FASTCALL BITMAPOBJ_CopyBitmap(HBITMAP  hBitmap)
{
  PBITMAPOBJ  bmp;
  HBITMAP  res;
  BITMAP  bm;

  bmp = BITMAPOBJ_LockBitmap(hBitmap);
  if (bmp == NULL)
  {
    return 0;
  }
  res = 0;

  bm = bmp->bitmap;
  bm.bmBits = NULL;
  BITMAPOBJ_UnlockBitmap(hBitmap);
  res = NtGdiCreateBitmapIndirect(&bm);
  if(res)
  {
    char *buf;

    buf = ExAllocatePool (NonPagedPool, bm.bmWidthBytes * bm.bmHeight);
    NtGdiGetBitmapBits (hBitmap, bm.bmWidthBytes * bm.bmHeight, buf);
    NtGdiSetBitmapBits (res, bm.bmWidthBytes * bm.bmHeight, buf);
    ExFreePool (buf);
  }

  return  res;
}

INT STDCALL BITMAP_GetObject(BITMAPOBJ * bmp, INT count, LPVOID buffer)
{
  if(bmp->dib)
  {
    if(count < (INT) sizeof(DIBSECTION))
    {
      if (count > (INT) sizeof(BITMAP)) count = sizeof(BITMAP);
    }
    else
    {
      if (count > (INT) sizeof(DIBSECTION)) count = sizeof(DIBSECTION);
    }
    memcpy(buffer, bmp->dib, count);
    return count;
  }
  else
  {
    if (count > (INT) sizeof(BITMAP)) count = sizeof(BITMAP);
    memcpy(buffer, &bmp->bitmap, count);
    return count;
  }
}
/* EOF */
