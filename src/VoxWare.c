/*
 *
 * $Id: VoxWare.c,v 1.1 1996/10/10 21:53:13 james Exp $
 *
 * Copyright (c) James Fidell 1995, 1996.
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
 * $Log: VoxWare.c,v $
 * Revision 1.1  1996/10/10 21:53:13  james
 * Sound implementation for VoxWare systems.
 *
 *
 */


/*
 * The code used in this file is based on code written by James Murray
 * <jsm@jsm-net.demon.co.uk> which I have since hacked about with.
 *
 * Since I'm not as familiar with VoxWare as I'd like to be, there
 * are probably problems with this that I've yet to discover.  I'm
 * not totally happy with the implementation because it uses a sample
 * FM "patch" to generate the tones that the Beeb is producing, rather
 * than just a variable frequency square wave which is what the Beeb
 * does.
 */


#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "Config.h"

#ifdef	VOXWARE_SOUND

#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/soundcard.h>

#include "VoxWare.h"

static void				PlayNote ( int, int, int );
#ifdef	SOUND_DEBUG
static unsigned int		CalcFreq( unsigned int );
#endif

/*
 * This is stuff that VoxWare needs.
 */

extern unsigned char	_seqbuf[];
extern int				_seqbuflen;
extern int				_seqbufptr;
SEQ_DEFINEBUF(1024);


int						SynthDevNo = -1;
int						SequencerFD;
int						SoundEnabled = 0;
struct sbi_instrument	Instr;
unsigned char			SoundCtrl, SoundChan; 
unsigned int			SoundFreq[4][2];
unsigned char			SoundVol[4], SoundNoise, SoundFb;

#define	SEQUENCER		"/dev/sequencer"


#undef SOUND_DEBUG


void
InitialiseVoxWare()
{
	int					NoDevs, i, j;
	struct synth_info	info;

	/*
	 * FIX ME
	 *
	 * These sound patches are just hacks -- what we really want is to
	 * generate square waves of the appropriate freqency.  Also, the
	 * patches for periodic and white noise are the same.
	 */

	int patch [ 3 ][ 51 ] =
	{
		{ 
			0x53, 0x42, 0x49, 0x1a, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x22, 0x00, 0x11, 0x00,
			0xf0, 0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00
		},
		{
			0x53, 0x42, 0x49, 0x1a, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x87, 0x25, 0x80, 0x88,
			0xff, 0xff, 0x00, 0x05, 0x00, 0x00, 0x0e, 0x00,
			0x00, 0x00, 0x00
		},
		{
			0x53, 0x42, 0x49, 0x1a, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x87, 0x25, 0x80, 0x88,
			0xff, 0xff, 0x00, 0x05, 0x00, 0x00, 0x0e, 0x00,
			0x00, 0x00, 0x00
		}
	};

	/*
	 * Need to be able to write to SEQUENCER to support sound
	 */

	if (( SequencerFD = open ( SEQUENCER, O_WRONLY, 0 )) < 0 )
	{
		fprintf ( stderr, "Could not open %s, disabling sound.\n", SEQUENCER );
		return;
	}

	/*
	 * check to see if any synth devices are available
	 */

	ioctl ( SequencerFD, SNDCTL_SEQ_NRSYNTHS, &NoDevs );
	if ( !NoDevs )
	{
		fprintf( stderr, "No synth devices available, disabling sound.\n");
		return;
	}

#ifdef	SOUND_DEBUG
	fprintf ( stderr,"Number of synth devices = %d\n", NoDevs );
#endif

	/*
	 * Now check what types of devices are available
	 */

	for ( i = 0; i < NoDevs; i++ )
	{
		info.device = i;	
		ioctl ( SequencerFD, SNDCTL_SYNTH_INFO, &info );
		if ( info.synth_type == SYNTH_TYPE_FM )
			SynthDevNo = i;
		break;
	}
		
	if ( SynthDevNo < 0 )
	{
		fprintf( stderr, "None of the synths are FM. Sound disabled.\n ");
		return;
  	}

	SoundEnabled = 1;

	info.device = SynthDevNo;
	ioctl ( SequencerFD, SNDCTL_SYNTH_INFO, &info );

#ifdef	SOUND_DEBUG
	printf ( "Synth device %d info:\n", SynthDevNo );
	printf ( "Name = %s, Voices = %d\n", info.name, info.nr_voices );
#endif

	/*
	 * load patches into the kernel
	 */

	for ( j = 0; j < 4; j++ )
	{
		Instr.channel = j;
		Instr.key = FM_PATCH;
		Instr.device = SynthDevNo;
		for ( i = 0; i < 16; i++ )
		{
	  		Instr.operators [ i ] = patch [ j / 3 ][ i + 0x24 ];
		}

		if ( write ( SequencerFD, &Instr, sizeof ( Instr )) == -1)
			perror ( SEQUENCER );

#ifdef SOUND_DEBUG
		fprintf ( stderr, "Loaded %d with patch %d\n", j, j / 3);
#endif
	}

	/*
	 * now set up tone voices to certain patches
	 */

	SEQ_SET_PATCH ( SynthDevNo, 1, 0 );
	SEQ_SET_PATCH ( SynthDevNo, 2, 0 );
	SEQ_SET_PATCH ( SynthDevNo, 3, 0 );
	SEQ_SET_PATCH ( SynthDevNo, 4, 1 );
	SEQ_SET_PATCH ( SynthDevNo, 5, 2 );

	for ( j = 0; j < 6; j++ )
	{
		SEQ_PANNING ( SynthDevNo, j, 0 );
		SEQ_BENDER_RANGE ( SynthDevNo, j, 8192 );
		SEQ_PITCHBEND ( SynthDevNo, j, 0 );
  	}

	return;
}


