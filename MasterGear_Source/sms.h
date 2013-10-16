/** MasterGear: portable MasterSystem/GameGear emulator ******/
/**                                                         **/
/**                           SMS.h                         **/
/**                                                         **/
/** This file contains declarations relevant to the drivers **/
/** and SMS/GG emulation itself. See Z80.h for #defines     **/
/** related to Z80 emulation.                               **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994,1995,1996            **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef SMS_H
#define SMS_H

#include "Z80.h"            /* Z80 emulation declarations    */
#define PATH_SEPARATOR  '\\'

#define NORAM     0xFF      /* Byte to be returned from      */
                            /* non-existing pages and ports  */
#define SOUNDBASE 131072L   /* Base sound frequency          */

/** Variables used to control emulator behavior **************/
extern byte Verbose;        /* Debugging messages ON/OFF     */
extern byte GameGear;       /* 1: GameGear   0: MasterSystem */
extern byte Country;        /* 0:Engl/Auto,1:Engl,2:Japanese */
extern byte DelayedRD;      /* 1: VRAM reads delayed by one  */
extern byte AutoA,AutoB;    /* Autofire ON/OFF               */
extern int  VPeriod;        /* CPU opcodes / VBlank          */
extern byte UPeriod;        /* Interrupts / Screen update    */
extern char *SndName;       /* Soundtrack log file name      */
/*************************************************************/

/** Following macros can be used in screen drivers ***********/
#define BigSprites  (VDP[1]&0x01) /* Zoomed sprites          */
#define Sprites16   (VDP[1]&0x02) /* Sprites 8x16/8x8        */
#define LeftSprites (VDP[0]&0x08) /* Shift sprites 8pix left */
#define LeftMask    (VDP[0]&0x20) /* Mask left-hand column   */
#define NoTopScroll (VDP[0]&0x40) /* Don't scroll top 16l.   */
#define NoRghScroll (VDP[0]&0x80) /* Don't scroll right 16l. */
#define HBlankON    (VDP[0]&0x10) /* Generate HBlank inter-s */
#define VBlankON    (VDP[1]&0x20) /* Generate VBlank inter-s */
#define ScreenON    (VDP[1]&0x40) /* Show screen             */
#define SprX(N) (SprTab+0x80)[N<<1] /* Sprite X coordinate   */
#define SprY(N) SprTab[N]           /* Sprite Y coordinate   */
#define SprP(N) (SprTab+0x81)[N<<1] /* Sprite pattern number */
#define SprA(N) (SprTab+0x40)[N<<1] /* Sprite attributes ?   */
/*************************************************************/

extern byte *VRAM,*RAM;        /* Main and Video RAMs        */
extern byte VDP[16];           /* VDP registers              */
extern byte *ChrTab;           /* Character Table            */
extern byte *ChrPal,*SprPal;   /* Palette Tables             */
extern byte *SprTab,*SprGen;   /* Sprite Tables              */
extern byte BGColor;           /* Background color           */
extern byte ScrollX,ScrollY;   /* Screen scroll position     */
extern word Freq[4];           /* Frequencies, Hz            */
extern byte Volume[4];         /* Volumes, linear 0..255     */

/** StartSMS() ***********************************************/
/** Allocate memory, load ROM image, initialize hardware,   **/
/** CPU and start the emulation. This function returns 0 in **/
/** the case of failure.                                    **/
/*************************************************************/
int StartSMS(char *Cartridge);

/** TrashSMS() ***********************************************/
/** Free memory allocated by StartSMS().                    **/
/*************************************************************/
void TrashSMS(void);

/** InitMachine() ********************************************/
/** Allocate resources needed by the machine-dependent code.**/
/************************************ TO BE WRITTEN BY USER **/
int InitMachine(void);

/** TrashMachine() *******************************************/
/** Deallocate all resources taken by InitMachine().        **/
/************************************ TO BE WRITTEN BY USER **/
void TrashMachine(void);

/** RefreshLine() ********************************************/
/** Refresh line Y (0..191), including sprites in this line.**/
/************************************ TO BE WRITTEN BY USER **/
void RefreshLine(byte Y);

/** RefreshScreen() ******************************************/
/** Refresh screen. This function is called in the end if   **/
/** refresh cycle to show the screen.                       **/
/************************************ TO BE WRITTEN BY USER **/
void RefreshScreen(void);

/** SetColor() ***********************************************/
/** Allocate a new color (0..15 picture, 16..31 sprites).   **/
/************************************ TO BE WRITTEN BY USER **/
void SetColor(byte N,byte R,byte G,byte B);

/** Joysticks() **********************************************/
/** This function is called to poll joysticks. It returns:  **/
/**                                                         **/
/** PAU.G2.G1.STA.RES.B2.A2.R2.L2.D2.U2.B1.A1.R1.L1.D1.U1   **/
/**                                                         **/
/** Least significant bit on the right. Active value is 1.  **/
/************************************ TO BE WRITTEN BY USER **/
long int Joysticks(void);

/** Sound() **************************************************/
/** Set sound volume (0..255) and frequency (Hz) for a      **/
/** given channel (0..3). This function is only needed with **/
/** #define SOUND.                                          **/
/************************************ TO BE WRITTEN BY USER **/
void Sound(int Channel,int Freq,int Volume);

void CheckWindowSize(void);

#endif /* SMS_H */
