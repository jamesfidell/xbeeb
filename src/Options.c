/*
 *
 * $Id: Options.c,v 1.1 1996/10/10 21:52:50 james Exp $
 *
 * Copyright (c) James Fidell 1996.
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
 * $Log: Options.c,v $
 * Revision 1.1  1996/10/10 21:52:50  james
 * Separate option-handling code from the main program.
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "Config.h"
#include "Beeb.h"
#include "Memory.h"
#include "Patchlevel.h"
#include "Keyboard.h"

#ifdef	EMUL_FS
#include "EFS.h"
#endif


static void		DisplayHelp();


void
ProcessOptions ( int argc, char** argv )
{
	char		c, *end;
	byteval		RomNumber;
	int			i = 1, j, next, mode = -1;
	char		*RomName;

	while ( --argc )
	{
		if ( *argv [ i ] == '-' )
		{
			next = 0;
			j = 1;
			while ( !next && (( c = argv [ i ][ j++ ] ) != '\0' ))
			{
				switch ( c )
				{
					case 'o' :					/* OS ROM name */
						if ( argv [ i ][ j ] == '\0' )
						{
							if ( argc > 1 )
							{
								i++;
								argc--;
								OsRomName = argv [ i ];
								next = 1;
							}
							else
							{
								fprintf ( stderr, "syntax error\nfilename " );
								fprintf ( stderr, "should follow -o\n" );
								exit ( 1 );
							}
						}
						else
						{
							OsRomName = &argv[i][j];
							next = 1;
						}
						break;

					case 'p' :					/* Paged ROM */
						switch ( argv[i][j] )
						{
							case '\0' :			/* No number, use 15 */
								RomNumber = 15;
								break;
							case '0' : case '1' : case '2' :
							case '3' : case '4' : case '5' :
							case '6' : case '7' : case '8' :
							case '9' :
								RomNumber = atoi ( &argv[i][j] );
								if ( RomNumber > 15 )
								{
									fprintf ( stderr, "ROM no. too big\n");
									exit ( 1 );
								}
								break;
							default :
								fprintf ( stderr, "Bad ROM number for -p\n" );
								fprintf ( stderr, "use -h for help\n" );
								exit ( 1 );
						}

						if ( argc > 1 )
						{
							i++;
							argc--;
							RomName = argv [ i ];
							next = 1;
						}
						else
						{
							fprintf ( stderr, "syntax error\nfilename " );
							fprintf ( stderr, "should follow -p\n" );
							exit ( 1 );
						}

						if ( RomNumber < 15 )
							LoadPagedRom ( RomName, RomNumber );
						else
							Rom15Name = RomName;
						break;

					case 'w' :					/* Make a ROM slot writeable */
						if ( argv [ i ][ j ] == '\0' )
						{
							if ( argc > 1 )
							{
								i++;
								argc--;
								next = 1;
								if ( isdigit ( *argv [ i ] ))
								{
									RomNumber = atoi ( argv[i] );
									if ( RomNumber < 16 )
										PageWrite [ RomNumber ] = 1;
									else
									{
										fprintf ( stderr, "ROM no. too big\n");
										exit ( 1 );
									}
								}
								break;
							}

							fprintf ( stderr, "syntax error\nrom number " );
							fprintf ( stderr, "should follow -w\n" );
							exit ( 1 );
						}
						else
						{
							if ( isdigit ( argv [ i ][ j ]))
							{
								RomNumber = atoi ( &argv[ i ][ j ]);
								if ( RomNumber < 16 )
									PageWrite [ RomNumber ] = 1;
								else
								{
									fprintf ( stderr, "ROM no. too big\n");
									exit ( 1 );
								}
							}
							else
							{
								fprintf( stderr, "syntax error\nrom number ");
								fprintf ( stderr, "should follow -w\n" );
								exit ( 1 );
							}
						}
						next = 1;
						break;

					case 's' :					/* Set keyboard DIP switches */
						if ( argv [ i ][ j ] == '\0' )
						{
							if ( argc > 1 )
							{
								i++;
								argc--;
								next = 1;

								DIPSwitches = strtoul ( argv [ i ], &end, 0 );
								if ( end == argv[i] || *end != '\0' )
								{
									fprintf( stderr, "syntax error\na number");
									fprintf ( stderr, "should follow -s\n" );
									exit ( 1 );
								}
								break;
							}
							fprintf( stderr, "syntax error\na number");
							fprintf ( stderr, "should follow -s\n" );
							exit ( 1 );
						}
						else
						{
							DIPSwitches = strtoul ( &argv[i][j], &end, 0 );
							if ( end == &argv[i][j] || *end != '\0' )
							{
								fprintf( stderr, "syntax error\na number");
								fprintf ( stderr, "should follow -s\n" );
								exit ( 1 );
							}
						}
						next = 1;
						break;

					case 'm' :					/* Set the start-up MODE */
						if ( argv [ i ][ j ] == '\0' )
						{
							if ( argc > 1 )
							{
								i++;
								argc--;
								next = 1;
								if ( isdigit ( *argv [ i ] ))
									mode = atoi ( argv[i] );
								break;
							}

							fprintf ( stderr, "syntax error\nMODE number " );
							fprintf ( stderr, "should follow -m\n" );
							exit ( 1 );
						}
						else
						{
							if ( isdigit ( argv [ i ][ j ]))
							{
								mode = atoi ( &argv[ i ][ j ]);
							}
							else
							{
								fprintf( stderr, "syntax error\nMODE number ");
								fprintf ( stderr, "should follow -m\n" );
								exit ( 1 );
							}
						}
						next = 1;
						break;

