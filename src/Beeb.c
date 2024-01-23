/*
 *
 * $Id: Beeb.c,v 1.25 1996/10/10 21:46:38 james Exp $
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
 * $Log: Beeb.c,v $
 * Revision 1.25  1996/10/10 21:46:38  james
 * Corrected missed conversion of 100 cycles to FASTCLOCK
 *
 * Revision 1.24  1996/10/10 21:45:51  james
 * Tidy up tabbing.
 *
 * Revision 1.23  1996/10/09 22:06:51  james
 * Overhaul of the bitmapped screen handling code with particular respect to
 * colour maps.
 *
 * Revision 1.22  1996/10/07 22:59:45  james
 * Modified FASTCLOCK implementation to allow configuration of the number
 * of instructions between interrupts.
 *
 * Revision 1.21  1996/10/07 22:44:18  james
 * Moved IRQ handling back into the main code rather than #defined as
 * previously -- basically, it doesn't work that way.
 *
 * Revision 1.20  1996/10/07 22:06:31  james
 * Added XDFS ROM & support code from David Ralph Stacey.
 *
 * Revision 1.19  1996/10/01 22:30:31  james
 * Added VoxWare sound code from James Murray <jsm@jsm-net.demon.co.uk>.
 *
 * Revision 1.18  1996/10/01 22:09:57  james
 * Split keyboard handling into kEYMAP_STRICT and KEYMAP_LEGEND models.
 *
 * Revision 1.17  1996/10/01 00:32:58  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.16  1996/09/30 23:46:50  james
 * Correction to load the correct language ROM
 *
 * Revision 1.15  1996/09/30 23:39:29  james
 * Split out option processing into Options.[ch].  Updated the help message,
 * added support for the Model A using the -a switch (and added the
 * MODEL_B_ONLY #define in Config.h, added the -m and -s switches to set the
 * initial screen mode and keyboard DIP switches.
 *
 * Revision 1.14  1996/09/24 23:05:34  james
 * Update copyright dates.
 *
 * Revision 1.13  1996/09/24 22:40:15  james
 * Massive overhaul of instruction decoding code.  Includes :
 *
 *   Correct implementation of (indirect),Y instructions when overflow occurs,
 *   allowing the removal of the RANGE_CHECK directive for those instructions.
 *
 *   Correct handling of address wrap-around for all zp,X and zp,Y
 *   instructions.  This removes the need for the RANGE_CHECK define.  Removed
 *   that, too.
 *
 *   Updated all disassembly instructions to give the full number of hex
 *   digits when displaying their parameters.
 *
 *   Split opcodes.h to give NMOS 6502 opcodes in 6502ops.h and EFS opcodes
 *   in EFSops.h
 *
 *   Add all NMOS 6502 HALT opcodes.
 *
 *   Add all NMOS 6502 NOP opcodes.
 *
 *   Coded for the undocumented NMOS 6502 NOP operations so that
 *   they load a value from memory according to their addressing mode (but
 *   neither store it anywhere nor set any SR flags).
 *
 *   Coded all other undocumented NMOS 6502 operations.
 *
 *   Changed the EFS dummy opcodes because of a clash with the undocumented
 *   NMOS 6502 DCP instructions.  The new trap values are now codes that would
 *   normally halt the CPU.
 *
 *   Added all the R65C02 opcodes.
 *
 *   Added all the R65C12 opcodes.
 *
 *   Correctly coded (zp,X) addressing mode where zp+X(+1) overlaps the
 *   page boundary.
 *
 *   Added #defined values for the number of cycles taken by each instruction
 *   in 6502.h
 *
 *   Added #defines for the original 6502 and Rockwell 65C02 and 65C12.
 *
 * Revision 1.12  1996/09/23 16:33:17  james
 * Tidied up code for exiting the emulator
 *
 * Revision 1.11  1996/09/23 16:21:15  james
 * Improvements to snapshot code.
 *
 * Revision 1.10  1996/09/22 21:00:54  james
 * Changed IRQ-handling code to be called only when I think an IRQ may have
 * happened.  Converted it all to #defines so that it can be compiled inline.
 *
 * Revision 1.9  1996/09/22 20:28:52  james
 * Added Patchlevel.h and -V command line option.
 *
 * Revision 1.8  1996/09/22 20:20:37  james
 * Corrections to DISASS code.
 *
 * Revision 1.7  1996/09/22 19:23:20  james
 * Add the emulated filing system code.
 *
 * Revision 1.6  1996/09/21 23:07:35  james
 * Call FatalError() rather than exit() so that screen stuff etc. can
 * be cleaned up.
 *
 * Revision 1.5  1996/09/21 22:48:21  james
 * Add instruction counting code.
 *
 * Revision 1.4  1996/09/21 22:39:52  james
 * Improved handling of instruction disassembly.
 *
 * Revision 1.3  1996/09/21 22:13:46  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.2  1996/09/21 19:04:09  james
 * Renamed Floppy.[ch] to Disk.[ch]
 *
 * Revision 1.1  1996/09/21 17:20:35  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
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

#include "Config.h"
#include "Beeb.h"
#include "Ops.h"
#include "opcodes.h"
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
#include "Options.h"
#include "Sound.h"
#include "Perms.h"

#ifdef	EMUL_FS
#include "EFS.h"
#endif


/*
 * Obviously this has to come first so that we know if a forward
 * declaration of the AddClockCycles function is required.
 */

#ifdef  FASTCLOCK

