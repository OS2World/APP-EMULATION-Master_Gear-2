// SpiceWare Sound Demo for OS/2 ver 1.0 released August 24, 1997
//
// Digital Signal Processor routines
//
// These routines are passed channel, frequency, and volume values
// which are used to generate a sound sample to be played back in
// a loop.  The routines are highly optimized versions based on
// Marat Fuyzullin's DSP routines.  The original routines can handle
// any number of sound channels, producing 8 or 16 bit audio, and
// each channel can be set to a sound type of tone or noise.  These
// routines are optimized for speed and will only produce 4 channel
// sound for 8 bit audio, with channels 0, 1, and 2 producing tones
// and channel 3 producing noise.
//
// Marat Fuyzullin's DSP routines which can be found at:
// http://freeflight.com/fms/EMUL8/

#include "DSP.h"
#include <stdlib.h>
#include <string.h>

#define NUM_CHANNELS 4 // Number of channels, these routines are optimized
                       // for 4 channels, do not change.  See Marat's routines
                       // for support for a variable number of channels
#define TRUE 1
#define FALSE 0

int dsp_buflen;
unsigned long dsp_freq;
int DSPok = FALSE;
int volume = 2;

static int  NoiseGen     = 1;

static long int  Freq[NUM_CHANNELS];
static int  Count[NUM_CHANNELS];
static int  K[NUM_CHANNELS];
static int  Kdiv2[NUM_CHANNELS];
static int  L[NUM_CHANNELS];
unsigned char *BufLoc = NULL;
static int FreqCount;

void SoundInit(unsigned long freq, int blen, unsigned char * bufl)
{
   int J;
   dsp_freq = freq;
   dsp_buflen = blen;

   BufLoc = bufl;

   FreqCount=freq/blen;

   for(J=0;J<NUM_CHANNELS;J++)
   {
      Freq[J]   = 0;
      Count[J]  = 0;
      K[J]      = 1;
      Kdiv2[J]  = 0;
      L[J]      = 0;

   }
   DSPok = TRUE;
}


void SoundCleanUp(void)
{
   DSPok = FALSE;
}



void FillBuffer(void)
{
   static int I;  // static is faster

   if (!DSPok) return; // if audio disabled, no point in wasting time

   for(I=0;I<dsp_buflen;I++)
   {
      Count[0]=(Count[0]+1)%K[0];
      Count[1]=(Count[1]+1)%K[1];
      Count[2]=(Count[2]+1)%K[2];
      if(!(Count[3]=(Count[3]+1)%K[3]))
         if((NoiseGen<<=1)&0x80000000)
            NoiseGen^=0x00040001;
      BufLoc[I] = 128 + (Count[0]<Kdiv2[0] ? L[0]:-L[0]) +
                        (Count[1]<Kdiv2[1] ? L[1]:-L[1]) +
                        (Count[2]<Kdiv2[2] ? L[2]:-L[2]) +
                        (NoiseGen&1        ? L[3]:-L[3]);
   }
}



void Play(int C,long int F,int V) // set (C)hannel to (F)requency and (V)olume passed
{
   if(C<NUM_CHANNELS)
   {
      if(F>dsp_freq)        // if frequency is > sound card's rate, it won't
         F=dsp_freq;        // be audible, so set it to the sound card's rate
      if (F > 0)
         K[C]=dsp_freq/F+1;
      else
      {
         K[C]=1;
            V=0;
      }
      Kdiv2[C] = K[C] / 2;

      if (V > 31) V = 31;    // if volume too high, set to max
      if (V < 0)  V = 0;     // if volume to low, set to min
      Freq[C]=F;             // save channel frequency
      L[C]=V * volume;       // save sound wave's amplitude
   }
}
