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


#ifndef	SCREEN_H
#define	SCREEN_H

extern	void			InitialiseScreen ( void );
extern	void			ShutdownScreen ( void );
extern	void			CheckEvents ( void );
extern	void			ScreenAddressStartHi ( byteval );
extern	void			ScreenAddressStartLo ( byteval );
extern	void			RecalculateScreenInfo ( void );

extern	unsigned char	ScreenChanged;

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
extern	Pixmap			BitmapPixmap;
extern	GC				DefaultGraphicsContext;
extern	GC				CursorGC;
extern	Colormap		Cmap;

#endif

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

#endif
