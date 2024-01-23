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


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Patchlevel.h"
#include "Config.h"
#include "Beeb.h"
#include "6502.h"
#include "Display.h"
#include "ADC.h"
#include "Acia.h"
#include "Crtc.h"
#include "Econet.h"
#include "Disk.h"
#include "RomSelect.h"
#include "Fred.h"
#include "Jim.h"
#include "Sheila.h"
#include "Memory.h"
#include "SystemVia.h"
#include "UserVia.h"
#include "Keyboard.h"
#include "Screen.h"
#include "TubeUla.h"
#include "SerialUla.h"
#include "VideoUla.h"
#include "Ops.h"
#include "opcodes.h"

#ifdef	EMUL_FS
#include "EFS.h"
#endif


/*
 * Misc. forward declarations...
 */

void				SaveSnapshot ();
void				RestoreSnapshot ( char* );
void				Beeb();
void				DisplayHelp();

/*
 * This lot are just for convenience so that the real variables can
 * be kept local to the instruction execution loop and thus assigned to
 * registers.
 */

byteval 			InitAccumulator;
byteval				InitRegisterX;
byteval				InitRegisterY;
byteval				InitStackPointer;
unsigned int		InitProgramCounter;
int					NewPC;

/*
 * The status register and the actual values of all the flags.  The
 * StatusRegister variable itself is rarely used and thus isn't
 * put in a register.
 */

byteval				StatusRegister;

byteval				NegativeFlag = 0;
byteval				OverflowFlag = 0;
byteval				DecimalModeFlag = 0;
byteval				IRQDisableFlag = 0;
byteval				ZeroFlag = 0;
byteval				CarryFlag = 0;

/*
 * The 3 jump addresses used by the 6502 -- set up at initialisation
 * once the OS ROM has been loaded.
 */

unsigned int		NMIAddress;
unsigned int		IRQAddress;
unsigned int		ResetAddress;

byteval				SnapshotRequested = 0;
byteval				QuitEmulator = 0;
char				*SnapshotName;

byteval 			MaskableInterruptRequest = 0;
static byteval 		ClockCyclesSoFar = 0;

#define	CheckIRQ() \
	if ( MaskableInterruptRequest && !IRQDisableFlag ) \
	{ \
		/* \
		 * We have to explicitly stack the status register here, otherwise \
		 * it will be pushed on the stack with the BRK flag set. \
		 */ \
 \
		StackPC(); \
		GenerateIRQStatusRegister; \
		StackByte ( StatusRegister ); \
		SetIRQDisableFlag; \
		SetProgramCounter ( IRQAddress ); \
 \
		/* \
		 * Have to add clock cycles ourselves, otherwise we end up with \
		 * mutually recursive references to AddClockCycles and CheckIRQ. \
		 * We can't actually cause an IRQ at this stage, though, because \
		 * interrupts have just been disabled (above). \
		 */ \
 \
		ClockCyclesSoFar += 7; \
		if ( ClockCyclesSoFar > 100 ) \
	    { \
			ViaClockUpdate ( ClockCyclesSoFar ); \
			ClockCyclesSoFar = 0; \
		} \
		MaskableInterruptRequest--; \
	}


#define	AddClockCycles(n) \
{ \
	ClockCyclesSoFar += n; \
	if ( ClockCyclesSoFar > 100 ) \
	{ \
		ViaClockUpdate ( ClockCyclesSoFar ); \
		ClockCyclesSoFar = 0; \
		CheckIRQ(); \
	} \
}


/*
 * Debugging stuff
 */

#ifdef	DISASS
unsigned char		DebugLevel = DISASS;
#else
unsigned char		DebugLevel = 0;
#endif

#ifdef	COUNT_INSTRS
static unsigned long	InstrCount [ 256 ];
#endif

