// SpiceWare Sound Demo for OS/2
//
// OS2 specific Sound Routines

#include <string.h>
#include <stdlib.h>
#include "SndOS2.h"
#include "dsp.h"

#include <stdio.h>
#include <float.h>
#define INCL_WINDIALOGS   // OS/2 include for Window Dialog
#include <os2.h>
#define  INCL_OS2MM
#include <os2me.h>

int buflen;
unsigned long AUDIOfreq;
int AudioOK = FALSE;
//  samp_rate = 11025;
//  samp_rate = 22050;
//  samp_rate = 44100;
//  samp_rate =  8000;

unsigned char *Buf;

/* PlayList Entry */

typedef struct _PLE {
    ULONG operation;
    ULONG operand1;
    ULONG operand2;
    ULONG operand3;
}PLE;


unsigned long samp_rate;
long bits = 8;
int quiet = FALSE;
int chipmunk = FALSE;
MCI_OPEN_PARMS mop;
PLE playlist[6];
MCI_PLAY_PARMS mpp;
MCI_GENERIC_PARMS mgp;
MCI_WAVE_SET_PARMS wsp;
ULONG rc;


void mci_err(unsigned long rc)
{
    char rbuff[128];
    unsigned long rc2;
    char Text[1000];
    char Title[100];
    strcpy(Title, "MCI Error has occurred!");

    rc2 = mciGetErrorString(rc,      // error code
                                  rbuff,   // return buffer
                                  128);  // rbuff size
    if (rc2 == MCIERR_SUCCESS)
        sprintf(Text,"Error: %s",rbuff);
    else
        sprintf(Text,"Error #: %d", rc);
    WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, Text, Title, 0, MB_OK | MB_INFORMATION );
}

int audio_init(unsigned long freq, int blen)
{
   int rate;
   AudioOK = FALSE;
   if ((freq == 0) || (blen == 0))
      return AudioOK;

   AUDIOfreq = freq;
   buflen = blen;
   Buf = (unsigned char*) malloc(blen);
   memset(Buf,0,blen);

   rate = 8;
   samp_rate = AUDIOfreq;
   bits = 8;  // bits per sample

   playlist[0].operation = DATA_OPERATION;
   playlist[0].operand1  = (long) Buf;
   playlist[0].operand2  = buflen;  // size of sample
   playlist[0].operand3  = 0;

   playlist[1].operation = BRANCH_OPERATION;
   playlist[1].operand1  = 0;
   playlist[1].operand2  = 0;  // goto playlist[0]
   playlist[1].operand3  = 0;

   playlist[2].operation = EXIT_OPERATION;

   mop.hwndCallback   = 0;
   mop.usDeviceID     = 0;
   mop.pszDeviceType  = MCI_DEVTYPE_WAVEFORM_AUDIO_NAME;
   mop.pszElementName = (char *)&playlist[0];

   rc = mciSendCommand(0,
                       MCI_OPEN,            // open message
                       MCI_WAIT | MCI_OPEN_SHAREABLE | // message flags
                       MCI_OPEN_PLAYLIST,
                       &mop,                // parameters
                       0);
   if (rc != MCIERR_SUCCESS)
   {
      mci_err(rc);
      return AudioOK;
   }

   // set device parameters
    wsp.hwndCallback    = 0;
    wsp.ulSamplesPerSec = samp_rate;
    wsp.usBitsPerSample = bits;
    rc = mciSendCommand(mop.usDeviceID,
                       MCI_SET,
                       MCI_WAIT |
                       MCI_WAVE_SET_SAMPLESPERSEC |
                       MCI_WAVE_SET_BITSPERSAMPLE,
                       &wsp,
                       0);

     if (rc != MCIERR_SUCCESS)
     {
        mci_err(rc);
        return AudioOK;
     }

     mpp.hwndCallback = 0;
     rc = mciSendCommand(mop.usDeviceID,
                         MCI_PLAY,
                         0,
                         &mpp,
                         0);
     if (rc != MCIERR_SUCCESS)
     {
       mci_err(rc);
       return AudioOK;
     }
   AudioOK = TRUE;
   SoundInit(freq, blen, Buf);
   return AudioOK;
}



void audio_close(void)
{
   if (Buf) free(Buf);
   Buf = NULL;
   SoundCleanUp();

   if (!AudioOK) return;
   playlist[1].operand2 = 2;

   mgp.hwndCallback = 0;
   rc = mciSendCommand(mop.usDeviceID, MCI_CLOSE, MCI_WAIT, &mgp, 0);
   if (rc != MCIERR_SUCCESS) mci_err(rc);
   AudioOK = FALSE;
}


void audio_quiet(void)
{
   Play(0,0,0);
   Play(1,0,0);
   Play(2,0,0);
   Play(3,0,0);
   FillBuffer();
}
