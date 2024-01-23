/*
 *
 * $Id: Crtc.h,v 1.6 1996/10/10 21:44:03 james Exp $
 *
 * Copyright (c) James Fidell 1994, 1995, 1996.
 *
 * Permission to use, copy, modify, distribute, and sell this software
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
 * $Log: Crtc.h,v $
 * Revision 1.6  1996/10/10 21:44:03  james
 * Fixes from David Ralph Stacey for scan-line updates.
 *
 * Revision 1.5  1996/10/01 00:33:00  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.4  1996/09/24 23:05:35  james
 * Update copyright dates.
 *
 * Revision 1.3  1996/09/23 16:09:51  james
 * Initial implementation of bitmap MODEs -- including modification of
 * screen handling to use different windows for teletext and bitmapped
 * modes and corrections/improvements to colour- and cursor-handling
 * code.
 *
 * Revision 1.2  1996/09/21 22:13:47  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:36  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#ifndef	CRTC_H
#define	CRTC_H

extern	byteval				CrtcRegisters[18];
extern	unsigned char		CursorBlinkEnable;
extern	unsigned char		CursorBlinkFrequency;
extern	unsigned char		CursorStartLine;
extern	unsigned char		CursorEnabled;
extern	unsigned char		CursorResized;
extern	unsigned char		CursorMoved;
extern	unsigned char		NewCursorX, NewCursorY;
extern	unsigned int		CrtcMagicNumber;

extern  void				ResetCrtc();
extern  byteval				ReadCrtc ( int );
extern  void				WriteCrtc ( int, byteval );

extern	int					SaveCRTC ( int );
extern	int					RestoreCRTC ( int, unsigned int );

#define	HorizTotal			CrtcRegisters[0]
#define	HorizDisplayed		CrtcRegisters[1]
#define	HorizSync			CrtcRegisters[2]
#define	SyncWidth			CrtcRegisters[3]
#define	VertTotal			CrtcRegisters[4]
#define	VertAdjust			CrtcRegisters[5]
#define	VertDisplayed		CrtcRegisters[6]
#define	VertSync			CrtcRegisters[7]
#define	IlaceDelay			CrtcRegisters[8]
#define	ScanLines			CrtcRegisters[9]
#define	CursorStart			CrtcRegisters[10]
#define	CursorEndLine		CrtcRegisters[11]
#define	ScreenStartHi		CrtcRegisters[12]
#define	ScreenStartLo		CrtcRegisters[13]
#define	CursorPosHi			CrtcRegisters[14]
#define	CursorPosLo			CrtcRegisters[15]
#define	LightPenHi			CrtcRegisters[16]
#define	LightPenLo			CrtcRegisters[17]

/*
 * Variables to allow more optimal performance in display of bitmapped
 * screens by reducing recalculation wherever possible.
 */

extern	byteval				ScanLinesPlus1;
extern	unsigned short		HorizDisplayed8;

/*
 * Something of a "bodge-up" constant that I don't really understand
 * right now, but it does appear to make things work.
 */

#define	VSYNC_TIME			1900

#endif	/* CRTC_H */
