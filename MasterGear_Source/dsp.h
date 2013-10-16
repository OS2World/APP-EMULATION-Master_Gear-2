// SpiceWare Sound Demo for OS/2
//
// Digital Signal Processor routines, header file
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


#ifndef DSP_H
#define DSP_H

void FillBuffer(void);    // update audio buffer
void SoundInit(unsigned long freq, int blen, unsigned char * bloc);     // initialize all sound
void SoundCleanUp(void);  // deallocate buffer
void Play(int Channel, long int Freq,int Volume);
void PlayFreq(int Channel, int Freq);
void PlayVol(int Channel, int Volume);

#endif // DSP_H
