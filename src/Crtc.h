/*
 * Copyright (c) James Fidell 1994.
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

#endif	/* CRTC_H */