#ifdef	EMUL_FS
					case 'd' :					/* Disk directory/image */
					{
						char		*disk;

						if ( argv [ i ][ j ] == '\0' )
						{
							if ( argc > 1 )
							{
								i++;
								argc--;
								disk = argv [ i ];
								next = 1;
								
							}
							else
							{
								fprintf ( stderr, "syntax error\nfilename " );
								fprintf ( stderr, "should follow -d\n" );
								exit ( 1 );
							}
						}
						else
						{
							disk = &argv[i][j];
							next = 1;
						}
						if ( ChangeDiskDirectory ( disk ))
						{
							fprintf ( stderr, "%s doesn't appear to ", disk );
							fprintf ( stderr, "contain a valid disk image\n" );
							exit ( 1 );
						}
						break;
					}
#endif	/* EMUL_FS */

					case 'a' :				/* Run as BBC Model A */
#ifdef	MODEL_B_ONLY
						fprintf ( stderr, "Support for the Model A machine" );
						fprintf ( stderr, " is not available\n" );
#else
						MaxRAMAddress = 0x3fff;
#endif	/* MODEL_B_ONLY */
						break;

					case 'h' : case '?' :		/* Help message and exit */
						DisplayHelp();
						exit ( 1 );
						break;

					case 'V' :
						fprintf ( stderr, "xbeeb %d.%d patchlevel %d\n",
							VERSION, RELEASE, PATCHLEVEL );
						exit ( 0 );
						break;

					default :					/* Unrecognised option */
						fprintf ( stderr, "unrecognised option '%c'\n", c );
						fprintf ( stderr, "use -h for help\n" );
						exit ( 1 );
						break;
				}
			}
			i++;
		}
		else
		{
			/*
			 * Assume a string on it's own is a snapshot name.
			 * Fall over if we already have one.
			 */

			if ( SnapshotName )
			{
				fprintf ( stderr, "bad option '%s'\n", argv [ i ]);
				fprintf ( stderr, "use -h for help\n" );
				exit ( 1 );
			}
			else
				SnapshotName = argv [ i ];
		}
	}

	/*
	 * Now set up the DIPSwitch variable according to any MODE chosen
	 * using the -m flag.
	 */

	if ( mode >= 0 )
	{
		mode &= 0x07;
		mode ^= 0x07;
		DIPSwitches &= ~0x07;
		DIPSwitches |= mode;
	}

    return;
}


static void
DisplayHelp()
{
	printf ( "xbeeb\t-[Vh?a] [<snapshot>] [-o <OS ROM>]" );
	printf ( "[-p[0-15] <paged ROM>]\n\t[-w <num>] [-s <num>]" );
#ifdef	EMUL_FS
	printf ( " [-d <dir>]" );
#endif
	printf ( "\n\n" );
	printf ( "\t-[h?]\t\tdisplay this help message\n" );
	printf ( "\t-V\t\tdisplay the version of xbeeb\n" );
	printf ( "\t-a\t\trun as a model A machine\n" );
	printf ( "\t-p[<num>] <rom>\tload <rom> as ROM <num> (default 15)\n" );
	printf ( "\t-o <rom>\tload <rom> as the OS\n" );
	printf ( "\t-w <rom>\tmake <rom> writeable\n" );
	printf ( "\t-s <num>\tset the keyboard DIP switches to <num>\n" );
#ifdef	EMUL_FS
	printf ( "\t-d <dir>\tuse <dir> as the disk image directory\n" );
#endif
	printf ( "\n\n" );
	return;
}
