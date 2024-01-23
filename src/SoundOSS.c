/*
 *
 * $Id: SoundOSS.c,v 1.5 2002/01/15 15:46:43 james Exp $
 *
 * Copyright (C) James Fidell 2000-2002.
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
 * $Log: SoundOSS.c,v $
 * Revision 1.5  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.4  2002/01/14 22:18:51  james
 * Added support for .inf files
 *
 * Revision 1.3  2002/01/13 22:27:19  james
 * Fix compile-time warnings
 *
 * Revision 1.2  2000/09/06 11:41:13  james
 * First cut at "proper" sound code
 *
 * Revision 1.1  2000/09/02 18:48:26  james
 * Changed all VoxWare references to OSS
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include "Config.h"

#ifdef	SOUND_OSS

#include <sys/soundcard.h>

#include "Sound.h"
#include "SoundOSS.h"

#define	AUDIODEV		"/dev/dsp"

#define	SAMPLERATE		44100
#define	SAMPLESIZE		256

#define	VOLUME_SHIFT	2
#define	CLOCK_MULT		2		/* SN76489 clock is 2x CPU clock */

#undef	SOUND_DEBUG

static void				ReloadSampleBuffer();
static void				FillSampleBuffer();
static unsigned int		CalcFreq( unsigned int );

static int				SoundEnabled = 0;
static int				SoundFD;
static unsigned char	SoundCtrl, SoundChan;
static byteval			VolVal[4] = { 0, 0, 0, 0 };
static byteval			Chan1Noise, SoundFb;
static unsigned long	SoundSampleUsec;
static unsigned int		HalfPeriod[4];
static unsigned int		Samples[4];
static unsigned int		Volume[4] = { 0, 0, 0, 0 };
static byteval			SampleState[4] = { 0, 0, 0, 0 };
static unsigned int		SampleCounter[4];
static int				SoundPlaying = 0;

static unsigned char	SoundSample[SAMPLESIZE];


void
InitialiseOSS()
{
	unsigned int		dummy;

	if (( SoundFD = open ( AUDIODEV, O_WRONLY )) <= 0 ) {
		fprintf ( stderr, "Can't open %s.  Sound is disabled\n", AUDIODEV );
		return;
	}

	if ( ioctl ( SoundFD, SNDCTL_DSP_RESET ) < 0 ) {
		fprintf ( stderr, "Can't reset sound device\n" );
		return;
	}

	dummy = AFMT_U8;
	if ( ioctl ( SoundFD, SNDCTL_DSP_SETFMT, &dummy ) < 0 ) {
		fprintf ( stderr, "Can't set sound sample format\n" );
		return;
	}

	dummy = 1;
	if ( ioctl ( SoundFD, SNDCTL_DSP_CHANNELS, &dummy ) < 0 ) {
		fprintf ( stderr, "Can't set mono sound\n" );
		return;
	}

	dummy = SAMPLERATE;
	if ( ioctl ( SoundFD, SNDCTL_DSP_SPEED, &dummy ) < 0 ) {
		fprintf ( stderr, "Can't set correct DSP rate\n" );
		return;
	}

	SoundEnabled = 1;

	/*
	 * SoundSampleUsec is the length of the SoundSample buffer in terms of
	 * microseconds.
	 */

	SoundSampleUsec = ( double ) SAMPLESIZE / SAMPLERATE * 1000000 * 0.4;
}



void
OSSWrite ( unsigned char ora )
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
			VolVal [ SoundChan ] = ( ora & 0xf ) ^ 0xf;
			Volume [ SoundChan ] = VolVal [ SoundChan ] << VOLUME_SHIFT;

#ifdef SOUND_DEBUG
			printf("chan %d vol. = %d\n", SoundChan, VolVal [ SoundChan ]);
#endif

/*
			if ( VolVal[SoundChan]) ReloadSampleBuffer();
*/

