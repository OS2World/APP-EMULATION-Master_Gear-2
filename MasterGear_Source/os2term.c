/** Master Gear - Portable Sega Emulator *********************/
/**                                                         **/
/**                           os2.c                         **/
/**                                                         **/
/** This file contains OS/2 dependent subroutines and       **/
/** drivers.                                                **/
/**                                                         **/
/** Copyright (C) Darrell Spice Jr, 1997, 1998              **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/


/** Standard Unix/X #includes ********************************/
#include "SMS.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os2term.h"
#include "os2res.h"
#include "dialog.h"

// sound
#include "DSP.h"
#include "sndos2.h"

#define INCL_WIN
#define INCL_PM
#define INCL_GPI
#define INCL_DOS
#define INCL_DOSPROCESS
#define INCL_WINSTDFILE   /* OS/2 include for File Dialog */
#define INCL_WINDIALOGS   /* OS/2 include for Window Dialog */
#include <os2.h>
#include "joyos2.h"


/* DIVE required header files  */
#define  _MEERROR_H_
#include <mmioos2.h>
#include <dive.h>
#include <fourcc.h>


/* Dive variables */
DIVE_CAPS          DiveCaps           = {0};
FOURCC             fccFormats[100]    = {0};
HDIVE              hDive              = NULLHANDLE;
BOOL               bDiveHaveBuffer    = FALSE;
ULONG              ulDiveBufferNumber = 0;
HDC                hdcClientHDC = NULLHANDLE;
HPAL               hpalMain;
ULONG              ulcPaletteColors = 34;

/* PM Window variables */
HAB          hab ;
HINI         hini;
HMQ          hmq ;
HWND         hwndFrame, hwndClient ;
QMSG         qmsg;

HPS         hpsMainHPS;
FONTMETRICS fm;
RECTL       udrcl;

MRESULT EXPENTRY ClientWndProc (HWND, ULONG, MPARAM, MPARAM) ;
MRESULT EXPENTRY DlgProc(HWND hwndWnd, ULONG ulMsg, MPARAM mpParm1, MPARAM mpParm2);

// dialog box variables
int db_volume, db_frequency, db_buffer, db_osc; // audio
int db_frt, db_min, db_max, db_init, db_size, db_sizegg; // video
int db_OK; // TRUE if dialog box not cancelled

/** Various variables and short functions ********************/
#define WIDTH 272
#define HEIGHT 208
int scrWIDTH;   // 256 = SMS, 160 = GG
int scrHEIGHT;  // 192 = SMS, 144 = GG
int infoheight;
byte XBuf[WIDTH * HEIGHT];           /* display buffer */
byte Pal[34],XPal[16],SPal[16];
POINTL ptl, aptl[4];
#define Black 32
#define Green 33
int ViewWave;
byte *ViewWaveStart;
extern unsigned char *Buf;

byte Palette[34][3];
byte InitPalette[34][3] =
{
    {0x00,0x00,0x00},{0x00,0x00,0x00},{0x30,0xD0,0x30},{0x70,0xF0,0x70},
    {0x30,0x30,0xF0},{0x50,0x70,0xF0},{0xB0,0x30,0x30},{0x50,0xD0,0xF0},
    {0xF0,0x30,0x30},{0xF0,0x70,0x70},{0xD0,0xD0,0x30},{0xD0,0xD0,0x90},
    {0x30,0x90,0x30},{0xD0,0x50,0xB0},{0xB0,0xB0,0xB0},{0xF0,0xF0,0xF0},
    {0x00,0x00,0x00},{0x00,0x00,0x00},{0x20,0xC0,0x20},{0x60,0xE0,0x60},
    {0x20,0x20,0xE0},{0x40,0x60,0xE0},{0xA0,0x20,0x20},{0x40,0xC0,0xE0},
    {0xE0,0x20,0x20},{0xE0,0x60,0x60},{0xC0,0xC0,0x20},{0xC0,0xC0,0x80},
    {0x20,0x80,0x20},{0xC0,0x40,0xA0},{0xA0,0xA0,0xA0},{0xE0,0xE0,0xE0},
    {0x00,0x00,0x00},{0x00,0xFF,0x00} // constant color Black & Green for oscilliscope
};
ULONG Palette2[34];

int IFreq=50; /* used external */

CHAR szTitle[MAXNAMEL + 1];
CHAR* szClientClass;
int screenx, screeny, xborderwidth, yborderwidth, titleheight, menubarheight;
int width, height;

ERRORID err;
HPS                hps, hpsClient ;

int game_paused;

/* Joystick Values */
long int JS = 0x0000;  /* joystick results */

int frt = 0; /* frame rate table.  0 = 60 fps, 1 = 50 fps */
double framerate, newrate;
int count = 0;
int autofr = TRUE;
int skip = 8;
int initskip = 8;
int maxskip = 15;
int minskip = 0;
int windowsize = 1;
int windowsizegg = 2;
int oscilloscope = 0;
int framecount;
int throttle;
int mg_active;

CHAR carthelppath[CCHMAXPATH] = "";
CHAR carthelpfile[CCHMAXPATH] = "";
CHAR carthelpfullfile[CCHMAXPATH] = "";
CHAR dragfilename[CCHMAXPATH] = "nada";
extern int newgame;
extern int firstgame;
char CartName[CCHMAXPATH] = "";
extern char* MgHelp;
extern char* MgIni;

extern int volume;
extern int buflen;           // 1536
extern ULONG AUDIOfreq;      // = 44100;


/** SetColor *************************************************/
/** Allocate new color (0-15 picture, 16-31 sprites).       **/
/*************************************************************/
void SetColor(byte N,byte R,byte G,byte B)
{
   Palette2[N] = PC_RESERVED * 16777216 + R * 65536 + G * 256 + B;
   DiveSetSourcePalette (hDive, 0, 34L, (PBYTE) Palette2);
}


int different_extension(char* ext, char* file)
{
    char* extpointer = file;
    int i = strlen(file);
    extpointer += i - strlen(ext);
    return strcmp(extpointer, ext);
}


int GetCartridge(char *filename)
{
   static int Init = FALSE;
   static FILEDLG fdFileDlg;
   char pszTitle[40] = "Select Cartridge for Master Gear/2";
   static char drive[10];
   char* separator;
   char* temp;

   if (Init == FALSE)
   {
     memset(&fdFileDlg, 0, sizeof(FILEDLG));
     fdFileDlg.cbSize = sizeof(FILEDLG);
     fdFileDlg.fl = FDS_CENTER | FDS_PRELOAD_VOLINFO | FDS_OPEN_DIALOG;
     fdFileDlg.pszTitle = pszTitle;
     strcpy(fdFileDlg.szFullFile, "*.GG;*.SMS");
     Init = TRUE;
   }
    if (firstgame != 0)
    {
       strcpy(dragfilename, filename);
       firstgame = 0;
    }
   if (strcmp(dragfilename,"nada") == 0)
   {
      WinFileDlg(HWND_DESKTOP, hwndClient, &fdFileDlg);
      if (fdFileDlg.lReturn!= DID_OK)
         return FALSE;
      strcpy (filename, fdFileDlg.szFullFile);
   }
   else
   {
      strcpy (filename, dragfilename);
      strcpy (dragfilename,"nada");
   }

   separator = strrchr(filename, PATH_SEPARATOR);


  if(separator != 0)
  {  strncpy(carthelppath, filename, separator - filename);
     carthelppath[separator - filename] = 0;
     strcat(carthelppath, "\\..\\docs\\");
     temp = (separator == 0) ? filename : separator + 1;
     strcpy (carthelpfile, temp);
     strcpy (CartName, temp);
  }
  else
  {
     carthelppath[0] = 0;
     strcpy(carthelpfile, filename);
     strcpy (CartName, temp);
  }

  separator = strrchr(carthelpfile, '.');
  if(separator != 0)
     strcpy(separator, ".doc");

   strcpy(carthelpfullfile,carthelppath);
   strcat(carthelpfullfile,carthelpfile);

   separator = strrchr(CartName, '.');
   if (separator != 0)
      strcpy(separator, "    ");

   /* new cartridge screen inits */
   framecount = count = 0;

   mg_active = TRUE;

   sprintf(szTitle, "MasterGear/2- %s", CartName); // update the window title
   WinSetWindowText(hwndFrame, szTitle);           // with current game

   return TRUE;
}

