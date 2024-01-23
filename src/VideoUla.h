/*
 *
 * $Id: VideoUla.h,v 1.9 2002/01/15 15:46:43 james Exp $
 *
 * Copyright (C) James Fidell 1994-2002.
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
 * $Log: VideoUla.h,v $
 * Revision 1.9  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.8  2000/08/16 17:58:29  james
 * Update copyright message
 *
 * Revision 1.7  1996/11/09 23:02:33  james
 * Take account of the horizontal start of display based on the clock rate
 * set in the Video ULA and the CRTC Horiz. Sync. register.
 *
 * Revision 1.6  1996/10/09 22:06:59  james
 * Overhaul of the bitmapped screen handling code with particular respect to
 * colour maps.
 *
 * Revision 1.5  1996/10/01 00:33:08  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.4  1996/09/24 23:05:46  james
 * Update copyright dates.
 *
 * Revision 1.3  1996/09/23 16:09:53  james
 * Initial implementation of bitmap MODEs -- including modification of
 * screen handling to use different windows for teletext and bitmapped
 * modes and corrections/improvements to colour- and cursor-handling
 * code.
 *
 * Revision 1.2  1996/09/21 22:13:53  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:43  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#ifndef	VIDEOULA_H
#define	VIDEOULA_H

extern  void				ResetVideoUla();
extern  byteval				ReadVideoUla ( int );
extern	byteval				DecodeColour ( byteval, byteval );
extern  void				WriteVideoUla ( int, byteval );

extern	int					SaveVideoUla ( int );
extern	int					RestoreVideoUla ( int, unsigned int );

extern	unsigned char		CharsPerLine;
extern	unsigned char		PixelWidth;
extern	unsigned char		ByteWidth;
extern	unsigned char		PixelsPerByte;
extern	unsigned char		CursorByteWidth;
extern	unsigned char		MasterCursorWidth;
extern	unsigned char		ClockRate;
extern	byteval				VidRegister0;

#endif	/* VIDEOULA_H */