int
main ( int argc, char** argv )
{
	char		c;
	byteval		RomNumber;
	int			i = 1, j, next;
	char		*OsRomName = OS_ROM;
	char		*Rom15Name = LANG_ROM;
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

							fprintf ( stderr, "syntax error\n rom number " );
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
								fprintf( stderr, "syntax error\n rom number ");
								fprintf ( stderr, "should follow -w\n" );
								exit ( 1 );
							}
						}
						next = 1;
						break;

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
	 * Load the OS and initialise the CPU jump vectors.
	 */

	LoadOS ( OsRomName );
	NMIAddress = ReadWord ( NMI_VECTOR );
	IRQAddress = ReadWord ( IRQ_VECTOR );
	ResetAddress = ReadWord ( RESET_VECTOR );

	/*
	 * Load the default language ROM
	 */

	LoadPagedRom ( Rom15Name, 15 );

	InitialiseKeyboard();
	InitialiseScreen();

	Beeb();

#ifdef	COUNT_INSTRS

	for ( i = 0; i < 256; i++ )
	{
		printf ( "Executed opcode %02x %ld times\n", i, InstrCount [ i ] );
	}

#endif

	return 0;
}


void
DisplayHelp()
{
	printf ( "xbeeb -[Vh?] [<snapshot>] [-o <OS ROM>]" );
	printf ( " [-p[0-15] <paged ROM>] [-w <num>]\n" );
	printf ( "\t-[h?]\t\tdisplay this help message\n" );
	printf ( "\t-V\t\tdisplay the version of xbeeb\n" );
	printf ( "\t-p[<num>] <rom>\tload <rom> as ROM <num> (default 15)\n" );
	printf ( "\t-o <rom>\tload <rom> as the OS\n" );
	printf ( "\t-w <rom>\tmake <rom> writeable\n" );
	printf ( "\n\n" );
	return;
}


void
Beeb()
{
	/*
	 * The real variables used to store the CPU registers...
	 */

	register byteval		*EmulatorPC;
	register byteval		Accumulator;
	register byteval		RegisterX;
	register byteval		RegisterY;
	register byteval		StackPointer;

	unsigned int			OpCount = 0;

	InitProgramCounter = ResetAddress;

	/*
	 * Now load a snapshot if one was specified.
	 */

	if ( SnapshotName )
		RestoreSnapshot ( SnapshotName );

	Accumulator = InitAccumulator;
	RegisterX = InitRegisterX;
	RegisterY = InitRegisterY;
	StackPointer = InitStackPointer;
	SetProgramCounter ( InitProgramCounter );

	while ( 1 )
	{

		unsigned int	opcode;

/*
 * The 6502 execute instruction loop is in a different source file
 * purely for reasons of maintainability.
 */

#include "6502.c"

		if (( ++OpCount & 0x7ff ) == 0 )
		{
			CheckEvents();
			if ( SnapshotRequested )
			{
				InitAccumulator = Accumulator;
				InitRegisterX = RegisterX;
				InitRegisterY = RegisterY;
				InitStackPointer = StackPointer;
				InitProgramCounter = GetProgramCounter;

				/*
				 * Need to do this to make sure everything is fully
				 * up to date.
				 */

				ViaClockUpdate ( ClockCyclesSoFar );

				SaveSnapshot();
				SnapshotRequested = 0;
			}
			else
				if ( QuitEmulator )
					break;
		}

#ifdef	LIMIT
	} while ( OpCount < LIMIT );
#else
	}
#endif

	ShutdownScreen();
	return;
}


#ifndef	AddClockCycles
#ifdef	FASTCLOCK

void
AddClockCycles ( byteval val )
{
	ClockCyclesSoFar += val;
	if ( ClockCyclesSoFar > 100 )
	{
		ViaClockUpdate ( ClockCyclesSoFar );
		ClockCyclesSoFar = 0;
	}

	return;
}

#else

void
AddClockCycles ( byteval val )
{
	ViaClockUpdate ( val );
	return;
}

#endif	/* FASTCLOCK */
#endif	/* AddClockCycles */


