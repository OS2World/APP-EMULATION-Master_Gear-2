/** MasterGear: portable MasterSystem/GameGear emulator ******/
/**                                                         **/
/**                           MG.c                          **/
/**                                                         **/
/** This file contains generic main() procedure starting    **/
/** the emulation.                                          **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994,1995,1996            **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/**                                                         **/
/** OS/2 modifications by Darrell Spice Jr.                 **/
/*************************************************************/

#include "SMS.h"
#include "sndos2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os2term.h"
#define INCL_WINSTDFILE   // OS/2 include for File Dialog
#define INCL_WINDIALOGS   // OS/2 include for Window Dialog
#include <os2.h>
int newgame = FALSE;
int firstgame = 0;
char* MgHelp;
char* MgIni;
extern int buflen;           // 1536
extern ULONG AUDIOfreq;      // = 44100;
extern int AudioOK;          // = true;
extern int frt;              // 0 = NTSC(60fps) 1 = PAL(50fps)
extern int volume;           // 0 = off, 1 = low, 2 = high


extern char *Title;      /* Program title                       */
extern int   UseSound;   /* Sound mode (#ifdef SOUND)           */
extern int   UseSHM;     /* Use SHM X extension (#ifdef MITSHM) */
extern int   SaveCPU;    /* Pause when inactive (#ifdef UNIX)   */
extern int   SyncScreen; /* Sync screen updates (#ifdef MSDOS)  */
extern char *BackName;   /* Background picture (#ifdef GIFLIB)  */



int main(int argc,char *argv[])
{
   char *CartName,*P;
   char filename[CCHMAXPATH] = "";
   int N,J,I,NoExt, i;

   TrapBadOps=0;
   VPeriod=8000;
   UPeriod=2;
   AutoA=AutoB=0;
   IntSync=1;
   Verbose=1;
   GameGear=0;
   Country=0;
   DelayedRD=0;
   CartName="CART.ROM";
   buflen = 768;
   AUDIOfreq = 22050;
   AudioOK = TRUE;
   frt = 0;

// find the OS/2 readme file for Master Gear
   MgHelp = (char*) malloc(CCHMAXPATH);
   memset(MgHelp, 0, CCHMAXPATH);

   char* separator = strrchr(argv[0], PATH_SEPARATOR);

   if(separator != 0)
   {
      strncpy(MgHelp, argv[0], separator - argv[0]);
      strcat(MgHelp, "\\mg.inf");
   }
   else
      strcpy(MgHelp, "mg.inf");

// find the OS/2 INI file for Master Gear
   MgIni = (char*) malloc(CCHMAXPATH);
   memset(MgIni, 0, CCHMAXPATH);

   separator = strrchr(argv[0], PATH_SEPARATOR);

   if(separator != 0)
   {
      strncpy(MgIni, argv[0], separator - argv[0]);
      strcat(MgIni, "\\mg.ini");
   }
   else
      strcpy(MgIni, "mg.ini");

    for (i = 0;i<argc;i++)
       firstgame = i;


   if (firstgame != 0)
      strcpy(filename, argv[firstgame]);

   if(!InitMachine()) return(1);

   for(;;)
   {
      if (GetCartridge(filename) == FALSE) /* display file dialog to get cartridge */
         break;
      newgame = FALSE;
      if(P=strrchr(filename,'.'))
      {
         if(!strcmp(P,".gg")||!strcmp(P,".GG"))   GameGear=1;
         if(!strcmp(P,".sms")||!strcmp(P,".SMS")) GameGear=0;
      }
      CheckWindowSize(); // resize window if changing from GG to SMS
      StartSMS(filename);
      audio_quiet();
      TrashSMS();
      if (newgame == FALSE) break;
   }

   TrashMachine();
   return(0);
}
