/*
 *
 * $Id: Bitmap.h,v 1.8 2002/01/15 15:46:43 james Exp $
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
 * $Log: Bitmap.h,v $
 * Revision 1.8  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.7  2000/08/16 17:58:26  james
 * Update copyright message
 *
 * Revision 1.6  1996/11/09 23:02:28  james
 * Take account of the horizontal start of display based on the clock rate
 * set in the Video ULA and the CRTC Horiz. Sync. register.
 *
 * Revision 1.5  1996/11/07 23:41:37  james
 * Add CVS headers.
 *
 *
 */


#ifndef	BITMAP_H
#define	BITMAP_H

extern	void			InitialiseBitmap();
extern	void			BitmapScanlineUpdate ( unsigned int );
extern	void			BitmapScreenUpdate ( void );
extern	void			ChangeBitmapWindowX ( int );
extern	void			ChangeBitmapWindowY ( int );

extern	byteval			BitsForColourInfo;
extern	int				BitmapWindowX;
extern	int				BitmapWindowY;
extern	unsigned short	StartPosnX;

#endif	/* BITMAP_H */
