#include <windows.h>
#include "cds.h"

typedef struct {
  BITMAPINFOHEADER bi;
  RGBQUAD aColors[256];
  LPVOID lpvData;
} _IMAGE;

static _IMAGE image;
static char Active = 0;
static int windowWidth, windowHeight;
static HDC hdcImage = NULL;
static HDC hdc;
static HBITMAP gbmOldMonoBitmap = 0;
static HPALETTE hpalApp = 0;
static unsigned char Palette[768];

static void _cdsInit(int, int, unsigned char *);
static void _cdsSetPalette(unsigned char *);

void cdsRender(HWND hWnd, void (*df)(unsigned char *)) {
  if (Active) {
    df(image.lpvData);
    InvalidateRect(hWnd, NULL, FALSE); 
  }
}

BOOL cdsPaint(HWND hwnd) {
  RECT lrc; 
  HDC lhdc;
  PAINTSTRUCT lps;
  if (Active) {
    lhdc = BeginPaint(hwnd,&lps);
    SelectPalette(lhdc, hpalApp, FALSE);
    RealizePalette(lhdc); 
	GetClientRect(hwnd, &lrc);
    BitBlt(lhdc, 0, 0, lrc.right-lrc.left, 
		   lrc.bottom-lrc.top,
           hdcImage, 0, 0, SRCCOPY);
    EndPaint(hwnd,&lps);
    return TRUE;
  } 
  else return FALSE;
}

void cdsDeInit() {
  HBITMAP hbm;
  if (!Active) return;
  Active = 0;
  if (hpalApp) DeleteObject(hpalApp);
  if (hdcImage) { 
    hbm = (HBITMAP) SelectObject(hdcImage, gbmOldMonoBitmap);
    DeleteObject(hbm);
    DeleteDC(hdcImage);
  }
}
void cdsInit(int w, int h) {
  _cdsInit(w,h,Palette);
}

void cdsSetPalette(unsigned char *pal) {
  memcpy(Palette,pal,768);
  if (Active) _cdsInit(windowWidth,windowHeight,Palette);
}

static void _cdsInit(int w, int h, unsigned char *pal) {
  HBITMAP hbm;
  if (Active) cdsDeInit();
  Active = 1;
  _cdsSetPalette(pal);
  windowWidth = w;
  windowHeight = h;
  image.bi.biSize = sizeof(BITMAPINFOHEADER);
  image.bi.biPlanes = 1;
  image.bi.biBitCount = 8;
  image.bi.biCompression = BI_RGB;
  image.bi.biSizeImage = 0;
  image.bi.biClrUsed = 0;
  image.bi.biClrImportant = 0;
  image.bi.biWidth = windowWidth;
  image.bi.biHeight = windowHeight;
  image.bi.biSizeImage = (windowWidth * windowHeight);
  hdcImage = CreateCompatibleDC(NULL);
  hbm = CreateDIBSection(hdcImage, (BITMAPINFO *)&image.bi,
        DIB_RGB_COLORS, &image.lpvData, NULL, 0);
  gbmOldMonoBitmap = (HBITMAP)SelectObject(hdcImage, hbm);
} 

static void _cdsSetPalette(unsigned char *pal) {
  int Counter;
  LOGPALETTE *LogicalPalette = (LOGPALETTE *) 
	  malloc(sizeof(LOGPALETTE) + 
			 sizeof(PALETTEENTRY)*256);
  LogicalPalette->palVersion = 0x300;
  LogicalPalette->palNumEntries = 256;

  for (Counter = 0; Counter < 256; Counter ++) {
    image.aColors[Counter].rgbRed =
      LogicalPalette->palPalEntry[Counter].peRed = *pal++;
    image.aColors[Counter].rgbGreen =
      LogicalPalette->palPalEntry[Counter].peGreen = *pal++;
    image.aColors[Counter].rgbBlue =
      LogicalPalette->palPalEntry[Counter].peBlue = *pal++;
    image.aColors[Counter].rgbReserved = 0;
    LogicalPalette->palPalEntry[Counter].peFlags = PC_NOCOLLAPSE;
  }
  if (hpalApp) DeleteObject(hpalApp);
  hpalApp = CreatePalette((LOGPALETTE *)&LogicalPalette);
}

