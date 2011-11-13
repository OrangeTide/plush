#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "cds.h"
#include "resource.h"

#include "../plush.h"

static char szAppName[] = "Cube";

void cubeRenderFrame(char *);
void cubeSetupColors(char *);
int cubeInit(int, int, float);
void cubeDeInit();

int windowWidth = 320, windowHeight = 240;

HWND hMainWindow;
HINSTANCE hMainInstance;
BOOL fAppActive;

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
BOOL InitApplication(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
						 WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInst,
                   LPSTR lpszCmdParam,
				   int nCmdShow) {
  MSG msg;
  if (!InitApplication(hInstance)) {
    MessageBox(NULL, "Could not initialize application", NULL, MB_OK);
    return (FALSE);
  }
  if (!InitInstance(hInstance, nCmdShow)) {
    MessageBox(NULL, "Could not create window", NULL, MB_OK);
    return (FALSE);
  }
  while (1) {
    if (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE)) {
      if (msg.message == WM_QUIT) break;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else WaitMessage();
  }
  return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
						 WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_CREATE:
      cdsInit(windowWidth,windowHeight);
	  if (cubeInit(windowWidth,windowHeight,70.0)) {
	    MessageBox(hWnd,"Could not load cube.pcx",
			       "Could not load cube.pcx",MB_OK);
		PostMessage(hWnd,WM_DESTROY,0,0);
      } else {
	    unsigned char pal[768];
		cubeSetupColors(pal);
	    cdsSetPalette(pal);
	  }
      SetTimer(hWnd, 1, 1000/60, NULL);
    break;
    case WM_TIMER:
      cdsRender(hWnd, cubeRenderFrame);
    break;
    case WM_ACTIVATEAPP:
      fAppActive = (BOOL)wParam;
    break;
    case WM_ERASEBKGND:
    return 0;
    case WM_SIZE:
	  windowWidth = (LOWORD(lParam)+3)&~3;
	  windowHeight = HIWORD(lParam);
	  cdsInit(windowWidth,windowHeight);
	  cubeInit(windowWidth,windowHeight, 70);
	return 0;
    case WM_PAINT:
      cdsPaint(hWnd);
    return 0;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
	    case IDR_MAINMENU_FILE_QUIT:
    		if (MessageBox(hWnd,"Would you like to quit?","Quit?",MB_YESNO) == IDYES)
	          PostMessage(hWnd,WM_DESTROY,0,0);
	    break;
	    case IDR_MAINMENU_HELP_ABOUT: {
          char str[256];
		  sprintf(str,"Cube Demo v1.0\n"
			          "Uses: \n"
					  "  %s\n"
					  "  %s\n", plVersionString, plCopyrightString);

		  MessageBox(hWnd,str,"About",MB_OK);

        } break;
      }
      break;
    case WM_DESTROY:
	  cubeDeInit();
	  cdsDeInit();
	  PostQuitMessage(0);
	return 0;
  }
  return (DefWindowProc(hWnd, message, wParam, lParam));
}

BOOL InitApplication(HINSTANCE hInstance) {
  WNDCLASS wc;

  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  hMainInstance = hInstance;
  wc.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_CDS));
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
  wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
  wc.lpszClassName = szAppName;

  return RegisterClass(&wc);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hMainWindow = CreateWindow(szAppName, "Plush 3D Cube Test",
        WS_OVERLAPPEDWINDOW,
        0,0,windowWidth,windowHeight,
	NULL, NULL,
	hMainInstance,
	NULL);
  if (!hMainWindow) return FALSE;

  ShowWindow(hMainWindow, nCmdShow);
  UpdateWindow(hMainWindow);
  return TRUE;
}