void RefreshScreen(void)
{
   static ULONG    ulTime0, ulTime1;     /* output buffer for DosQierySysInfo      */
   static int FramesShown = 0;
   static int J;
   static pow[9] = {256, 224, 192, 160, 128, 96, 64, 32, 0};
   register byte *P,I;
   FillBuffer();  // update sound buffer

   if ((throttle>0) && (count&0x1))
      DosSleep(((1-skiprate[throttle & 0xf][count]) + (throttle >> 4))*5);


   /* if our current skiprate value is a 1, we've got a frame to show */
   if (skiprate[skip][count] == 1)
   {
      /* Mask out left 8 pixels if needed  */
      if(LeftMask)
      {
         P=XBuf+WIDTH*(HEIGHT-192)/2+(WIDTH-256)/2;
         for(J=0;J<192;J++,P+=WIDTH)
            P[0]=P[1]=P[2]=P[3]=P[4]=P[5]=P[6]=P[7]=Black;
      }
      // generate oscilliscope view
      if(ViewWave)
         for (I=0;I<8;I++)
            for(J=0;J<scrWIDTH;J++)
               ViewWaveStart[J + I * WIDTH] = ((Buf[J] <= pow[I]) && (Buf[J] > pow[I+1])) ? Green : Black;

      DiveBlitImage (hDive, ulDiveBufferNumber, DIVE_BUFFER_SCREEN ); /* display MasterGear frame buffer */
      FramesShown++;
      if (framecount-- <= 0)   /* framecounter should drop to zero once per second */
      {
         // calculate FPS
         DosQuerySysInfo ( QSV_MS_COUNT, QSV_MS_COUNT, &ulTime1, 4L );
         newrate = (double) ((FramesShown * 1000) / (ulTime1 - ulTime0));
         ulTime0 = ulTime1;
         framerate = (framerate * 3 / 4) + (newrate / 4); /* age frame rate */
         FramesShown = 0;

         // update status box
         if (throttle > 0)
            sprintf(szTitle, "%2.0ffps %dT%c %3d%c %dx%d",
               framerate, throttle, ((autofr) ? 'A':' '),
               (int) (100 * (framerate + fps[frt][0] - fps[frt][skip]) / fps[frt][0]), '%', width, height);
         else
            sprintf(szTitle, "%2.0ffps %2.0ft%c %2.0fm %3d%c %dx%d",
               framerate, fps[frt][skip], ((autofr) ? 'A':' '), fps[frt][maxskip],
               (int) (100 * (framerate + fps[frt][0] - fps[frt][skip]) / fps[frt][0]), '%', width, height);
         GpiCharStringPosAt(hpsClient, &ptl, &udrcl, CHS_CLIP | CHS_OPAQUE, (LONG) strlen(szTitle), szTitle, NULL);

         // adjust target frame rate
         if (autofr)                          /* if autoframerate routines are active */
         {                                    /*   then adjust frame rate */
            if (throttle > 0)
            {
               if ((framerate - fps[frt][minskip])  > fps[frt][15])
                  throttle += 1;
               else
                  if ((fps[frt][minskip] - framerate) > fps[frt][15])
                     throttle -= 1;
            }
            else
            {
               if ((fps[frt][skip] - framerate) > fps[frt][15]) /* if emulation is to slow */
               {                                        /* fps[frt][15] is difference between adjacent fps[] values */
                  if (skip < maxskip)                   /* and not at user's slowest rate */
                     skip++;                            /* choose a slower frame rate */
               }
               else
               {
                  if ((framerate - fps[frt][skip]) > fps[frt][15])/* if emulation is to fast        */
                     if (skip > minskip)                     /* and not already at highest fps */
                        skip--;                           /* choose a faster frame rate     */
                     else
                        throttle = 1;
               }
            }
         }
         framecount = framerate; /* init framecount for 1 second of frames */
      } /* end of --> if (framecount-- <= 0) */
   } /* end of --> if (skip[skip][count]) */
   count++;
   count &= 0xf;    /* inc counter by 1 & limit values from 0-15 */
}




/** TrashMachine *********************************************/
/** Deallocate all resources taken by InitMachine().        **/
/*************************************************************/
void TrashMachine(void)
{
   JoystickOff();
   JoystickSaveCalibration(); /* save calibration info */
   audio_close();
   DiveUnMap();
   CloseWindow();
}

/** InitMachine **********************************************/
/** Allocate resources needed by Unix/X-dependent code.     **/
/*************************************************************/
int InitMachine(void)
{
   int i, j;
   /*** Palette ***/
   for (i = 0;i < 34;i++)
   {
      for (j = 0;j < 3;j++)
         Palette[i][j] = InitPalette[i][j];
      Palette2[i] = PC_RESERVED * 16777216 + Palette[i][0] * 65536 + Palette[i][1] * 256 + Palette[i][2];
   }
   scrWIDTH  = 256;  // 256 = SMS, 160 = GG
   scrHEIGHT = 192;  // 192 = SMS, 144 = GG

   for (i = 0;i < 16; i++)
   {
      XPal[i] = i;
      SPal[i] = i + 16;
   }
   if (!OpenWindow())   /* open failed */
      return 0;
   DiveMap();
   JoystickInit(0); // Initialize joystick calibration information
   JoystickOn();
   if (volume > 0)
      audio_init(AUDIOfreq, buflen);
   return 1;
}

/** Joysticks ************************************************/
/** Check for keyboard events, parse them, and modify       **/
/** joystick controller status                              **/
/*************************************************************/
long int Joysticks(void)
{
   static JOYSTICK_STATUS stick;
   static long int JSreal;
   /* Clear PAUSE button flag */
   JS &= 0xFFFF;

loop:
   if (WinPeekMsg(hab, &qmsg, NULLHANDLE, 0, 0, PM_NOREMOVE))
      if (WinGetMsg (hab, &qmsg, NULLHANDLE, 0, 0))
         WinDispatchMsg (hab, &qmsg) ;
      else
         CPURunning = FALSE;

   while (game_paused)
   {
      audio_quiet();
      if (WinGetMsg (hab, &qmsg, NULLHANDLE, 0, 0))
         WinDispatchMsg (hab, &qmsg);
      else
      {
         CPURunning = FALSE;
         game_paused = FALSE;
      }
   }

   if (WinPeekMsg(hab, &qmsg, NULLHANDLE, 0, 0, PM_NOREMOVE))
      goto loop; /* don't let messages queue up */

   if (JoystickValues(&stick))
   {
      JSreal = 0;
      /* process joystick 1 */
      if (stick.Joy1X == -1)     JSreal |=0x0004;  /* Player 1 left   */
      else if (stick.Joy1X == 1) JSreal |=0x0008;  /*          right  */
      if (stick.Joy1Y == -1)     JSreal |=0x0001;  /*          up     */
      else if (stick.Joy1Y == 1) JSreal |=0x0002;  /*          down   */
      if (stick.Joy1A)           JSreal |=0x0010;  /*          fire 1 */
      if (stick.Joy1B)           JSreal |=0x0020;  /*          fire 2 */
      /* process joystick 2 */
      if (!GameGear)
      {
         if (stick.Joy2X == -1)     JSreal |=0x0100;  /* Player 2 left   */
         else if (stick.Joy2X == 1) JSreal |=0x0200;  /*          right  */
         if (stick.Joy2Y == -1)     JSreal |=0x0040;  /*          up     */
         else if (stick.Joy2Y == 1) JSreal |=0x0080;  /*          down   */
         if (stick.Joy2A)           JSreal |=0x0400;  /*          fire 1 */
         if (stick.Joy2B)           JSreal |=0x0800;  /*          fire 2 */
      }
   }
   return (JS|JSreal);
}

static void RefreshSprites(byte Y,byte *Z);

