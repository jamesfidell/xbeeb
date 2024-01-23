/*
 *
 * $Id: Modes.h,v 1.6 2002/01/15 15:46:43 james Exp $
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
 * $Log: Modes.h,v $
 * Revision 1.6  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.5  2000/08/16 17:58:28  james
 * Update copyright message
 *
 * Revision 1.4  1996/09/30 22:59:08  james
 * Prevent multiple inclusion of header files.
 *
 * Revision 1.3  1996/09/24 23:05:40  james
 * Update copyright dates.
 *
 * Revision 1.2  1996/09/23 16:09:52  james
 * Initial implementation of bitmap MODEs -- including modification of
 * screen handling to use different windows for teletext and bitmapped
 * modes and corrections/improvements to colour- and cursor-handling
 * code.
 *
 * Revision 1.1  1996/09/21 17:20:39  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#ifndef	MODES_H
#define	MODES_H

/*
 * Possible values for CurrentScreenMode
 */

#define	MODE_UNKNOWN	0
#define	MODE_BITMAP		1
#define	MODE_TELETEXT	2


extern	void			InitialiseTeletext();

extern	unsigned char	CurrentScreenMode;
extern	unsigned short	ScreenLength;
extern	unsigned short	TopOfScreen;
extern	unsigned short	StartOfScreenMemory;

#endif	/* MODES_H */