void
SaveSnapshot ()
{
	int						fd, done, ilen, xlen;
	static unsigned char	MagicNo[4] = { 0x0b, 0xbc, 0x00, 0x01 };
	char					InputBuffer [ PATH_MAX ];
	static char				*Home = 0;
	char					*p;

	/*
	 * Get the value of $HOME
	 */

	if ( !Home )
		Home = getenv ( "HOME" );

	/*
	 * Get the snapshot filename
	 */

	done = 0;
	while ( !done )
	{
		printf ( "Snapshot name : " );
		if ( fgets ( InputBuffer, PATH_MAX - 1, stdin ) == 0 )
		{
			perror ( "Input error : " );
			fprintf ( stderr, "Try again\n" );
			continue;
		}

		if ( *InputBuffer == '\n' || *InputBuffer == '\0' )
			strcpy ( InputBuffer, DEF_SNAPSHOT );
		else
			* ( strchr ( InputBuffer, '\n' )) = '\0';

		switch ( *InputBuffer )
		{
			case '+' :
			{
				/*
				 * snapshot file is relative to XBEEBSNAPS
				 */

				char		tmp [ PATH_MAX ];

				if ((strlen( InputBuffer) + strlen(XBEEBSNAPS) - 1) > PATH_MAX)
				{
					fprintf ( stderr, "The pathname is too long\n" );
					fprintf ( stderr, "Try again\n" );
					continue;
				}

				( void ) strcpy ( tmp, & ( InputBuffer[1] ));
				( void ) strcpy ( InputBuffer, XBEEBSNAPS );
				( void ) strcat ( InputBuffer, tmp );
				break;
			}
			case '~' :
			{
				/*
				 * snapshot file is relative to $HOME
				 */

				char		tmp [ PATH_MAX ];

				if (( strlen ( InputBuffer ) + strlen ( Home ) - 1) > PATH_MAX)
				{
					fprintf ( stderr, "The pathname is too long\n" );
					fprintf ( stderr, "Try again\n" );
					continue;
				}

				( void ) strcpy ( tmp, & ( InputBuffer[1] ));
				( void ) strcpy ( InputBuffer, Home );
				( void ) strcat ( InputBuffer, tmp );
				break;
			}
			default :		/* full path, relative path -- no change */
				break;
		}

		if (( ilen = strlen ( InputBuffer )) > ( xlen = strlen(SNAPSHOT_EXT)))
		{
			p = InputBuffer + ilen - xlen;
			if ( strcmp ( p, SNAPSHOT_EXT ))
			{
				if (( ilen + xlen ) > PATH_MAX )
				{
					fprintf ( stderr, "The pathname is too long\n" );
					fprintf ( stderr, "Try again\n" );
					continue;
				}

				( void ) strcat ( InputBuffer, SNAPSHOT_EXT );
			}
			done = 1;
		}
		else
		{
			/*
			 * POSSIBLE ENHANCEMENT ?
			 *
			 * Could check strings for overflow here, but we already know
			 * that InputBuffer is no longer than SNAPSHOT_EXT, and is
			 * SNAPSHOT_EXT really in danger of overflowing things ?
			 *
			 * No, I don't think so either :-)
			 */

			( void ) strcat ( InputBuffer, SNAPSHOT_EXT );
			done = 1;
		}

		if ( done )
		{
			printf ( "Saving snapshot : %s\n", InputBuffer );
		}
	}

	if (( fd = open ( InputBuffer, O_WRONLY | O_TRUNC | O_CREAT, 0644 )) < 0 )
	{
		fprintf ( stderr, "Couldn't create snapshot file\n" );
		FatalError();
	}

	if ( write ( fd, MagicNo, 4 ) != 4 )
	{
		fprintf ( stderr, "Couldn't write to snapshot file\n" );
		unlink ( InputBuffer );
		FatalError();
	}

	if ( SaveUserMemory ( fd ) < 0 )
	{
		fprintf ( stderr, "Couldn't write to snapshot file\n" );
		unlink ( InputBuffer );
		FatalError();
	}

	if ( SaveCPU ( fd ) < 0 )
	{
		fprintf ( stderr, "Couldn't write to snapshot file\n" );
		unlink ( InputBuffer );
		FatalError();
	}

	if ( SaveADC ( fd ) < 0 )
	{
		fprintf ( stderr, "Couldn't write to snapshot file\n" );
		unlink ( InputBuffer );
		FatalError();
	}

	if ( SaveACIA ( fd ) < 0 )
	{
		fprintf ( stderr, "Couldn't write to snapshot file\n" );
		unlink ( InputBuffer );
		FatalError();
	}

	if ( SaveKeyboard ( fd ) < 0 )
	{
		fprintf ( stderr, "Couldn't write to snapshot file\n" );
		unlink ( InputBuffer );
		FatalError();
	}

	if ( SaveRomSelect ( fd ) < 0 )
	{
		fprintf ( stderr, "Couldn't write to snapshot file\n" );
		unlink ( InputBuffer );
		FatalError();
	}

	if ( SaveSerialUla ( fd ) < 0 )
	{
		fprintf ( stderr, "Couldn't write to snapshot file\n" );
		unlink ( InputBuffer );
		FatalError();
	}

	if ( SaveSystemVia ( fd ) < 0 )
	{
		fprintf ( stderr, "Couldn't write to snapshot file\n" );
		unlink ( InputBuffer );
		FatalError();
	}

	if ( SaveUserVia ( fd ) < 0 )
	{
		fprintf ( stderr, "Couldn't write to snapshot file\n" );
		unlink ( InputBuffer );
		FatalError();
	}

	if ( SaveVideoUla ( fd ) < 0 )
	{
		fprintf ( stderr, "Couldn't write to snapshot file\n" );
		unlink ( InputBuffer );
		FatalError();
	}

	/*
	 * It's easiest to do the CRTC last, because we now have the information
	 * from the video ULA about the screen memory addresses etc. which
	 * we need to calculate the top of screen addresses in the CRTC.
	 */

	if ( SaveCRTC ( fd ) < 0 )
	{
		fprintf ( stderr, "Couldn't write to snapshot file\n" );
		unlink ( InputBuffer );
		FatalError();
	}

	close ( fd );

	printf ( "snapshot saved\n" );
}


