/*
 *
 * $Id: Screen.h,v 1.12 2002/01/15 15:46:43 james Exp $
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
 * $Log: Screen.h,v $
 * Revision 1.12  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.11  2000/08/16 17:58:28  james
 * Update copyright message
 *
 * Revision 1.10  1996/11/24 22:23:59  james
 * Comment on Boffin screen size.
 *
 * Revision 1.9  1996/10/13 21:59:58  james
 * Changed all window size/position parameters to #defined values.
 *
 * Revision 1.8  1996/10/09 23:19:10  james
 * Added support for using the MIT X11 Shared Memory Extensions.
 *
 * Revision 1.7  1996/10/09 22:06:56  james
 * Overhaul of the bitmapped screen handling code with particular respect to
 * colour maps.
 *
 * Revision 1.6  1996/10/08 00:04:33  james
 * Added InfoWindow to show LED status.  Also required addition of the
 * SHIFTLOCK_SOUND_HACK to prevent the Shift Lock LED being light up
 * whenever the sound buffer is full, which means that far too much
 * time can be spent re-drawing the LED.
 *
 * Revision 1.5  1996/09/24 23:05:43  james
 * Update copyright dates.
 *
 * Revision 1.4  1996/09/23 16:09:52  james
 * Initial implementation of bitmap MODEs -- including modification of
 * screen handling to use different windows for teletext and bitmapped
 * modes and corrections/improvements to colour- and cursor-handling
 * code.
 *
 * Revision 1.3  1996/09/21 23:16:13  james
 * Loading of new X fonts for double height.  Unloading of all fonts.
 *
 * Revision 1.2  1996/09/21 22:13:50  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:40  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#ifndef	SCREEN_H
#define	SCREEN_H

extern	void			InitialiseScreen ( void );
extern	void			ShutdownScreen ( void );
extern	void			CheckEvents ( void );
extern	void			ScreenAddressStartHi ( byteval );
extern	void			ScreenAddressStartLo ( byteval );
extern	void			RecalculateScreenInfo ( void );

extern	unsigned char	ScreenMemoryChanged;
extern	unsigned char	ScreenImageChanged;

#ifdef	_XLIB_H_

extern	GC				TtextTextGC;
extern	GC				TtextMosaicGC;

extern	GC				BitmapGC [ 16 ];
extern	GC				CopyAreaGC;

extern	Font			TtextText;
extern	Font			TtextTextDblU;
extern	Font			TtextTextDblL;
extern	Font			TtextContiguousMosaic;
extern	Font			TtextContiguousMosaicDblU;
extern	Font			TtextContiguousMosaicDblL;
extern	Font			TtextSeparateMosaic;
extern	Font			TtextSeparateMosaicDblU;
extern	Font			TtextSeparateMosaicDblL;

extern	Display			*dpy;
extern	Window			TeletextScreen;
extern	Window			BitmapScreen;
extern	Window			InfoWindow;
extern	Pixmap			BitmapPixmap;
extern	GC				CursorGC, InfoWindowGC;
extern	Colormap		DefCmap;
extern	unsigned long	InfoWindowRed, InfoWindowWhite, InfoWindowBlack;

#ifdef	MITSHM

extern	unsigned char	UseSharedXimage;
extern	unsigned char	UseSharedPixmap;
extern	char			*ImageData;
extern	int				BytesPerImageLine;
extern	XImage			*BitmapImage;

#endif	/* MITSHM */
#endif	/* _XLIB_H_ */

extern	unsigned long	Cells [ 16 ];
extern	unsigned long	Masks [ 4 ];
extern	unsigned long	ColourBits;
extern	int				RgbValues [ 8 ][ 3 ];

extern	byteval			ScreenLengthIndex;

#define	Black			Cells [ 0 ]
#define	Red				Cells [ 1 ]
#define	Green			Cells [ 2 ]
#define	Yellow			Cells [ 3 ]
#define	Blue			Cells [ 4 ]
#define	Magenta			Cells [ 5 ]
#define	Cyan			Cells [ 6 ]
#define	White			Cells [ 7 ]
#define	BlackWhite		Cells [ 8 ]
#define	RedCyan			Cells [ 9 ]
#define	GreenMagenta	Cells [ 10 ]
#define	YellowBlue		Cells [ 11 ]
#define	BlueYellow		Cells [ 12 ]
#define	MagentaGreen	Cells [ 13 ]
#define	CyanRed			Cells [ 14 ]
#define	WhiteBlack		Cells [ 15 ]


/*
 * Sizes and locations of the various windows
 */

/*
 * FIX ME
 *
 * Boffin requires the bitmapped window width to be 752, according to DRS.
 */

#define	TTXT_WIN_H		475
#define	TTXT_WIN_W		480

#define	BMAP_WIN_H		528
#define	BMAP_WIN_W		640

#define	INFO_WIN_H		40
#define	INFO_WIN_W		400

#define	MAIN_WIN_H		568		/* Should be biggest window + info window */
#define	MAIN_WIN_W		640		/* Should be biggest window */

#define	TTXT_WIN_X		(( MAIN_WIN_W - TTXT_WIN_W ) / 2 )
#define	TTXT_WIN_Y		0

#define	BMAP_WIN_X		(( MAIN_WIN_W - BMAP_WIN_W ) / 2 )
#define	BMAP_WIN_Y		0

#define	INFO_WIN_X		(( MAIN_WIN_W - INFO_WIN_W ) / 2 )
#define	INFO_WIN_Y		( MAIN_WIN_H - INFO_WIN_H )


#endif	/* SCREEN_H */
