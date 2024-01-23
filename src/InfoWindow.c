/*
 *
 * $Id: InfoWindow.c,v 1.3 2002/01/15 15:46:43 james Exp $
 *
 * Copyright (C) James Fidell 1996-2002.
 *
 * Permission to use, copy, modify and distribute this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of the copyright holder
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission. The
 * copyright holder makes no representations about the suitability of
 * this software for any purpose. It is provided "as is" without express
 * or implied warranty.
 *
 * THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 * Modification History
 *
 * $Log: InfoWindow.c,v $
 * Revision 1.3  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.2  2000/08/16 17:58:27  james
 * Update copyright message
 *
 * Revision 1.1  1996/10/10 21:52:20  james
 * Add Info Window with status LEDs.
 *
 *
 */


#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Config.h"
#include "Screen.h"
#include "Keyboard.h"
#include "SerialUla.h"
#include "InfoWindow.h"


#define	InfoString(x,y,s,l)		XDrawImageString ( dpy, InfoWindow, \
									InfoWindowGC, x, y, s, l );

void
InfoWindowRedraw()
{
	XClearWindow ( dpy, InfoWindow );

	XSetForeground ( dpy, InfoWindowGC, InfoWindowWhite );
	XSetBackground ( dpy, InfoWindowGC, InfoWindowBlack );
	InfoString ( 40, 25, "CAPS", 4 );
	InfoString ( 40, 38, "LOCK", 4 );
	InfoString ( 87, 25, "SHIFT", 5 );
	InfoString ( 90, 38, "LOCK", 4 );
	InfoString ( 136, 25, "MOTOR", 5 );

	DrawCapsLockLED();
#ifdef	SHIFTLOCK_SOUND_HACK
	LockKeysChanged = 1;
#endif
	DrawShiftLockLED();
	DrawMotorLED();

	return;
}


void
DrawCapsLockLED()
{
	XSetFillStyle ( dpy, InfoWindowGC, FillSolid );
	XSetFunction ( dpy, InfoWindowGC, GXcopy );

	if ( CapsLockLED )
	{
		XSetForeground ( dpy, InfoWindowGC, InfoWindowRed );
		XFillArc ( dpy, InfoWindow, InfoWindowGC, 47, 1, 14, 14, 0, 23040 );
	}
	else
	{
		XSetForeground ( dpy, InfoWindowGC, InfoWindowBlack );
		XFillArc ( dpy, InfoWindow, InfoWindowGC, 47, 1, 14, 14, 0, 23040 );
		XSetForeground ( dpy, InfoWindowGC, InfoWindowRed );
		XDrawArc ( dpy, InfoWindow, InfoWindowGC, 47, 1, 14, 14, 0, 23040 );
	}

	XFlush ( dpy );
}


void
DrawShiftLockLED()
{
#ifdef	SHIFTLOCK_SOUND_HACK
	if ( LockKeysChanged )
	{
#endif

	XSetFillStyle ( dpy, InfoWindowGC, FillSolid );
	XSetFunction ( dpy, InfoWindowGC, GXcopy );

	if ( ShiftLockLED )
	{
		XSetForeground ( dpy, InfoWindowGC, InfoWindowRed );
		XFillArc ( dpy, InfoWindow, InfoWindowGC, 97, 1, 14, 14, 0, 23040 );
	}
	else
	{
		XSetForeground ( dpy, InfoWindowGC, InfoWindowBlack );
		XFillArc ( dpy, InfoWindow, InfoWindowGC, 97, 1, 14, 14, 0, 23040 );
		XSetForeground ( dpy, InfoWindowGC, InfoWindowRed );
		XDrawArc ( dpy, InfoWindow, InfoWindowGC, 97, 1, 14, 14, 0, 23040 );
	}

	XFlush ( dpy );
#ifdef	SHIFTLOCK_SOUND_HACK
	LockKeysChanged = 0;
	}
#endif
}


void
DrawMotorLED()
{
	XSetFillStyle ( dpy, InfoWindowGC, FillSolid );
	XSetFunction ( dpy, InfoWindowGC, GXcopy );

	if ( MotorLED )
	{
		XSetForeground ( dpy, InfoWindowGC, InfoWindowRed );
		XFillArc ( dpy, InfoWindow, InfoWindowGC, 147, 1, 14, 14, 0, 23040 );
	}
	else
	{
		XSetForeground ( dpy, InfoWindowGC, InfoWindowBlack );
		XFillArc ( dpy, InfoWindow, InfoWindowGC, 147, 1, 14, 14, 0, 23040 );
		XSetForeground ( dpy, InfoWindowGC, InfoWindowRed );
		XDrawArc ( dpy, InfoWindow, InfoWindowGC, 147, 1, 14, 14, 0, 23040 );
	}

	XFlush ( dpy );
}