void
RestoreSnapshot ( char* sname )
{
	int				fd, l;
	char			*p;
	char			FnameBuffer [ PATH_MAX ];
	char			SnapName [ NAME_MAX ];
	unsigned char	MagicNo[4];
	unsigned int	Version;

	/*
	 * If the filename doesn't have the .sst suffix, then append it
	 */

	if (( l = strlen ( sname )) > PATH_MAX )
	{
		fprintf ( stderr, "snapshot path is too long -- ignoring it\n" );
		return;
	}

	( void ) strcpy ( SnapName, sname );
	p = SnapName + l - 4;
	if ( l < 4 || strncmp ( p, ".sst", 4 ) != 0 )
	{
		if (( l + 4 ) > PATH_MAX )
		{
			fprintf ( stderr, "snapshot path is too long -- ignoring it\n" );
			return;
		}
		( void ) strcat ( SnapName, ".sst" );
		l += 4;
	}

	/*
	 * Try locally first, then prepend the snapshot directory.
	 */

	if (( fd = open ( SnapName, O_RDONLY )) < 0 )
	{
		if (( l + strlen ( XBEEBSNAPS )) <= PATH_MAX )
		{
			( void ) strcpy ( FnameBuffer, XBEEBSNAPS );
			( void ) strcat ( FnameBuffer, SnapName );

			if (( fd = open ( FnameBuffer, O_RDONLY )) < 0 )
			{
				fprintf ( stderr, "Couldn't open snapshot file\n" );
				exit ( 1 );
			}
		}
	}

	if ( read ( fd, MagicNo, 4 ) != 4 )
	{
		fprintf ( stderr, "Couldn't read snapshot file\n" );
		exit ( 1 );
	}

	if ( MagicNo [ 0 ] != 0x0b || MagicNo [ 1 ] != 0xbc )
	{
		fprintf ( stderr, "%s doesn't look like a snapshot file\n", SnapName );
		exit ( 1 );
	}

	Version = ( MagicNo [ 2 ] << 8 ) + MagicNo [ 3 ];

	if ( Version > 1 )
	{
		fprintf ( stderr, "Snapshot version too high\n" );
		exit ( 1 );
	}

	/*
	 * These functions will be executed for every version.
	 */

	if ( RestoreUserMemory ( fd, Version ) < 0 )
	{
		fprintf ( stderr, "Couldn't read snapshot file\n" );
		exit ( 1 );
	}

	if ( RestoreCPU ( fd, Version ) < 0 )
	{
		fprintf ( stderr, "Couldn't read snapshot file\n" );
		exit ( 1 );
	}

	if ( RestoreADC ( fd, Version ) < 0 )
	{
		fprintf ( stderr, "Couldn't read snapshot file\n" );
		exit ( 1 );
	}

	if ( RestoreACIA ( fd, Version ) < 0 )
	{
		fprintf ( stderr, "Couldn't read snapshot file\n" );
		exit ( 1 );
	}

	if ( RestoreKeyboard ( fd, Version ) < 0 )
	{
		fprintf ( stderr, "Couldn't read snapshot file\n" );
		exit ( 1 );
	}

	if ( RestoreRomSelect ( fd, Version ) < 0 )
	{
		fprintf ( stderr, "Couldn't read snapshot file\n" );
		exit ( 1 );
	}

	if ( RestoreSerialUla ( fd, Version ) < 0 )
	{
		fprintf ( stderr, "Couldn't read snapshot file\n" );
		exit ( 1 );
	}

	if ( RestoreSystemVia ( fd, Version ) < 0 )
	{
		fprintf ( stderr, "Couldn't read snapshot file\n" );
		exit ( 1 );
	}

	if ( RestoreUserVia ( fd, Version ) < 0 )
	{
		fprintf ( stderr, "Couldn't read snapshot file\n" );
		exit ( 1 );
	}

	if ( RestoreVideoUla ( fd, Version ) < 0 )
	{
		fprintf ( stderr, "Couldn't read snapshot file\n" );
		exit ( 1 );
	}

	if ( RestoreCRTC ( fd, Version ) < 0 )
	{
		fprintf ( stderr, "Couldn't read snapshot file\n" );
		exit ( 1 );
	}

	/*
	 * Now should come functions which are only executed for
	 * later snapshot file versions
	 */

	close ( fd );

	/*
	 * Finally, stuff we can't do any earlier because we need data from
	 * various places in the hardware...
	 */

	RestoreColourMap();
}


