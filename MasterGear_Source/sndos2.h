// SpiceWare Sound Demo for OS/2
//
// OS2 specific Sound Routines

#ifndef SOUNDOS2_H
#define SOUNDOS2_H

int audio_init (unsigned long freq, int buflen);
void audio_close (void);
void audio_quiet (void);

#endif