#define AddClockCycles(n) \
{ \
	ClockCyclesSoFar += n; \
	if ( ClockCyclesSoFar > FASTCLOCK ) \
	{ \
		ViaClockUpdate ( ClockCyclesSoFar ); \
		ClockCyclesSoFar = 0; \
	} \
}

#else   /* FASTCLOCK */

#define AddClockCycles(n)	ViaClockUpdate(n)

#endif  /* FASTCLOCK */


/*
 * Misc. forward declarations...
 */

void						SaveSnapshot();
void						RestoreSnapshot ( char* );
void						Beeb();
static void					HardwareReset();
#ifndef	AddClockCycles
void						AddClockCycles ( byteval );
#endif

/*
 * This lot are just for convenience so that the real variables can
 * be kept local to the instruction execution loop and thus assigned to
 * registers.
 */

byteval						InitAccumulator;
byteval						InitRegisterX;
byteval						InitRegisterY;
byteval						InitStackPointer;
unsigned int				InitProgramCounter;
int							NewPC;

/*
 * The status register and the actual values of all the flags.  The
 * StatusRegister variable itself is rarely used and thus isn't
 * put in a register.
 */

byteval						StatusRegister;

byteval						NegativeFlag = 0;
byteval						OverflowFlag = 0;
byteval						DecimalModeFlag = 0;
byteval						IRQDisableFlag = 0;
byteval						ZeroFlag = 0;
byteval						CarryFlag = 0;

/*
 * The 3 jump addresses used by the 6502 -- set up at initialisation
 * once the OS ROM has been loaded.
 */

unsigned int				NMIAddress;
unsigned int				IRQAddress;
unsigned int				ResetAddress;

/*
 * Names of files to load which are passed in as command line options.
 */

char						*OsRomName = OS_ROM;
char						*Rom15Name = LANG_ROM;
#ifdef	XDFS
char						*Rom14Name = XDFS_ROM;
#endif
char						*SnapshotName;

unsigned char				SnapshotRequested = 0;
unsigned char				QuitEmulator = 0;
unsigned char				BreakKeypress = 0;

unsigned char				MaskableInterruptRequest = 0;
static unsigned char		ClockCyclesSoFar = 0;


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
#ifdef	COUNT_INSTRS
	int				i;
#endif

	ProcessOptions ( argc, argv );

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
#ifdef	XDFS
	LoadPagedRom ( Rom14Name, 14 );
#endif

	InitialiseKeyboard();
	InitialiseScreen();
	InitialiseSound();

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

	/*
	 * Now load a snapshot if one was specified.
	 */

	if ( SnapshotName )
		RestoreSnapshot ( SnapshotName );
	else
		HardwareReset();

	Accumulator = InitAccumulator;
	RegisterX = InitRegisterX;
	RegisterY = InitRegisterY;
	StackPointer = InitStackPointer;
	SetProgramCounter ( InitProgramCounter );

#ifdef	LIMIT
#define	CONT_CONDITION		OpCount < LIMIT
#else
#define	CONT_CONDITION		1
#endif

	while ( CONT_CONDITION )
	{

		byteval			opcode;

/*
 * The 6502 execute instruction loop is in a different source file
 * purely for reasons of maintainability.
 */

#include "6502.c"

		/*
		 * Now check to see if we need to service an interrupt
		 */

		if ( MaskableInterruptRequest && !IRQDisableFlag )
		{
			/*
			 * We have to explicitly stack the status register here, otherwise
			 * it will be pushed on the stack with the BRK flag set.
			 */

			StackPC();
			GenerateIRQStatusRegister;
			StackByte ( StatusRegister );
			SetIRQDisableFlag;
#ifndef	M6502
			DecimalModeFlag = 0x0;
#endif
			SetProgramCounter ( IRQAddress );
			MaskableInterruptRequest--;
			AddClockCycles ( CLK_INTERRUPT );
		}

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
			{
				if ( QuitEmulator )
					break;
				else
				{
					if ( BreakKeypress )
					{
						do
						{
							CheckEvents();
						} while ( BreakKeypress );

						HardwareReset();
						SetProgramCounter ( InitProgramCounter );
					}
				}
			}
		}
	}

ExitEmulator:

	ShutdownScreen();
	return;
}


static void
HardwareReset()
{
	/*
	 * Do a power-up reset for all the hardware...
	 */

	ResetADConverter();
	ResetAcia();
	ResetCrtc();
	ResetDiskController();
	ResetEconetController();
	ResetSerialUla();
	ResetSystemVia();
	ResetTubeUla();
	ResetUserVia();
	ResetVideoUla();

	/*
	 * Reset for the CPU is easy -- the only thing that gets initialised
	 * is the program counter.
	 */

	InitProgramCounter = ResetAddress;

#ifndef	M6502

	/*
	 * Except for the CMOS processors, which clear the D flag as well.
	 */

	DecimalModeFlag = 0x0;

#endif	/* M6502 */

	return;
}


#ifndef	AddClockCycles
#ifdef	FASTCLOCK

void
AddClockCycles ( byteval val )
{
	ClockCyclesSoFar += val;
	if ( ClockCyclesSoFar > FASTCLOCK )
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
SaveSnapshot()
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

	if (( fd = open ( InputBuffer, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY,
																0644 )) < 0 )
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

	if (( fd = open ( SnapName, O_RDONLY | O_BINARY )) < 0 )
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

	return;
}


#ifndef	IRQ
void
IRQ()
{
	MaskableInterruptRequest++;
}
#endif	/* IRQ */


int
SaveCPU ( int fd )
{
	byteval			cpu [ 16 ];

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
	byteval			cpu [ 16 ];

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