#ifndef	IRQ
void
IRQ()
{
	MaskableInterruptRequest++;
}
#endif


int
SaveCPU ( int fd )
{
	byteval		cpu [ 16 ];

	GenerateStatusRegister;
	cpu [ 0 ] = InitAccumulator;
	cpu [ 1 ] = InitRegisterX;
	cpu [ 2 ] = InitRegisterY;
	cpu [ 3 ] = StatusRegister;
	cpu [ 4 ] = InitStackPointer;
	cpu [ 5 ] = InitProgramCounter & 0xff;
	cpu [ 6 ] = InitProgramCounter >> 8;
	cpu [ 7 ] = MaskableInterruptRequest;

	if ( write ( fd, cpu, 16 ) != 16 )
		return -1;

	return 0;
}


int
RestoreCPU ( int fd, unsigned int ver )
{
	byteval		cpu [ 16 ];

	if ( ver > 1 )
		return -1;

	if ( read ( fd, cpu, 16 ) != 16 )
		return -1;

	InitAccumulator = cpu [ 0 ];
	InitRegisterX = cpu [ 1 ];
	InitRegisterY = cpu [ 2 ];
	StatusRegister = cpu [ 3 ];
	InitStackPointer = cpu [ 4 ];
	InitProgramCounter = cpu [ 5 ] + ( cpu [ 6 ] << 8 );
	MaskableInterruptRequest = cpu [ 7 ];

	GenerateStatusFlags;

	return 0;
}


void
FatalError()
{
	/*
	 * Just tidy up the X display and exit with non-zero
	 * status.
	 */

	ShutdownScreen();
	exit ( 1 );
}
