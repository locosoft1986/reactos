

#include <win32k/brush.h>

HBRUSH  W32kCreateBrushIndirect(CONST LOGBRUSH  *lb)
{
  UNIMPLEMENTED;
}

HBRUSH  W32kCreateDIBPatternBrush(HGLOBAL  hDIBPacked,
                                  UINT  ColorSpec)
{
  UNIMPLEMENTED;
}

HBRUSH  W32kCreateDIBPatternBrushPt(CONST VOID  *PackedDIB,
                                    UINT  Usage)
{
  UNIMPLEMENTED;
}

HBRUSH  W32kCreateHatchBrush(INT  Style,
                             COLORREF  Color)
{
  UNIMPLEMENTED;
}

HBRUSH  W32kCreatePatternBrush(HBITMAP  hBitmap)
{
  UNIMPLEMENTED;
}

HBRUSH  W32kCreateSolidBrush(COLORREF  Color)
{
  UNIMPLEMENTED;
}

BOOL  W32kFixBrushOrgEx(VOID)
{
  return FALSE;
}

BOOL  W32kPatBlt(HDC  hDC,
                 INT  XLeft,
                 INT  YLeft,
                 INT  Width,
                 INT  Height,
                 DWORD  ROP)
{
  UNIMPLEMENTED;
}

BOOL  W32kSetBrushOrgEx(HDC  hDC,
                        INT  XOrg,
                        INT  YOrg,
                        LPPOINT  Point)
{
  UNIMPLEMENTED;
}