#ifdef SOUND_DEBUG
			printf ( "1 Ch. %d, vol = %d,", SoundChan, VolVal [ SoundChan ]);
			printf ( " half period = %d\n", HalfPeriod[ SoundChan ]);
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
						Samples[0] = 86;	/* SAMPLERATE / 512 */
						Chan1Noise = 0;
						break;
			    	case 0x1 : /* "medium" frequency */
						Samples[0] = 43;	/* SAMPLERATE / 1024 */
						Chan1Noise = 0;
						break;
			    	case 0x2 : /* "high" frequency */
						Samples[0] = 22;	/* SAMPLERATE / 2048 */
						Chan1Noise = 0;
						break;
			    	case 0x3 : /* frequency controlled by sound channel 1 */
						Samples[0] = Samples[1];
						Chan1Noise = 1;
						break;
				}
				SampleCounter[0] = 0;
				SampleState[0] = 0;
				if ( VolVal[0] ) ReloadSampleBuffer();
			}
		    else
			{
				HalfPeriod[SoundChan] = ( HalfPeriod[SoundChan] & 0x3f0 )
					| ( ora & 0xf );
/*
				Samples[SoundChan] = 16 * ( double ) HalfPeriod[SoundChan] *
					SAMPLERATE / 1000000 / CPU_SPEED / CLOCK_MULT + 0.5;
				if ( !Samples[SoundChan] ) Samples[SoundChan] = 1;
				SampleCounter[SoundChan] = 0;
				SampleState[SoundChan] = 0;
				ReloadSampleBuffer();
*/
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
			HalfPeriod[ SoundChan ] = ( HalfPeriod [ SoundChan ] & 0xf ) |
				(( ora & 0x3f ) << 4 );
			Samples[SoundChan] = 16 * ( double ) HalfPeriod[SoundChan] *
				SAMPLERATE / 1000000 / CPU_SPEED / CLOCK_MULT + 0.5;
			if ( !Samples[SoundChan] ) Samples[SoundChan] = 1;
			SampleCounter[SoundChan] = 0;
			SampleState[SoundChan] = 0;
			if ( Chan1Noise && SoundChan == 1 ) {
				Samples[0] = Samples[1];
				SampleCounter[0] = 0;
				SampleState[0] = 0;
			}

#ifdef SOUND_DEBUG
			printf ( "2 Ch. %d, vol = %d,", SoundChan, VolVal [ SoundChan ]);
			printf ( " half period. = %d\n", HalfPeriod [ SoundChan ]);
#endif

			if ( VolVal[SoundChan] ) ReloadSampleBuffer();
		}
	}

#ifdef SOUND_DEBUG				   
	printf ( "Exit sound:\n" );
	printf ( "SoundCtrl = 0x%x, SoundChan = 0x%x\n", SoundCtrl, SoundChan ); 
#endif

	return;
}


static void
ReloadSampleBuffer()
{
	FillSampleBuffer();
	if ( !SoundPlaying ) OSSSoundRefresh();
	return;
}


void
OSSSoundRefresh()
{
	struct timeval		tv;

	if ( VolVal[0] || VolVal[1] || VolVal[2] || VolVal[3] ) {
		if ( write ( SoundFD, SoundSample, SAMPLESIZE ) < 0 )
			fprintf ( stderr, "error on sound write\n" );
		SoundPlaying = 1;
	} else
		SoundPlaying = 0;
	gettimeofday ( &tv, 0 );
	SoundCallSecs = tv.tv_sec;
	SoundCallUsecs = tv.tv_usec;
	if (( SoundCallUsecs += SoundSampleUsec ) > 1000000 ) {
		SoundCallSecs++; 
		SoundCallUsecs -= 1000000;
	}
	FillSampleBuffer();
	return;
}


void
FillSampleBuffer()
{
	unsigned int		i, c, ampl;
	static int			pnoise = 0;

	for ( i = 0; i < SAMPLESIZE; i++ ) {
		for ( ampl = 0, c = 0; c < 4; c++ ) {
			if ( !SampleCounter[c]-- ) {
				SampleCounter[c] = Samples[c];
				if ( c )
					SampleState[c] ^= 1;
				else {
					if ( SoundFb )
						SampleState[c] = rand() & 1;
					else {
						if ( pnoise++ == 10240 ) {
							srand(time(0));
							pnoise = 0;
						}
						SampleState[c] = rand() & 1;
					}
				}
			}
			if ( SampleState[c] )
				ampl += Volume[c];
		}
		SoundSample[i] = ampl;
	}
}

#endif	/* SOUND_OSS */