/** RefreshLine() ********************************************/
/** Refresh line Y (0..191), including sprites in this line.**/
/*************************************************************/
void RefreshLine(register byte Y)
{
  /* Conversion matrix where result has bits flipped over */
  static byte Conv[256] =
  {
    0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,
    0x30,0xB0,0x70,0xF0,0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,
    0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,0x04,0x84,0x44,0xC4,
    0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
    0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,
    0x3C,0xBC,0x7C,0xFC,0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,
    0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,0x0A,0x8A,0x4A,0xCA,
    0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
    0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,
    0x36,0xB6,0x76,0xF6,0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,
    0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,0x01,0x81,0x41,0xC1,
    0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
    0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,
    0x39,0xB9,0x79,0xF9,0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,
    0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,0x0D,0x8D,0x4D,0xCD,
    0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
    0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,
    0x33,0xB3,0x73,0xF3,0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,
    0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,0x07,0x87,0x47,0xC7,
    0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
    0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,
    0x3F,0xBF,0x7F,0xFF
  };

  register byte I,M,J,K,*P,*T,*R,*C,*Z;
  register unsigned int L;
  byte X1,X2,XR,Offset,Shift;
  byte ZBuf[35];

  /* Calculate the starting address of a picture in XBuf, ZBuf */
  L=WIDTH*(HEIGHT-192)/2+(WIDTH-256)/2+WIDTH*Y;
  P=XBuf+L;Z=ZBuf+1;

  /* If display is turned off, fill scanline with black and exit */
  if(!ScreenON) { memset(P,Black,256);memset(Z,0x00,256/8);return; }

  L=Y+ScrollY;
  if(L>223) L-=223;
  T=ChrTab+((L&0xF8)<<3);
  Offset=(L&0x07)<<2;
  X2=(ScrollX&0xF8)>>2;
  Shift=ScrollX&0x07;
  P-=Shift;Z--;
  Z[0]=0x00;
  XR=NoRghScroll? 24:255;

  for(X1=0;X1<33;X1++)
  {
    /* If right 9 columns not scrolled, modify T and Offset */
    if(X1==XR) { T=ChrTab+((int)(Y&0xF8)<<3);Offset=(Y&0x07)<<2; }

    C=T+X2;                             /* C <== Address of tile #, attrs */
    J=C[1];                             /* J <== Tile attributes          */

    C = VRAM
      + (J&0x01? 0x2000:0x0000)         /* 0x01: Pattern table selection  */
      + ((int)C[0]<<5)                  /* Pattern offset in the table    */
      + (J&0x04? 28-Offset:Offset);     /* 0x04: Vertical flip            */

    R=J&0x08? SPal:XPal;                /* 0x08: Use sprite palette       */
    K=J&0x10;                           /* 0x10: Show in front of sprites */
    J&=0x02;                            /* 0x02: Horizontal flip          */

    I=M=J? Conv[C[0]]:C[0];
    L=((M&0x88)<<21)|((M&0x44)<<14)|((M&0x22)<<7)|(M&0x11);
    I|=M=J? Conv[C[1]]:C[1];
    L|=((M&0x88)<<22)|((M&0x44)<<15)|((M&0x22)<<8)|((M&0x11)<<1);
    I|=M=J? Conv[C[2]]:C[2];
    L|=((M&0x88)<<23)|((M&0x44)<<16)|((M&0x22)<<9)|((M&0x11)<<2);
    I|=M=J? Conv[C[3]]:C[3];
    L|=((M&0x88)<<24)|((M&0x44)<<17)|((M&0x22)<<10)|((M&0x11)<<3);

    P[0]=R[(L>>28)&0x0F];
    P[1]=R[(L>>20)&0x0F];
    P[2]=R[(L>>12)&0x0F];
    P[3]=R[(L>>4)&0x0F];
    P[4]=R[(L>>24)&0x0F];
    P[5]=R[(L>>16)&0x0F];
    P[6]=R[(L>>8)&0x0F];
    P[7]=R[L&0x0F];

    if(K)
    {
      L=(unsigned int)I<<Shift;
      Z[0]|=L>>8;Z[1]=L&0xFF;
    }
    else Z[1]=0x00;

    X2=(X2+2)&0x3F;
    P+=8;Z++;
  }

  /* Refresh sprites in this scanline */
  RefreshSprites(Y,ZBuf+1);
}

/** RefreshSprites() *****************************************/
/** Refresh sprites in a given scanline.                    **/
/*************************************************************/
void RefreshSprites(register byte Line,register byte *ZBuf)
{
  register byte H,J,*P,*C,*Z,Shift;
  register unsigned int L,M;
  int N,Y,X;

  /* If display turned off, we do not show sprites */
  if(!ScreenON) return;
  /* Find the number of the last displayed sprite */
  for(N=0;(N<64)&&(SprY(N)!=208);N++);
  /* Find the sprite height */
  H=Sprites16? 16:8;

  for(N--;N>=0;N--)
  {
    Y=SprY(N)+1;
    if(Y>240) Y-=256;
    if((Line>=Y)&&(Line<Y+H))
    {
      X=SprX(N);
      if(LeftSprites) X-=8;
      J=SprP(N);
      if(H>8) J&=0xFE;
      C=SprGen+((int)J<<5)+((int)(Line-Y)<<2);
      P=XBuf+WIDTH*(HEIGHT-192)/2+(WIDTH-256)/2+WIDTH*Line+X;
      Z=ZBuf+((X&0xFF)>>3);
      Shift=8-(X&0x07);

      J=M=C[0];
      L=((M&0x88)<<21)|((M&0x44)<<14)|((M&0x22)<<7)|(M&0x11);
      J|=M=C[1];
      L|=((M&0x88)<<22)|((M&0x44)<<15)|((M&0x22)<<8)|((M&0x11)<<1);
      J|=M=C[2];
      L|=((M&0x88)<<23)|((M&0x44)<<16)|((M&0x22)<<9)|((M&0x11)<<2);
      J|=M=C[3];
      L|=((M&0x88)<<24)|((M&0x44)<<17)|((M&0x22)<<10)|((M&0x11)<<3);
      J&=~((((int)Z[0]<<8)|Z[1])>>Shift);

      if(J)
      {
        if(J&0x80) P[0]=SPal[(L>>28)&0x0F];
        if(J&0x40) P[1]=SPal[(L>>20)&0x0F];
        if(J&0x20) P[2]=SPal[(L>>12)&0x0F];
        if(J&0x10) P[3]=SPal[(L>>4)&0x0F];
        if(J&0x08) P[4]=SPal[(L>>24)&0x0F];
        if(J&0x04) P[5]=SPal[(L>>16)&0x0F];
        if(J&0x02) P[6]=SPal[(L>>8)&0x0F];
        if(J&0x01) P[7]=SPal[L&0x0F];
      }
    }
  }
}


int DiveMap(void)
{
   ulDiveBufferNumber = 0;
   bDiveHaveBuffer = FALSE;
   /* Associate the canvas with a DIVE handle */
   if (DiveAllocImageBuffer (hDive, &ulDiveBufferNumber, FOURCC_LUT8,
                          WIDTH, HEIGHT, WIDTH,  (PBYTE) XBuf) != NULL)
   {
      /* post error message here.                               */
      /* PostError (hab, hwndFrame, IDS_ERR_CREATE_DIVEBUFFER); */
      return (FALSE);
    }
    bDiveHaveBuffer = TRUE;

    /* Turn on visible region notification. */
    WinSetVisibleRegionNotify (hwndClient, TRUE);

    /* Send an invalidation message to the client. */
    WinPostMsg (hwndFrame, WM_VRNENABLED, 0L, 0L );

    return (TRUE);
}



int DiveUnMap(void)
{
   if (bDiveHaveBuffer)
   {
      DiveFreeImageBuffer (hDive, ulDiveBufferNumber);
      ulDiveBufferNumber = 0;
      bDiveHaveBuffer = FALSE;
   }
   return TRUE;
}

/* set up DIVE - if not possible, display message and quit program */
int InitializeDive(void)
{
   /* Get the screen capabilities, and if the system supports only 16 colors */
   /*  the program should be terminated. */

   DiveCaps.pFormatData    = fccFormats;
   DiveCaps.ulFormatLength = 100;
   DiveCaps.ulStructLen    = sizeof(DIVE_CAPS);

   if ( DiveQueryCaps ( &DiveCaps, DIVE_BUFFER_SCREEN ))
   {
      WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
         (PSZ)"Error: DIVE routines cannot function in this system environment.",
         (PSZ)"   This program is unable to run.", 0, MB_OK | MB_INFORMATION );
      return (FALSE);
   }

   if ( DiveCaps.ulDepth < 8 )
   {
      WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
         (PSZ)"Error: Not enough screen colors to run DIVE.  Must be at least 256 colors.",
         (PSZ)"   This program is unable to run.", 0, MB_OK | MB_INFORMATION );
      return (FALSE);
   }

   /* Get an instance of DIVE APIs. */
   if ( DiveOpen ( &hDive, FALSE, 0 ) )
   {
      WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
         (PSZ)"Error: Unable to open an instance of DIVE.",
         (PSZ)"   This program is unable to run.", 0, MB_OK | MB_INFORMATION );
      return (FALSE);
   }

   return (TRUE);
}

