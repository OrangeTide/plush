/******************************************************************************
**                         Plush Version 1.0                                 **
**                                                                           **
**                       Dirty-Scanlining Code                               **
**                                                                           **
**             All code copyright (c) 1996-1997, Justin Frankel              **
******************************************************************************/

/*
** About dirty scanlining:
**   Dirty scanlining is a method of reducing the amount of memory one needs
**   to copy to the screen, or z-buffer to clear, etc. For example, you have
**   a 64x64 icon you are rendering in the bottom right hand corner of a
**   1024x768 screen, why bother updating the whole screen? This is the idea
**   behind dirty-rectangles. Me, I'm lazy, so here is dirty-scanlining. The
**   scanline buffer has the same number of entries as scanlines, each entry
**   consisting of four 16 bit integers. The first integer is the starting 
**   point in the scanline that was updated this frame. The next is the end.
**   Then next is the starting point that was updated last frame. The next
**   is the end. So to find out how much mem you need to update, you just
**   find the starting point by min(first,third), and the end by max(second,
**   fourth). Then, when done, you assign the old min and max (3&4) to the
**   calculated min and max. And viola, you have dirty scanlining.
**
**   Sorry for the lousy explanation, it's 2:25am, and I am tired... J.F.
*/


#include <stdlib.h>
#include "plush.h"

void plFreeScanLineBuffers(pl_CameraType *Camera) {
  if (Camera->_ScanLineBuffer) free(Camera->_ScanLineBuffer);
  Camera->_ScanLineBuffer = 0;
}

void plInitScanLineBuffers(pl_CameraType *Camera) {
  pl_uInt16Type i, *b;
  if (Camera->_ScanLineBuffer) plFreeScanLineBuffers(Camera);
  b = (pl_uInt16Type *) 
    malloc(Camera->ScreenHeight*4*sizeof(pl_uInt16Type));
  Camera->_ScanLineBuffer = b;
  i = Camera->ScreenHeight<<1;
  do {                  /* Setup buffer to need updating (initial frame) */
    *b++ = Camera->ScreenWidth;
    *b++ = 0;
  } while (--i);
}

void plWriteBuffer(pl_CameraType *Camera,
                void (*moveout)(pl_CameraType *, pl_uInt32Type, pl_uInt32Type),
                void (*movein)(pl_CameraType *, pl_uInt32Type, pl_uInt32Type)) {
  pl_uInt32Type i, offset, minn, maxx, min2, max2;
  pl_uInt16Type *active = Camera->_ScanLineBuffer;
  if (!active) { /* Scanlinebuffer not active, just copy all data to screen */
    for (i = Camera->ClipTop; i < Camera->ClipBottom; i ++) 
      moveout(Camera,Camera->ScreenWidth*i + Camera->ClipLeft,
              Camera->ClipRight-Camera->ClipLeft);
    for (i = Camera->ClipTop; i < Camera->ClipBottom; i ++) 
      movein(Camera,Camera->ScreenWidth*i + Camera->ClipLeft,
             Camera->ClipRight-Camera->ClipLeft);
    return;
  }
  i = Camera->ScreenHeight;
  offset = 0;
  do {
    minn = *(active++);      /* Get this frame min */
    maxx = *(active++);      /* Get this frame max */
    min2 = *(active++);      /* Get last frame min */
    max2 = *(active++);      /* Get last frame max */
    minn = plMin(minn,min2); /* Our min is the min of the first two mins */
    maxx = plMax(max2,maxx) ;/* Our max is the max of the first two maxs */
                             /* Copy data out */
    if (maxx > minn) moveout(Camera,offset+minn, maxx-minn);
    offset += Camera->ScreenWidth;
  } while (--i);
  active = Camera->_ScanLineBuffer;
  offset = 0;
  i = Camera->ScreenHeight;
  do {
    minn = *active;
    maxx = *(active+1);
    *active++ = Camera->ScreenWidth; /* Initialize next frame to empty */
    *active++ = 0;
    *active++ = minn; /* Set last frame to this frame */
    *active++ = maxx;
                     /* Clear framebuffer, zbuffer, etc */
    if (maxx > minn) movein(Camera,offset+minn,maxx-minn);
    offset += Camera->ScreenWidth;
  } while (--i);
}