void
VoxWareWrite ( unsigned char ora )
{
	if ( !SoundEnabled )
		return;

#ifdef SOUND_DEBUG
	printf ( "Enter sound\n" );
	printf ( "Byte 0x%x written to sound chip\n", ora );
	printf ( "SoundCtrl = 0x%x, SoundChan = 0x%x\n", SoundCtrl, SoundChan); 
#endif

	if ( ora & 0x80 )
	{
		SoundCtrl = ( ora >> 4 ) & 0x7;
		SoundChan = 3 - ( SoundCtrl >> 1 );

#ifdef SOUND_DEBUG
		printf ( "Reg. = 0x%x, channel = 0x%x, ", SoundCtrl, SoundChan );
		printf ( "data = 0x%x\n", ora & 0xf );
#endif

		if ( SoundCtrl & 0x1 )
	   	{
			SoundVol [ SoundChan ] = ( ora & 0xf ) ^ 0xf;

#ifdef SOUND_DEBUG
			printf("chan %d vol. = %d\n", SoundChan, SoundVol [ SoundChan ]);
#endif

			if ( SoundVol [ SoundChan ] )
				PlayNote( SoundChan, SoundVol [ SoundChan ],
										SoundFreq [ SoundChan ][1]);
			else
				PlayNote ( SoundChan, 0, SoundFreq [ SoundChan ][1]);

#ifdef SOUND_DEBUG
			printf ( "1 Ch. %d, vol = %d,", SoundChan, SoundVol [ SoundChan ]);
			printf ( " freq. = %d Hz\n",
								CalcFreq ( SoundFreq [ SoundChan ][1]));
#endif
		}
		else
		{
		    if ( !SoundChan )
			{
				if ( ora & 0x4 )
			    {
#ifdef SOUND_DEBUG
			     	printf ( "White noise\n" );
#endif
			     	SoundFb = 1;
			    }
				else
			    {
#ifdef SOUND_DEBUG
			    	printf("Periodic noise\n");
#endif
			    	SoundFb = 0;
			    }

				switch ( ora & 0x3 )
			    {
			    	case 0x0 : /* "low" frequency */
						SoundFreq [0][0] = 0x000;
						break;
			    	case 0x1 : /* "medium" frequency */
						SoundFreq [0][0] = 0x1ff;
						break;
			    	case 0x2 : /* "high" frequency */
						SoundFreq [0][0] = 0x3ff;
						break;
			    	case 0x3 : /* frequency controlled by sound channel 1 */
						SoundNoise = 1;
						SoundFreq [0][0] = SoundFreq[1][1];
						break;
				}
			}
		    else
			{
				/*
				 * FIX ME
				 *
				 * Ought to recalculate the frequency and play the new note
				 * here.
				 */

				SoundFreq [ SoundChan ][0] = ora & 0xf;
			}
		}
	}	
	else
	{
		/*
		 * Continuation byte for frequency of channels 1 thru' 3
		 */

		if (( SoundCtrl & 0x1 ) || ( SoundCtrl >= 0x6 ) )
		{
			fprintf ( stderr, "Bad sound reg. write: SoundCtrl = 0x%x\n",
																SoundCtrl );
		}
		else
		{
			SoundFreq [ SoundChan ] [1]= SoundFreq [ SoundChan ][0] |
													(( ora & 0x3f ) << 4 );

#ifdef SOUND_DEBUG
			printf ( "2 Ch. %d, vol = %d,", SoundChan, SoundVol [ SoundChan ]);
			printf ( " freq. = %d Hz\n",
								CalcFreq ( SoundFreq [ SoundChan ][1]));
#endif

			/*
			 * Play the sound
			 */

			if ( SoundVol [ SoundChan ] )
			{
#ifdef SOUND_DEBUG
				printf("start note\n");
#endif
				PlayNote ( SoundChan, SoundVol [ SoundChan ],
											SoundFreq [ SoundChan ][1] );
		    }
		    else
		    {
#ifdef SOUND_DEBUG
				printf("stop note\n");
#endif
				PlayNote ( SoundChan, 0, 0 );
		    }
		}
	}

#ifdef SOUND_DEBUG				   
	printf ( "Exit sound:\n" );
	printf ( "SoundCtrl = 0x%x, SoundChan = 0x%x\n", SoundCtrl, SoundChan ); 
#endif

	return;
}