/* create window */
int OpenWindow(void)
{
   int reg;
   ULONG cb;
   /* open terminal session */
   static ULONG flFrameFlags = FCF_TITLEBAR      | FCF_SYSMENU |    FCF_MENU   |
                               FCF_SIZEBORDER    | FCF_MINMAX  |
                               FCF_TASKLIST      | FCF_ICON ;

   szClientClass = strdup("MgWindow");

   skip = 8;                   /* set starting rate at 25 or 30fps */
   framerate = frt ? 25 : 30;  /* set current frame rate at 25 or 30 */
   throttle = 0;               /* set throttle delay to 0 */
   mg_active = FALSE;

   DosSetCurrentDir("ROMS");   /* default into ROMS directory if present */

   /* divisor value for paddle reading based on mouse's current x position on screen */
   screenx      = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
   screeny      = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
   xborderwidth = WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER);
   yborderwidth = WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);
   titleheight  = WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
   menubarheight = 37; /* button graphics */

   hab = WinInitialize(0) ;
   err = WinGetLastError(hab);
   hmq = WinCreateMsgQueue(hab, 0) ;
   err = WinGetLastError(hab);

   if (InitializeDive() == FALSE)     /* bail out if no DIVE support */
      return FALSE;

   // Mg user setting defaults
   frt        = 0; // initialize to current settings
   maxskip    = 15; // min frame rate defined by maximum skip value
   minskip    = 0; // maximum frame rate defined by minimum skip value
   initskip   = 8;
   windowsize = 1;
   windowsizegg = 2;
   oscilloscope = 0;
   volume     = 2;
   buflen     = 768;
   AUDIOfreq  = 22050;

   hini = PrfOpenProfile(hab, MgIni); // load saved settings
   if (hini)
   {
      PrfQueryProfileSize(hini, "mg", "frt", &cb);
      if (cb == sizeof(frt))
         if (!PrfQueryProfileData(hini,"mg","frt", &frt, &cb))
            frt = 0; // default

      PrfQueryProfileSize(hini, "mg", "maxskip", &cb);
      if (cb == sizeof(maxskip))
         if (!PrfQueryProfileData(hini,"mg","maxskip", &maxskip, &cb))
            maxskip = 15; // default

      PrfQueryProfileSize(hini, "mg", "minskip", &cb);
      if (cb == sizeof(minskip))
         if (!PrfQueryProfileData(hini,"mg","minskip", &minskip, &cb))
            minskip = 0; // default

      PrfQueryProfileSize(hini, "mg", "initskip", &cb);
      if (cb == sizeof(initskip))
         if (!PrfQueryProfileData(hini,"mg","initskip", &initskip, &cb))
            initskip = 8; // default

      PrfQueryProfileSize(hini, "mg", "windowsize", &cb);
      if (cb == sizeof(windowsize))
         if (!PrfQueryProfileData(hini,"mg","windowsize", &windowsize, &cb))
            windowsize = 1; // default

      PrfQueryProfileSize(hini, "mg", "windowsizegg", &cb);
      if (cb == sizeof(windowsizegg))
         if (!PrfQueryProfileData(hini,"mg","windowsizegg", &windowsizegg, &cb))
            windowsizegg = 2; // default

      PrfQueryProfileSize(hini, "mg", "oscilloscope", &cb);
      if (cb == sizeof(oscilloscope))
         if (!PrfQueryProfileData(hini,"mg","oscilloscope", &oscilloscope, &cb))
            oscilloscope = 0; // default

      PrfQueryProfileSize(hini, "mg", "volume", &cb);
      if (cb == sizeof(volume))
         if (!PrfQueryProfileData(hini,"mg","volume", &volume, &cb))
            volume = 2; // default

      PrfQueryProfileSize(hini, "mg", "buflen", &cb);
      if (cb == sizeof(buflen))
         if (!PrfQueryProfileData(hini,"mg","buflen", &buflen, &cb))
            buflen = 768; // default

      PrfQueryProfileSize(hini, "mg", "AUDIOfreq", &cb);
      if (cb == sizeof(AUDIOfreq))
         if (!PrfQueryProfileData(hini,"mg","AUDIOfreq", &AUDIOfreq, &cb))
            AUDIOfreq = 22050; // default

      PrfCloseProfile(hini);
   }

   ViewWave = 8 * oscilloscope;

   reg = WinRegisterClass (hab, szClientClass, ClientWndProc, CS_SIZEREDRAW, 0) ;
   err = WinGetLastError(hab);
   hwndFrame = WinCreateStdWindow (HWND_DESKTOP, 0,
                                     &flFrameFlags, szClientClass, NULL,
                                     0L, 0, 444, &hwndClient);
   err = WinGetLastError(hab);

   hpsClient = WinGetPS(hwndClient);
   GpiQueryFontMetrics(hpsClient, (LONG) sizeof fm, &fm);
   infoheight = fm.lMaxBaselineExt + 4;
   width = scrWIDTH * windowsize;
   height = (scrHEIGHT + ViewWave) * windowsize;

   if (hwndFrame)
       WinSetWindowPos(hwndFrame, NULLHANDLE, (screenx - width - 2*xborderwidth)/2,
                                              (screeny - height- 2*yborderwidth - titleheight - menubarheight)/2,
                                              width + 2*xborderwidth,
                                              height + 2*yborderwidth + titleheight + menubarheight + infoheight,
                                              SWP_SIZE|SWP_MOVE|SWP_ACTIVATE|SWP_SHOW);

   while (WinPeekMsg(hab, &qmsg, NULLHANDLE, 0, 0, PM_NOREMOVE))
       if (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
            WinDispatchMsg(hab, &qmsg) ;


   return ((hwndFrame) ? TRUE : FALSE);
}


/* destroy window */
int CloseWindow()
{
   hini = PrfOpenProfile(hab, MgIni); // save settings
   if (hini) // load saved settings
   {
      PrfWriteProfileData(hini,"mg","frt", &frt, sizeof(frt));
      PrfWriteProfileData(hini,"mg","maxskip", &maxskip, sizeof(maxskip));
      PrfWriteProfileData(hini,"mg","minskip", &minskip, sizeof(minskip));
      PrfWriteProfileData(hini,"mg","initskip", &initskip, sizeof(initskip));
      PrfWriteProfileData(hini,"mg","windowsize", &windowsize, sizeof(windowsize));
      PrfWriteProfileData(hini,"mg","windowsizegg", &windowsizegg, sizeof(windowsizegg));
      PrfWriteProfileData(hini,"mg","oscilloscope", &oscilloscope, sizeof(oscilloscope));
      PrfWriteProfileData(hini,"mg","volume", &volume, sizeof(volume));
      PrfWriteProfileData(hini,"mg","buflen", &buflen, sizeof(buflen));
      PrfWriteProfileData(hini,"mg","AUDIOfreq", &AUDIOfreq, sizeof(AUDIOfreq));
      PrfCloseProfile(hini);
   }
   if (hDive)     DiveClose (hDive);
   if (hwndFrame) WinDestroyWindow (hwndFrame) ;
   if (hmq)       WinDestroyMsgQueue (hmq) ;
   if (hab)       WinTerminate (hab) ;

   return TRUE;
}


VOID SizeTheWindow(HWND hwnd, int size)
{
   static int width, height;
   static int lastsize = 1;
   if (size > 0)
   {
      width = scrWIDTH * size;
      height = (scrHEIGHT + ViewWave) * size;
      lastsize = size;
   }
   else
   {
      width = scrWIDTH * lastsize;
      height = (scrHEIGHT + ViewWave) * lastsize;
   }
   WinSetWindowPos(hwndFrame, NULLHANDLE,
                     (screenx - width - 2*xborderwidth)/2,
                     (screeny - height - 2*yborderwidth - titleheight - menubarheight)/2,
                     width + 2*xborderwidth,
                     height + 2*yborderwidth + titleheight + menubarheight + infoheight,
                     SWP_SIZE|SWP_MOVE|SWP_ACTIVATE|SWP_SHOW);
}

void CheckWindowSize(void)
{
   static int first=TRUE;
   int newwidth, newheight;
   if (GameGear)
   {
      newwidth = 160;
      newheight = 144;
   }
   else
   {
      newwidth = 256;
      newheight = 192;
   }
   if ((newwidth != scrWIDTH) || first)
   {
      scrWIDTH  = newwidth;
      scrHEIGHT = newheight;
      SizeTheWindow(hwndFrame, (GameGear) ? windowsizegg : windowsize);
   }
   ViewWaveStart = XBuf + (WIDTH - scrWIDTH) / 2 + WIDTH * ((HEIGHT - scrHEIGHT)/2 + scrHEIGHT);
   first=FALSE;
}



