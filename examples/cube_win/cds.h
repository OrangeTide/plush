void cdsInit(int, int);
void cdsDeInit();

void cdsRender(HWND, void (*df)(unsigned char *));
BOOL cdsPaint(HWND);

void cdsSetPalette(unsigned char *);