static void
PlayNote ( int channel, int volume, int pitch )
{
	float			MidiValue;
	unsigned int	MidiNote;

	if ( !volume )
	{
		SEQ_STOP_NOTE ( SynthDevNo, channel, 0, 0 );
	}
	else
	{
		if ( channel )
		{
   	   		if ( !pitch )
				pitch = 1;

			/*
			 * This next calculation is based on the midi note numbers as
			 * given in the sndkit README, the User Guide and the values
			 * that are passed to this function.
			 *
			 * log(pitch) gave a beautiful straight line when plotted
			 * against the correct midi value.
			 */

			MidiValue = (8.2343 - log(pitch) ) / 0.0579;
			/* MidiNote =  (unsigned int)(30 + ((10 - log(pitch))*4)); */

			MidiNote = ( unsigned int ) MidiValue;
			SEQ_START_NOTE ( SynthDevNo, channel, MidiNote, volume );
		}
    	else
		{
			/*
			 * FIX ME
			 *
			 * Should actually do something here!
			 */

			/*
			 * Noise generator
			 */

#ifdef SOUND_DEBUG
			fprintf ( stderr, "noise generator\n" );
#endif
		}
	}

	SEQ_DUMPBUF();
	return;
}


void
seqbuf_dump()
{
	/*
	 * This function is required by the VoxWare sound stuff...
	 */

	if ( _seqbufptr )
	{
 		if ( write ( SequencerFD, _seqbuf, _seqbufptr ) < 0 )
    		perror("write /dev/sequencer");

		_seqbufptr = 0;
	}
	return;
}


#ifdef	SOUND_DEBUG
static unsigned int
CalcFreq ( unsigned int f )
{
	unsigned int	freq = 0;

	/*
	 * This formula comes from the AUG.
	 */

	if ( f )
 		freq = 4000000 / ( 32 * f );

	return freq;
}
#endif	/* SOUND_DEBUG */

#endif	/* VOXWARE_SOUND */