MRESULT EXPENTRY ClientWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   int x, y;
   int i, i2;
   static HBITMAP hbm;
   static RECTL          rcl;
   static RECTL          flip;
   char key;
   char vkey;
   WNDPARAMS *pwprm;
   HWND hwndFrame3;

   /* added w/dive routines */
   SIZEL              sizl;
   POINTL             pointl;         /* Point to offset from Desktop      */
   SWP                swp;            /* Window position                   */
   HRGN               hrgn;           /* Region handle                     */
   RECTL              rcls[50];       /* Rectangle coordinates             */
   RGNRECT            rgnCtl;         /* Processing control structure      */
   SETUP_BLITTER      SetupBlitter;   /* structure for DiveSetupBlitter    */

   PDRAGINFO pDragInfo;
   PDRAGITEM pDragItem;
   char dragname[CCHMAXPATH];

   STARTDATA   sd;
   PID         procID;
   ULONG       sessID;
   APIRET      rc;
   APIRET      arReturn;
   FILESTATUS3 fsStatus;


   switch (msg)
   {
      case WM_COMMAND:
         switch (COMMANDMSG(&msg)->cmd)
         {
            case IDM_NEWCART: CPURunning = 0;
                              newgame = TRUE;
                              break;
            case IDM_SETTINGS:
                              db_frt = frt; // initialize to current settings
                              db_min = maxskip; // min frame rate defined by maximum skip value
                              db_max = minskip; // maximum frame rate defined by minimum skip value
                              db_init = initskip;
                              db_size = windowsize;
                              db_sizegg = windowsizegg;
                              db_osc = oscilloscope;
                              db_volume = volume;
                              db_buffer = buflen;
                              db_frequency = AUDIOfreq;
                              audio_quiet(); // quiet, we're thinking :-)
                              WinDlgBox(HWND_DESKTOP, hwnd, DlgProc, NULLHANDLE, DB_MAIN, NULL);
                              if (db_OK == TRUE)
                              {
                                 frt = db_frt; // set to new settings
                                 maxskip = db_min;
                                 minskip = db_max;
                                 skip = db_init;
                                 if (GameGear)
                                 {
                                    if ((windowsizegg != db_sizegg) || (oscilloscope != db_osc))
                                    {
                                       ViewWave = db_osc * 8;
                                       SizeTheWindow(hwnd, db_sizegg);
                                       windowsizegg = db_sizegg;
                                       oscilloscope = db_osc;
                                    }
                                    if (windowsize != db_size)
                                       windowsize = db_size;
                                 }
                                 else
                                 {
                                    if ((windowsize != db_size) || (oscilloscope != db_osc))
                                    {
                                       ViewWave = db_osc * 8;
                                       SizeTheWindow(hwnd, db_size);
                                       windowsize = db_size;
                                       oscilloscope = db_osc;
                                    }
                                    if (windowsizegg != db_sizegg)
                                       windowsizegg = db_sizegg;
                                 }
                                 if ((db_volume != volume) || (db_frequency != AUDIOfreq) || (db_buffer != buflen))
                                 {
                                    audio_close();
                                    if (db_volume != 0)
                                       audio_init(db_frequency, db_buffer);
                                    volume = db_volume;
                                    AUDIOfreq = db_frequency;
                                    buflen = db_buffer;
                                 }
                              }
                              break;

            case IDM_GAMEHELP:
                              arReturn = DosQueryPathInfo(carthelpfullfile, FIL_STANDARD, &fsStatus, sizeof(fsStatus));
                              audio_quiet(); // quiet, we're thinking :-)
                              if (arReturn != 0)
                                 WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, (PSZ)carthelpfile, (PSZ)"No helpfile found for this game!", 0, MB_OK | MB_INFORMATION );
                              else
                              {
                                 sprintf(dragname, "\"%s\"", carthelpfullfile);
                                 JoystickOff();
                                 procID = 0;
                                 sessID = 0;
                                 memset(&sd, 0, sizeof(STARTDATA));
                                 sd.PgmName = "E.EXE";
                                 sd.PgmInputs = dragname; //carthelpfullfile;
                                 sd.SessionType = SSF_TYPE_PM;
                                 sd.Length = sizeof(STARTDATA);
                                 sd.Related = SSF_RELATED_INDEPENDENT;
                                 sd.FgBg = SSF_FGBG_FORE;
                                 sd.TraceOpt = SSF_TRACEOPT_NONE;
                                 sd.TermQ = 0;
                                 sd.Environment = NULL;
                                 sd.InheritOpt = SSF_INHERTOPT_PARENT;
                                 sd.PgmControl = SSF_CONTROL_VISIBLE | SSF_CONTROL_MAXIMIZE;
                                 sd.InitXPos = 10;
                                 sd.InitYPos = 10;
                                 sd.InitXSize = 400;
                                 sd.InitYSize = 600;
                                 sd.Reserved = 0;
                                 sd.ObjectBuffer = NULL;
                                 sd.ObjectBuffLen = 0;
                                 rc = DosStartSession(&sd, &sessID, &procID);
                                 DosSleep(5000);
                                 JoystickOn();
                              }
                              break;
            case IDM_MGHELP:  arReturn = DosQueryPathInfo(MgHelp, FIL_STANDARD, &fsStatus, sizeof(fsStatus));
                              audio_quiet(); // quiet, we're thinking :-)
                              if (arReturn != 0)
                                 WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, MgHelp, (PSZ)"File is missing!", 0, MB_OK | MB_INFORMATION );
                              else
                              {
                                 sprintf(dragname, "\"%s\"", MgHelp);
                                 JoystickOff();
                                 procID = 0;
                                 sessID = 0;
                                 memset(&sd, 0, sizeof(STARTDATA));
                                 sd.PgmName = "VIEW.EXE";
                                 sd.PgmInputs = dragname; // MgHelp;
                                 sd.SessionType = SSF_TYPE_PM;
                                 sd.Length = sizeof(STARTDATA);
                                 sd.Related = SSF_RELATED_INDEPENDENT;
                                 sd.FgBg = SSF_FGBG_FORE;
                                 sd.TraceOpt = SSF_TRACEOPT_NONE;
                                 sd.TermQ = 0;
                                 sd.Environment = NULL;
                                 sd.InheritOpt = SSF_INHERTOPT_PARENT;
                                 sd.PgmControl = SSF_CONTROL_VISIBLE | SSF_CONTROL_MAXIMIZE;
                                 sd.InitXPos = 10;
                                 sd.InitYPos = 10;
                                 sd.InitXSize = 400;
                                 sd.InitYSize = 600;
                                 sd.Reserved = 0;
                                 sd.ObjectBuffer = NULL;
                                 sd.ObjectBuffLen = 0;
                                 rc = DosStartSession(&sd, &sessID, &procID);
                                 DosSleep(5000);
                                 JoystickOn();
                              }
                              break;
         }

      /* here we are creating the new window and setting up some DIVE information */
      case WM_CREATE:
         if (mg_active == FALSE)
         {
            hps = WinGetPS (hwnd) ;
            hbm = GpiLoadBitmap (hps, 0, START_BMP, 0L, 0L);
            WinReleasePS (hps) ;
         }

         sizl.cx = sizl.cy = 0;
         hdcClientHDC = WinOpenWindowDC (hwnd);
         hpsMainHPS = GpiCreatePS (hab, hdcClientHDC, &sizl,
                      PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);

         hpalMain = GpiCreatePalette (hab,
                                 LCOL_PURECOLOR,
                                 LCOLF_CONSECRGB,
                                 ulcPaletteColors,
                                (PULONG) Palette2);

         GpiSelectPalette (hpsMainHPS, hpalMain);
         WinRealizePalette (hwnd, hpsMainHPS, &ulcPaletteColors);
         DiveSetSourcePalette (hDive, 0, 34L, (PBYTE) Palette2);
         DiveSetDestinationPalette (hDive, 0, 34, 0 );

         WinQueryWindowRect (hwnd, &rcl);
         aptl[0].x = flip.xLeft;
         aptl[0].y = flip.yTop;
         aptl[1].x = flip.xRight;
         aptl[1].y = flip.yBottom;
         aptl[2].x = 0;
         aptl[2].y = 0;
         aptl[3].x = scrWIDTH;
         aptl[3].y = scrHEIGHT + ViewWave;

         WinQueryWindowRect (hwnd, &rcl);

         width = rcl.xRight;
         height = rcl.yTop - infoheight;
         aptl[0].x = 0;aptl[0].y = 0;
         aptl[1].x = width-1;aptl[1].y = 0;
         aptl[2].x = width-1;aptl[2].y = infoheight - 2;
         GpiSetColor(hpsClient, CLR_WHITE);
         GpiMove(hpsClient, &aptl[0]);
         GpiPolyLine(hpsClient, 2, &aptl[1]);

         aptl[1].x = 0;aptl[1].y = infoheight - 2;
         aptl[2].x = 0;aptl[2].y = 0;
         GpiSetColor(hpsClient, CLR_DARKGRAY);
         GpiPolyLine(hpsClient, 2, &aptl[1]);

         aptl[0].x = 0;         aptl[0].y = infoheight - 1;
         aptl[1].x = width - 1; aptl[1].y = infoheight - 1;
         GpiSetColor(hpsClient, CLR_PALEGRAY);
         GpiMove(hpsClient, &aptl[0]);
         GpiLine(hpsClient, &aptl[1]);

         GpiSetBackColor(hpsClient, CLR_PALEGRAY);
         GpiSetColor(hpsClient, CLR_PALEGRAY);

         ptl.x = 2; ptl.y = fm.lMaxDescender + 1;
         udrcl.xLeft = udrcl.yBottom = 1;
         udrcl.xRight = width-2; udrcl.yTop = infoheight-3;

         aptl[0].x = 1; aptl[0].y = 1;
         aptl[1].x = width - 2; aptl[1].y = infoheight - 3;
         GpiMove(hpsClient, &aptl[0]);
         GpiBox(hpsClient, DRO_FILL, &aptl[1], 0L, 0L);
         GpiSetColor(hpsClient, CLR_DARKBLUE);
         return 0 ;


      case WM_PAINT:
         if (mg_active)
         {
            hps = WinBeginPaint (hwnd, NULLHANDLE, NULL);
            DiveBlitImage (hDive, ulDiveBufferNumber, DIVE_BUFFER_SCREEN );
            WinEndPaint (hps);
         }
         else
            if (hwnd == hwndClient)
            {
               hps = WinBeginPaint (hwnd, NULLHANDLE, NULL) ;
               GpiErase (hps) ;
               WinQueryWindowRect (hwnd, &rcl) ;
               rcl.yBottom = infoheight;
               WinDrawBitmap (hps, hbm, NULL, (PPOINTL) &rcl,
                              CLR_BACKGROUND, CLR_NEUTRAL, DBM_STRETCH) ;
               DosSleep(500);
               WinEndPaint (hps) ;
            }
            else
            {
               hps = WinBeginPaint (hwnd, NULLHANDLE, NULL) ;
               GpiErase (hps) ;
               WinEndPaint (hps) ;
            }
         WinQueryWindowRect (hwnd, &rcl);

         width = rcl.xRight;
         height = rcl.yTop - infoheight;
         aptl[0].x = 0;aptl[0].y = 0;
         aptl[1].x = width-1;aptl[1].y = 0;
         aptl[2].x = width-1;aptl[2].y = infoheight - 2;
         GpiSetColor(hpsClient, CLR_WHITE);
         GpiMove(hpsClient, &aptl[0]);
         GpiPolyLine(hpsClient, 2, &aptl[1]);
         aptl[1].x = 0;aptl[1].y = infoheight - 2;
         aptl[2].x = 0;aptl[2].y = 0;
         GpiSetColor(hpsClient, CLR_DARKGRAY);
         GpiPolyLine(hpsClient, 2, &aptl[1]);
         aptl[0].x = 0;         aptl[0].y = infoheight - 1;
         aptl[1].x = width - 1; aptl[1].y = infoheight - 1;
         GpiSetColor(hpsClient, CLR_PALEGRAY);
         GpiMove(hpsClient, &aptl[0]);
         GpiLine(hpsClient, &aptl[1]);
         GpiSetBackColor(hpsClient, CLR_PALEGRAY);
         GpiSetColor(hpsClient, CLR_PALEGRAY);
         ptl.x = 2; ptl.y = fm.lMaxDescender + 1;
         udrcl.xLeft = udrcl.yBottom = 1;
         udrcl.xRight = width-2; udrcl.yTop = infoheight-3;
         aptl[0].x = 1; aptl[0].y = 1;
         aptl[1].x = width - 2; aptl[1].y = infoheight - 3;
         GpiMove(hpsClient, &aptl[0]);
         GpiBox(hpsClient, DRO_FILL, &aptl[1], 0L, 0L);
         GpiSetColor(hpsClient, CLR_DARKBLUE);
         return 0 ;

      case WM_CLOSE:
         CPURunning = 0;
         return 0;

      case WM_SIZE:
         WinQueryWindowRect (hwnd, &rcl);
         width = rcl.xRight;
         height = rcl.yTop - infoheight;
         aptl[0].x = 0;aptl[0].y = 0;
         aptl[1].x = width-1;aptl[1].y = 0;
         aptl[2].x = width-1;aptl[2].y = infoheight - 2;
         GpiSetColor(hpsClient, CLR_WHITE);
         GpiMove(hpsClient, &aptl[0]);
         GpiPolyLine(hpsClient, 2, &aptl[1]);
         aptl[1].x = 0;aptl[1].y = infoheight - 2;
         aptl[2].x = 0;aptl[2].y = 0;
         GpiSetColor(hpsClient, CLR_DARKGRAY);
         GpiPolyLine(hpsClient, 2, &aptl[1]);
         aptl[0].x = 0;         aptl[0].y = infoheight - 1;
         aptl[1].x = width - 1; aptl[1].y = infoheight - 1;
         GpiSetColor(hpsClient, CLR_PALEGRAY);
         GpiMove(hpsClient, &aptl[0]);
         GpiLine(hpsClient, &aptl[1]);
         GpiSetBackColor(hpsClient, CLR_PALEGRAY);
         GpiSetColor(hpsClient, CLR_PALEGRAY);
         ptl.x = 2; ptl.y = fm.lMaxDescender + 1;
         udrcl.xLeft = udrcl.yBottom = 1;
         udrcl.xRight = width-2; udrcl.yTop = infoheight-3;
         GpiSetColor(hpsClient, CLR_DARKBLUE);
         return 0;

      case WM_CHAR:
         if (CHARMSG(&msg)->fs & KC_PREVDOWN)
            return 0;
         key  = SHORT1FROMMP(mp2);
         vkey = SHORT2FROMMP(mp2);
         if (CHARMSG(&msg)->fs & KC_KEYUP)
         {                                  /* key released */
            if (key || vkey)
            {
               if (vkey)
                  switch (vkey)
                  {
                     case VK_DOWN:  JS &= 0xFFFD;  break; /* player 1 move down  */
                     case VK_UP:    JS &= 0xFFFE;  break; /*               up    */
                     case VK_LEFT:  JS &= 0xFFFB;  break; /*               left  */
                     case VK_RIGHT: JS &= 0xFFF7;  break; /*               right */
                     case VK_ESC:   CPURunning = 0;   break; /* Quit */
                     case VK_TAB:   JS &= 0xEFFF;  break; /* reset Master system */
                     case VK_NEWLINE:
                     case VK_ENTER: JS &= 0xDFFF; break; /* start Game Gear */
                  }
               else
                  if (CHARMSG(&msg)->fs & KC_CTRL)
                     switch (key)
                     {
                        case '1': case '2': case '3':
                        case '4': case '5': case '6':
                        case '7': case '8': case '9':
                           SizeTheWindow(hwnd, key - '0');    break;
                     }
                  else
                     switch(key)
                     {
                        case '+': case '=': if (maxskip > 0)  /* select faster framerate */
                                                      maxskip--;
                                                   if ((maxskip < skip) || (autofr == FALSE))
                                                      skip = maxskip;
                                                   /* force update of window title display */
                                                   // titlecount = titletoggle = 0;
                                                   framecount = 0;
                                                   break;
                        case '-': case '_': if (maxskip < 15)  /* select slower framerate */
                                                      maxskip++;
                                                   if (autofr == FALSE)
                                                      skip = maxskip;
                                                   /* force update of window title display */
                                                   // titlecount = titletoggle = 0;
                                                   framecount = 0;
                                                   break;
                        case 'a': case 'A': autofr = (autofr) ? FALSE : TRUE;
                                                   /* force update of window title display */
                                                   // titlecount = titletoggle = 0;
                                                   framecount = 0;
                                                   break;
                        case 'p': case 'P': game_paused = (game_paused != 0) ? 0 : 1; break;
                        case 'g': case 'G': JS &= 0xFF7F; break; /* player 2 move down   */
                        case 't': case 'T': JS &= 0xFFBF; break; /*               up     */
                        case 'f': case 'F': JS &= 0xFEFF; break; /*               left   */
                        case 'h': case 'H': JS &= 0xFDFF; break; /*               right  */
                        case 'z': case 'Z': JS &= 0xFBFF; break; /*               fire 1 */
                        case 'x': case 'X': JS &= 0xF7FF; break; /*               fire 2 */
                        case 'n': case 'N': CPURunning = 0;
                                            newgame = TRUE;
                                            break;
                        case '#': // For German users
                        case '>': case '.': JS &= 0xFFEF; break; /* player 1 fire 1 */
                        case 'ä': // For German users umlauted 'a'
                        case '/': case '?': JS &= 0xFFDF; break; /*          fire 2 */
                        case 's': case 'S': JS &= 0xD000; break; /* start game */
                        case 'o': case 'O': ViewWave = 8 - ViewWave;
                                            SizeTheWindow(hwnd, 0);
                                            break;
                     }
            }
         }
         else
         {                                  /* key pressed */
            if (key || vkey)
               if (vkey)
                  switch (vkey)
                  {
                     case VK_DOWN:  JS |= 0x0002;  break; /* player 1 move down  */
                     case VK_UP:    JS |= 0x0001;  break; /*               up    */
                     case VK_LEFT:  JS |= 0x0004;  break; /*               left  */
                     case VK_RIGHT: JS |= 0x0008;  break; /*               right */
                     case VK_NEWLINE:
                     case VK_ENTER: JS |= 0x2000;  break; /* GG Start */
                     case VK_TAB:   JS |= 0x1000;  break; /* MS reset */
                     case VK_BACKSPACE:
                                    JS |= 0x10000; break; /* MS pause */
                  }
               else
                  if (CHARMSG(&msg)->fs & KC_CTRL)
                     switch (key)
                     {
                        case '1': case '2': case '3':
                        case '4': case '5': case '6':
                        case '7': case '8': case '9':
                           SizeTheWindow(hwnd, key - '0');    break;
                     }
                  else
                     switch(key)
                     {
                        case 'g': case 'G': JS |= 0x0080; break; /* player 2 move down   */
                        case 't': case 'T': JS |= 0x0040; break; /*               up     */
                        case 'f': case 'F': JS |= 0x0100; break; /*               left   */
                        case 'h': case 'H': JS |= 0x0200; break; /*               right  */
                        case 'z': case 'Z': JS |= 0x0400; break; /*               fire 1 */
                        case 'x': case 'X': JS |= 0x0800; break; /*               fire 2 */
                        case '#': // For German users
                        case '>': case '.': JS |= 0x0010; break; /* player 1 fire 1 */
                        case 'ä': // For German users umlauted 'a'
                        case '/': case '?': JS |= 0x0020; break; /*          fire 2 */
                        case 's': case 'S': JS |= 0x2000; break; /* start game */
                     }
         }
         return 0;


      case DM_DRAGOVER:
         pDragInfo = (PDRAGINFO) mp1;
         DrgAccessDraginfo(pDragInfo);
         pDragItem = DrgQueryDragitemPtr(pDragInfo, 0);
         DrgQueryStrName(pDragItem->hstrSourceName, CCHMAXPATH, dragname);
         /* exclude multi select & not snapshot/tap */
         if((pDragInfo->cditem > 1) ||
            (different_extension(".sms", dragname) && different_extension(".SMS", dragname)) &&
            (different_extension(".gg", dragname) && different_extension(".GG", dragname)))
               return (MPFROM2SHORT (DOR_NEVERDROP, DO_UNKNOWN));
         else
               return (MPFROM2SHORT (DOR_DROP, DO_UNKNOWN));
         break;

      case DM_DROP:
         pDragInfo = (PDRAGINFO)mp1;
         DrgAccessDraginfo(pDragInfo);
         pDragItem = DrgQueryDragitemPtr(pDragInfo, 0);
         DrgQueryStrName(pDragItem->hstrContainerName, CCHMAXPATH, dragname);
         DrgQueryStrName(pDragItem->hstrSourceName, CCHMAXPATH, dragname+strlen(dragname));

         strcpy(dragfilename, dragname);
         newgame = TRUE;
         CPURunning = 0;
         WinSetFocus(HWND_DESKTOP, hwnd);
         break;

      case WM_SETWINDOWPARAMS:
         pwprm = (PWNDPARAMS) PVOIDFROMMP (mp1);
         if (pwprm->fsStatus & WPM_TEXT)
            strcpy(pwprm->pszText, szTitle);
         return MRFROMSHORT(1);

      case WM_ACTIVATE:
         game_paused = (mp1) ? (game_paused & 1) : (game_paused | 2);
         return 0;

      case WM_DESTROY:
         CPURunning = 0;
         if (hpsMainHPS  != NULLHANDLE)
            GpiSelectPalette (hpsMainHPS, NULLHANDLE);
         if (hpalMain != NULLHANDLE)
            GpiDeletePalette (hpalMain);
         if (hpsMainHPS != NULLHANDLE)
            GpiDestroyPS (hpsMainHPS);
         return 0 ;

      case WM_REALIZEPALETTE:
         /* This tells DIVE that the physical palette may have changed. */
         DiveSetDestinationPalette (hDive, 0, 34, 0 );
         break;

      case WM_VRNDISABLED:
         DiveSetupBlitter (hDive, 0);
         break;

      case WM_VRNENABLED:
         if ( !hpsMainHPS )
            break;
         hrgn = GpiCreateRegion ( hpsMainHPS, 0L, NULL );
         if ( hrgn )
         {
            WinQueryVisibleRegion ( hwnd, hrgn );
            rgnCtl.ircStart     = 0;
            rgnCtl.crc          = 50;
            rgnCtl.ulDirection  = 1;

            /* Get the all ORed rectangles */
            if ( GpiQueryRegionRects ( hpsMainHPS, hrgn, NULL, &rgnCtl, rcls) )
            {
               /* Now find the window position and size, relative to parent. */
               WinQueryWindowPos (hwndClient, &swp );

               /* Convert the point to offset from desktop lower left. */
               pointl.x = swp.x;
               pointl.y = swp.y;
               WinMapWindowPoints ( hwndFrame, HWND_DESKTOP, &pointl, 1 );

               /* Tell DIVE about the new settings. */
               SetupBlitter.ulStructLen = sizeof ( SETUP_BLITTER );
               SetupBlitter.fccSrcColorFormat = FOURCC_LUT8;
               SetupBlitter.ulSrcWidth   = scrWIDTH;
               SetupBlitter.ulSrcHeight  = scrHEIGHT + ViewWave;
               SetupBlitter.ulSrcPosX    = (WIDTH-scrWIDTH)/2;
               SetupBlitter.ulSrcPosY    = (HEIGHT-scrHEIGHT)/2;
               SetupBlitter.fInvert      = FALSE;
               SetupBlitter.ulDitherType = 1;

               SetupBlitter.fccDstColorFormat = FOURCC_SCRN;
               SetupBlitter.ulDstWidth        = swp.cx;
               SetupBlitter.ulDstHeight       = swp.cy - infoheight;
               SetupBlitter.lDstPosX          = 0;
               SetupBlitter.lDstPosY          = infoheight;
               SetupBlitter.lScreenPosX       = pointl.x;
               SetupBlitter.lScreenPosY       = pointl.y;
               SetupBlitter.ulNumDstRects     = rgnCtl.crcReturned;
               SetupBlitter.pVisDstRects      = rcls;
               DiveSetupBlitter (hDive, &SetupBlitter);
            }
            else
               DiveSetupBlitter (hDive, 0);

            GpiDestroyRegion (hpsMainHPS, hrgn);
         }
         break;
   }
   return WinDefWindowProc (hwnd, msg, mp1, mp2) ;
}



