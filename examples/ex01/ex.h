// ex.h: provides a standard interface for video and keyboard for the
// example programs to use.

#if defined(linux)
  // Linux: we will use svgalib
  #include <vga.h>
#elif defined(DJGPP)
  // DJGPP: we will use mode13 for video, and use conio for kb
  #include <dos.h>
  #include <conio.h>
  #include <sys/nearptr.h>
#elif defined(__WATCOMC__)
  // Watcom: we will use mode13 for video, and use conio for kb
  #include <dos.h>
  #include <conio.h>
  #define inportb inp
  #define outportb outp
#else
  #error I don\'t know what OS/compiler this is
#endif

char *exGraphMem;

static int exGetKey() {
#ifdef linux
  return (vga_getkey());
#else
  if (kbhit()) return (getch());
  return 0;
#endif
}

#ifdef linux
// we need to make inportb() and outportb()
  static void outportb(short int port, char value) {
    __asm__("outb %%al, %%dx"::"d" (port), "a" (value));
  }
  static char inportb(short int port) {
    register char ret;
    __asm__("inb %%dx, %%al":"=a" (ret):"d" (port));
    return ret;
  }
#endif

static void exWaitVSync() {
  // This code should work on all three
#ifndef NO_RETRACE_WAIT
  while (!(inportb(0x3DA)&8)); // wait for end
  while ((inportb(0x3DA)&8));  // wait for start
#endif
}

static void exSetPalette(char *palette) {
  // This code should also work on all three
  int i;
  outportb(0x3C8,0); // set index to 0
  for (i = 0; i < 768; i ++) outportb(0x3C9,*palette++);
}

static void exSetGraphics() {
#ifdef linux
  vga_init();
  vga_setmode(G320x200x256);
  exGraphMem = graph_mem;
#endif
#ifdef DJGPP
  union REGS regs;
  __djgpp_nearptr_enable();
  regs.d.eax = 0x13;
  int86(0x10,&regs,&regs);
  exGraphMem = (char *) __djgpp_conventional_base + 0xA0000;
#endif
#ifdef __WATCOMC__
  union REGS regs;
  regs.x.eax = 0x13;
  int386(0x10,&regs,&regs);
  exGraphMem = (char *) 0xA0000;
#endif
}

static void exSetText() {
#ifdef linux
  vga_setmode(TEXT);
#endif
#ifdef DJGPP
  union REGS regs;
  __djgpp_nearptr_disable();
  regs.d.eax = 0x3;
  int86(0x10,&regs,&regs);
#endif
#ifdef __WATCOMC__
  union REGS regs;
  regs.x.eax = 0x3;
  int386(0x10,&regs,&regs);
#endif
}