MRESULT EXPENTRY DlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   char dragname[CCHMAXPATH];
   STARTDATA   sd;
   PID         procID;
   ULONG       sessID;
   APIRET      rc;
   APIRET      arReturn;
   FILESTATUS3 fsStatus;

   switch(msg)
   {
      case WM_INITDLG:   // initialize all the sliders and numeric displays
         if (db_frt == 0)
            WinCheckButton(hwnd, DB_VR_NTSC, 1);
         else
            WinCheckButton(hwnd, DB_VR_PAL, 1);

         WinCheckButton(hwnd, DB_AF_8 + (int) (db_frequency / 11025), 1);
         WinCheckButton(hwnd, DB_AV_OFF + (int) db_volume, 1);

         WinSetDlgItemShort(hwnd, DB_BS_BT, db_buffer, TRUE);                    // init buffer size text
         WinSendDlgItemMsg(hwnd, DB_BS_B, SLM_SETSLIDERINFO,                     // init buffer size slider position
              MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE), MPFROMLONG((db_buffer - 256) / 32) );

         WinSendDlgItemMsg(hwnd, DB_OSC, BM_SETCHECK, MPFROMSHORT(db_osc), NULL);

         WinSetDlgItemShort(hwnd, DB_FPS_MIN_T, fps[db_frt][db_min] + .5, TRUE); // init min text
         WinSendDlgItemMsg(hwnd, DB_FPS_MIN, SLM_SETSLIDERINFO,                  // init min slider position
              MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE), MPFROMLONG((15 - db_min) * 5) );

         WinSetDlgItemShort(hwnd, DB_FPS_MAX_T, fps[db_frt][db_max] + .5, TRUE); // init min text
         WinSendDlgItemMsg(hwnd, DB_FPS_MAX, SLM_SETSLIDERINFO,                  // init min slider position
              MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE), MPFROMLONG((15 - db_max) * 5) );

         WinSetDlgItemShort(hwnd, DB_FPS_INIT_T, fps[db_frt][db_init] + .5, TRUE);       // init min text
         WinSendDlgItemMsg(hwnd, DB_FPS_INIT, SLM_SETSLIDERINFO,                   // init min slider position
              MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE), MPFROMLONG((15 - db_init) * 5) );


         WinSendDlgItemMsg(hwnd, DB_IWS_GG, SPBM_SETLIMITS, MPFROMLONG(9), MPFROMLONG(1));
         WinSendDlgItemMsg(hwnd, DB_IWS_GG, SPBM_SETCURRENTVALUE, MPFROMLONG(db_sizegg), NULL);

         WinSendDlgItemMsg(hwnd, DB_IWS, SPBM_SETLIMITS, MPFROMLONG(9), MPFROMLONG(1));
         WinSendDlgItemMsg(hwnd, DB_IWS, SPBM_SETCURRENTVALUE, MPFROMLONG(db_size), NULL);

         return 0;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case DB_J_REC: JoystickInit(1);
               WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                  "The Joystick calibration information has been reset.  Once back in Master Gear you will need to move your joystick to all four corners to recalibrate!",
                  "Joysticks Reset" , 0, MB_OK | MB_INFORMATION );
               return 0;

            case DB_AF_DFLT: db_volume = 2;
               db_frequency = 22050;
               db_buffer = 768;
               db_osc = 0;
               WinCheckButton(hwnd, DB_AF_8 + (int) (db_frequency / 11025), 1);
               WinCheckButton(hwnd, DB_AV_OFF + (int) db_volume, 1);
               WinSetDlgItemShort(hwnd, DB_BS_BT, db_buffer, TRUE);                    // init buffer size text
               WinSendDlgItemMsg(hwnd, DB_BS_B, SLM_SETSLIDERINFO,                     // init buffer size slider position
               MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE), MPFROMLONG((db_buffer - 256) / 32) );
               WinSendDlgItemMsg(hwnd, DB_OSC, BM_SETCHECK, MPFROMSHORT(db_osc), NULL);
               return 0;

            case DB_HELP: arReturn = DosQueryPathInfo(MgHelp, FIL_STANDARD, &fsStatus, sizeof(fsStatus));
               if (arReturn != 0)
                  WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, MgHelp, (PSZ)"File is missing!", 0, MB_OK | MB_INFORMATION );
               else
               {
                  sprintf(dragname, "\"%s\" \"settings dialog\"", MgHelp);
                  JoystickOff();
                  procID = 0;
                  sessID = 0;
                  memset(&sd, 0, sizeof(STARTDATA));
                  sd.PgmName = "VIEW.EXE";
                  sd.PgmInputs = dragname; // Master GearHelp;
                  sd.SessionType = SSF_TYPE_PM;
                  sd.Length = sizeof(STARTDATA);
                  sd.Related = SSF_RELATED_INDEPENDENT;
                  sd.FgBg = SSF_FGBG_FORE;
                  sd.TraceOpt = SSF_TRACEOPT_NONE;
                  sd.TermQ = 0;
                  sd.Environment = NULL;
                  sd.InheritOpt = SSF_INHERTOPT_PARENT;
                  sd.PgmControl = SSF_CONTROL_VISIBLE | SSF_CONTROL_MAXIMIZE;
                  sd.InitXPos = 10;
                  sd.InitYPos = 10;
                  sd.InitXSize = 400;
                  sd.InitYSize = 600;
                  sd.Reserved = 0;
                  sd.ObjectBuffer = NULL;
                  sd.ObjectBuffLen = 0;
                  rc = DosStartSession(&sd, &sessID, &procID);
                  DosSleep(5000);
                  JoystickOn();
               }
               return 0;

            case DB_CAN:
               db_OK = FALSE;
               WinDismissDlg(hwnd, TRUE);
               return 0;

            case DB_OK:
               db_OK = TRUE;
               db_osc = (int) WinSendDlgItemMsg(hwnd, DB_OSC, BM_QUERYCHECK, NULL, NULL);
               WinSendDlgItemMsg(hwnd, DB_IWS, SPBM_QUERYVALUE, (PVOID) &db_size,  MPFROM2SHORT( 0, SPBQ_UPDATEIFVALID));
               WinSendDlgItemMsg(hwnd, DB_IWS_GG, SPBM_QUERYVALUE, (PVOID) &db_sizegg,  MPFROM2SHORT( 0, SPBQ_UPDATEIFVALID));
               WinDismissDlg(hwnd, TRUE);
               return 0;
         }

      case WM_CONTROL:
         switch(SHORT1FROMMP(mp1))
         {
            case DB_VR_NTSC:
               db_frt = 0;
               WinSetDlgItemShort(hwnd, DB_FPS_MIN_T, fps[db_frt][db_min] + .5, TRUE);
               WinSetDlgItemShort(hwnd, DB_FPS_MAX_T, fps[db_frt][db_max] + .5, TRUE);
               WinSetDlgItemShort(hwnd, DB_FPS_INIT_T, fps[db_frt][db_init] + .5, TRUE);
               return 0;

            case DB_VR_PAL:
               db_frt = 1;
               WinSetDlgItemShort(hwnd, DB_FPS_MIN_T, fps[db_frt][db_min] + .5, TRUE);
               WinSetDlgItemShort(hwnd, DB_FPS_MAX_T, fps[db_frt][db_max] + .5, TRUE);
               WinSetDlgItemShort(hwnd, DB_FPS_INIT_T, fps[db_frt][db_init] + .5, TRUE);
               return 0;

            case DB_AV_OFF:
               db_volume = 0;
               return 0;

            case DB_AV_LO:
               db_volume = 1;
               return 0;

            case DB_AV_HI:
               db_volume = 2;
               return 0;

            case DB_AF_8:
               db_frequency =  8000;
               return 0;

            case DB_AF_11:
               db_frequency =  11025;
               return 0;

            case DB_AF_22:
               db_frequency =  22050;
               return 0;

            case DB_AF_33:
               db_frequency =  33075;
               return 0;

            case DB_AF_44:
               db_frequency =  44100;
               return 0;

            case DB_BS_B:
               if ((SHORT2FROMMP(mp1) == SLN_CHANGE) || (SHORT2FROMMP(mp1) == SLN_SLIDERTRACK))
               {
                  db_buffer = (int) mp2 * 32 + 256;
                  WinSetDlgItemShort(hwnd, DB_BS_BT, db_buffer, TRUE);
               }
               return 0;

            case DB_FPS_MIN:
               if ((SHORT2FROMMP(mp1) == SLN_CHANGE) || (SHORT2FROMMP(mp1) == SLN_SLIDERTRACK))
               {
                  db_min = 15 - (int) mp2 / 5;
                  WinSetDlgItemShort(hwnd, DB_FPS_MIN_T, fps[db_frt][db_min] + .5, TRUE);
                  if (db_min < db_max)
                  {
                     db_max = db_min;
                     WinSetDlgItemShort(hwnd, DB_FPS_MAX_T, fps[db_frt][db_max] + .5, TRUE);       // init min text
                     WinSendDlgItemMsg(hwnd, DB_FPS_MAX, SLM_SETSLIDERINFO,                   // init min slider position
                        MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE), MPFROMLONG((15 - db_max) * 5) );
                  }
                  if (db_min < db_init)
                  {
                     db_init = db_min;
                     WinSetDlgItemShort(hwnd, DB_FPS_INIT_T, fps[db_frt][db_init] + .5, TRUE);       // init min text
                     WinSendDlgItemMsg(hwnd, DB_FPS_INIT, SLM_SETSLIDERINFO,                   // init min slider position
                        MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE), MPFROMLONG((15 - db_init) * 5) );
                  }
               }
               return 0;

            case DB_FPS_MAX:
               if ((SHORT2FROMMP(mp1) == SLN_CHANGE) || (SHORT2FROMMP(mp1) == SLN_SLIDERTRACK))
               {
                  db_max = 15 - (int) mp2 / 5;
                  WinSetDlgItemShort(hwnd, DB_FPS_MAX_T, fps[db_frt][db_max] + .5, TRUE);
                  if (db_max > db_min)
                  {
                     db_min = db_max;
                     WinSetDlgItemShort(hwnd, DB_FPS_MIN_T, fps[db_frt][db_min] + .5, TRUE);       // init min text
                     WinSendDlgItemMsg(hwnd, DB_FPS_MIN, SLM_SETSLIDERINFO,                   // init min slider position
                        MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE), MPFROMLONG((15 - db_min) * 5) );
                  }
                  if (db_max > db_init)
                  {
                     db_init = db_max;
                     WinSetDlgItemShort(hwnd, DB_FPS_INIT_T, fps[db_frt][db_init] + .5, TRUE);       // init min text
                     WinSendDlgItemMsg(hwnd, DB_FPS_INIT, SLM_SETSLIDERINFO,                   // init min slider position
                        MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE), MPFROMLONG((15 - db_init) * 5) );
                  }
               }
               return 0;

            case DB_FPS_INIT:
               if ((SHORT2FROMMP(mp1) == SLN_CHANGE) || (SHORT2FROMMP(mp1) == SLN_SLIDERTRACK))
               {
                  db_init = 15 - (int) mp2 / 5;
                  WinSetDlgItemShort(hwnd, DB_FPS_INIT_T, fps[db_frt][db_init] + .5, TRUE);
                  if (db_max > db_init)
                  {
                     db_max = db_init;
                     WinSetDlgItemShort(hwnd, DB_FPS_MAX_T, fps[db_frt][db_max] + .5, TRUE);       // init min text
                     WinSendDlgItemMsg(hwnd, DB_FPS_MAX, SLM_SETSLIDERINFO,                   // init min slider position
                        MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE), MPFROMLONG((15 - db_max) * 5) );
                  }
                  if (db_min < db_init)
                  {
                     db_min = db_init;
                     WinSetDlgItemShort(hwnd, DB_FPS_MIN_T, fps[db_frt][db_min] + .5, TRUE);       // init min text
                     WinSendDlgItemMsg(hwnd, DB_FPS_MIN, SLM_SETSLIDERINFO,                   // init min slider position
                        MPFROM2SHORT( SMA_SLIDERARMPOSITION, SMA_RANGEVALUE), MPFROMLONG((15 - db_min) * 5) );
                  }
               }
               return 0;
         }
      return 0;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
