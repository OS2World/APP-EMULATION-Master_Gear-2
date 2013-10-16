/** MasterGear: portable MasterSystem/GameGear emulator ******/
/**                                                         **/
/**                           SMS.c                         **/
/**                                                         **/
/** This file contains implementation for the SMS-specific  **/
/** hardware: VDP, interrupts, etc. Initialization code and **/
/** definitions needed for the machine-dependent drivers    **/
/** are also here.                                          **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994,1995,1996            **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

#define INCL_WINDIALOGS   // OS/2 include for Window Dialog
#include <os2.h>


#include "SMS.h"
#include "dsp.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

    char Text[1000];
    char Title[100];

byte Verbose     = 1;             /* Debug messages ON/OFF switch     */
byte UPeriod     = 2;             /* Interrupts per screen update     */
int  VPeriod     = 8000;          /* Number of opcodes per VBlank     */
byte GameGear    = 0;             /* <>0: GameGear emulation          */
byte Country     = 0;             /* 0=Engl/Auto, 1=Engl, 2=Japanese  */
byte DelayedRD   = 0;             /* <>0: VRAM reads delayed by one   */
byte AutoA=0,AutoB=0;             /* Joystick autofire ON/OFF switch  */

byte *RAM,*VRAM,*PRAM;            /* Main, Video, and Palette RAMs    */

word JoyState;                    /* State of joystick buttons        */

byte VDP[16];                     /* VDP registers                    */
byte VDPStatus;                   /* VDP Status register              */
word VAddr;                       /* VRAM and Palette Address latches */
byte PAddr;
byte VKey,DKey,RKey;              /* VRAM Address and Access keys     */

byte *ChrTab;                     /* Character Name Table             */
byte *SprTab,*SprGen;             /* Sprite Attribute/Pattern Tables  */
byte *ChrPal,*SprPal;             /* Palette Tables                   */
byte BGColor;                     /* Background color                 */
byte ScrollX,ScrollY;             /* Current screen scroll values     */
int  CURLINE;                     /* Current scanline being displayed */
byte LinesLeft;                   /* Lines left before line interrupt */
byte MinLine,MaxLine;             /* Top/bottom scanlines to refresh  */

word Freq[4];                     /* Channel frequencies, Hz          */
byte Volume[4];                   /* Channel volumes, linear 0..15    */
byte NoiseMode;                   /* Noise channel modes              */

byte *Page[8];                    /* Page pointers to 8kB RAM pages   */
byte *ROMMap[256];                /* Up to 256x16kB ROM pages         */
int  ROMPages;                    /* Number of ROM pages              */
byte ROMMask;                     /* Mask to apply to ROM page number */

byte EnWrite;                     /* <>0: Enable write to 8000-BFFF   */
byte Battery;                     /* <>0: Save battery-backed SRAM    */
char *SaveName;                   /* Save file name (if SRAM present) */

byte SIODir,SIOData,SIOLink;

static void VDPOut(byte R,byte V);   /* Write into VDP register       */
static void PSGOut(byte V);          /* Write into sound chip         */
static byte CheckSprites(void);      /* Check for sprite collisions   */
static void SetScroll(void);         /* Update ScrollX/ScrollY        */


/** StartSMS() ***********************************************/
/** Allocate memory, load ROM image, initialize hardware,   **/
/** CPU and start the emulation. This function returns 0 in **/
/** the case of failure.                                    **/
/*************************************************************/
int StartSMS(char *Cartridge)
{
  static char *Countries[16] =
  {
    NULL,NULL,NULL,NULL,NULL,"Japan","Japan, USA, Europe","USA, Europe",
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
  };
  static char *Sizes[16] =
  {
    ">128kB",NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,"32kB","64kB","128kB",NULL
  };
  static byte VDPInit[16] =
  {
    0x00,0x60,0x0E,0x00,0x00,0x7F,0x00,0x00,
    0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00
  };

  FILE *F;
  int I,J,*T;
  char *P;
  reg R;

  /*** STARTUP CODE starts here: ***/
  T=(int *)"\01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
#ifdef LSB_FIRST
  if(*T!=1)
  {
    puts("********** This machine is high-endian. **********");
    puts("Take #define LSB_FIRST out and compile MasterGear again.");
    return(0);
  }
#else
  if(*T==1)
  {
    puts("********* This machine is low-endian. **********");
    puts("Insert #define LSB_FIRST and compile MasterGear again.");
    return(0);
  }
#endif

  RAM=VRAM=NULL;
  ROMPages=0;ROMMask=0;
  for(J=0;J<256;J++) ROMMap[J]=NULL;
  SaveName=NULL;Battery=0;

  if(Verbose) printf("Allocating 64kB for CPU address space...");
  if(!(RAM=(byte*)malloc(0x10000))) { if(Verbose) puts("FAILED");return(0); }
  memset(RAM,NORAM,0x10000);
  if(Verbose) printf("OK\nAllocating 16kB for Video address space...");
  if(!(VRAM=(byte*)malloc(0x4040))) { if(Verbose) puts("FAILED");return(0); }
  memset(VRAM,NORAM,0x4040);
  PRAM=VRAM+0x4000;

  if(Verbose) printf("OK\nOpening %s...",Cartridge);
  P=NULL;

#ifdef ZLIB
#define fopen          gzopen
#define fclose         gzclose
#define fread(B,N,L,F) gzread(F,B,(L)*(N))
#endif

  if(F=fopen(Cartridge,"rb"))
  {
    for(ROMPages=0;ROMPages<256;ROMPages++)
      if(!(ROMMap[ROMPages]=(byte*)malloc(0x4000)))
      { P="MALLOC FAILED";break; }
      else
      {
        J=fread(ROMMap[ROMPages],1,0x4000,F);
        if(!J)        { P=NULL;break; }
        if(J==512) // cartridge w/header
        {
           fclose(F);
           for(J=0;J<ROMPages;J++) free(ROMMap[J]);
           if(F=fopen(Cartridge,"rb"))
           {
              for(ROMPages=0;ROMPages<256;ROMPages++)
              if(!(ROMMap[ROMPages]=(byte*)malloc(0x4000)))
                 { P="MALLOC FAILED";break; }
              else
              {
                 if (ROMPages == 0) J=fread(ROMMap[ROMPages],1,512,F); // skip header
                 J=fread(ROMMap[ROMPages],1,0x4000,F);
                 if(!J)        { P=NULL;break; }
                 if(J!=0x4000) { P="SHORT FILE";break; }
              }
           }
           fclose(F);
           break;
        }
        if(J!=0x4000) { P="SHORT FILE";break; }
      }
    fclose(F);
  }
  else P="NOT FOUND";

#ifdef ZLIB
#undef fopen
#undef fclose
#undef fread
#endif

  /* Exit if there was a problem */
  if(P) { if(Verbose) puts(P);return(0); }

  /* I = Country Code, J = ROM Size */
  J=ROMMap[1][0x3FFF];I=J>>4;J&=0x0F;
  /* Enforce Japanese/English nationalization if needed */
  if(!Country) if(I==5) Country=2; else if(I==7) Country=1;

  /* Print out cartridge data if needed */
  if(Verbose)
  {
    printf("%d pages loaded\n",ROMPages);
    if(GameGear&&ROMMap[1])
    {
      if(Countries[I]) printf("  Country: %s\n",Countries[I]);
      else             printf("  Country: Unknown (%d)\n",I);
      if(Sizes[J])     printf("  Size:    %s\n",Sizes[J]);
      else             printf("  Size:    Unknown (%d)\n",J);
    }
  }

  /* Generate the .SAV file name and try to load it. */
  /* If found the .SAV file, assume battery backup.  */
    if(SaveName=(char*)malloc(strlen(Cartridge)+10))
  {
    strcpy(SaveName,Cartridge);
    if(P=strrchr(SaveName,'.')) strcpy(P,".sav");
    else strcat(SaveName,".sav");
    if(F=fopen(SaveName,"rb"))
    {
      if(Verbose) printf("Found %s...reading...",SaveName);
      J=(fread(RAM+0x8000,1,0x4000,F)==0x4000);
      if(Verbose) puts(J? "OK":"FAILED");
      fclose(F);
    }
  }

//  if(SndName)
//  {
//    if(Verbose) printf("Logging soundtrack to %s...",SndName);
//    SndStream=fopen(SndName,"wb");
//    if(Verbose) puts(SndStream? "OK":"FAILED");
//    if(SndStream)
//      fwrite
//      ("SND\032\001\004\062\0\0\0\0\0\0\0\0\0\376\003\002",1,19,SndStream);
//  }

  /* Calculate IPeriod from VPeriod */
  IPeriod=VPeriod/280;
  if(IPeriod<10) IPeriod=10;
  VPeriod=IPeriod*280;

  if(Verbose)
  {
    printf("Initializing CPU and System Hardware:\n");
    printf("  VBlank = %d opcodes\n  HBlank = %d opcodes\n",VPeriod,IPeriod);
  }

  for(J=1;J<ROMPages;J<<=1);
  ROMMask=J-1;
  EnWrite=0;
  RAM[0xC000]=RAM[0xDFFC]=0x00;
  Page[0]=ROMMap[RAM[0xDFFD]=0&ROMMask];
  Page[1]=Page[0]+0x2000;
  Page[2]=ROMMap[RAM[0xDFFE]=1&ROMMask];
  Page[3]=Page[2]+0x2000;
  Page[4]=ROMMap[RAM[0xDFFF]=2&ROMMask];
  Page[5]=Page[4]+0x2000;
  Page[6]=Page[7]=RAM+0xC000;

  memcpy(VDP,VDPInit,16);                /* VDP registers              */
  VKey=DKey=RKey=1;                      /* Accessing VRAM...          */
  VAddr=0x0000;PAddr=0;                  /* ...from address 0000h      */
  BGColor=0;                             /* Background color           */
  CURLINE=0;                             /* Current scanline           */
  VDPStatus=0x00;                        /* VDP status register        */
  ChrTab=VRAM+0x3800;                    /* Screen buffer at 3800h     */
  SprTab=VRAM+0x3F00;                    /* Sprite attributes at 3F00h */
  SprGen=VRAM;                           /* Sprite patterns at 0000h   */
  ChrPal=PRAM;                           /* Screen palette             */
  SprPal=PRAM+(GameGear? 0x20:0x10);     /* Sprite palette             */
  MinLine=GameGear? (192-144)/2:0;       /* First scanline to refresh  */
  MaxLine=MinLine+(GameGear? 143:191);   /* Last scanline to refresh   */
  LinesLeft=255;                         /* Lines left to line inte-pt */
  SetScroll();                           /* ScrollX,ScrollY            */
  JoyState=0xFFFF;                       /* Joypad state               */
  NoiseMode=0x00;                        /* Noise channel modes        */
  Freq[0]=Freq[1]=Freq[2]=Freq[3]=0;     /* Sound channel frequencies  */
  Volume[0]=Volume[1]=0;                 /* Sound channel volumes      */
  Volume[2]=Volume[3]=0;
  ResetZ80(&R);                          /* Reset Z80 registers        */

  if(Verbose) printf("RUNNING ROM CODE...\n");
  J=Z80(R);

  if(Verbose) printf("EXITED at PC = %04Xh.\n",J);
  return(1);
}


/** TrashSMS() ***********************************************/
/** Free memory allocated by StartSMS().                    **/
/*************************************************************/
void TrashSMS(void)
{
  FILE *F;
  int J;

  /* If battery backup present, write .SAV file */
  if(SaveName&&Battery)
  {
    if(Verbose) printf("\nOpening %s...",SaveName);
    if(F=fopen(SaveName,"wb"))
    {
      if(Verbose) printf("writing...");
      J=(fwrite(RAM+0x8000,1,0x4000,F)==0x4000);
      if(Verbose) puts(J? "OK":"FAILED");
      fclose(F);
    }
    else if(Verbose) puts("FAILED");
  }

  for(J=0;J<ROMPages;J++) free(ROMMap[J]);
//  if(SndStream) fclose(SndStream);
  if(SaveName)  free(SaveName);
  if(RAM)       free(RAM);
  if(VRAM)      free(VRAM);
}


/** M_WRMEM() ************************************************/
/** Z80 emulation calls this function to write byte V to    **/
/** address A of Z80 address space.                         **/
/*************************************************************/
void M_WRMEM(register word A,register byte V)
{
  register byte J;

  if(A>0xFFFB)
  {
    if(Verbose&0x08) printf("SWITCH[%d] = %02Xh\n",A&3,V);
    switch(A&3)
    {
      case 0: if(EnWrite=V&0x08)
              { Battery=1;Page[4]=RAM+(V&0x04? 0x4000:0x8000); }
              else
              { J=RAM[0xDFFF];Page[4]=ROMMap[J]? ROMMap[J]:RAM; }
              Page[5]=Page[4]+0x2000;
              break;
      case 1: V&=ROMMask;
              Page[0]=ROMMap[V]? ROMMap[V]:RAM;
              Page[1]=Page[0]+0x2000;
              break;
      case 2: V&=ROMMask;
              Page[2]=ROMMap[V]? ROMMap[V]:RAM;
              Page[3]=Page[2]+0x2000;
              break;
      case 3: V&=ROMMask;
              if(!EnWrite) Page[4]=ROMMap[V]? ROMMap[V]:RAM;
              Page[5]=Page[4]+0x2000;
              break;
    }
  }

  switch(A>>13)
  {
    case 6:
    case 7:  Page[6][A&0x1FFF]=V;return;
    case 4:  if(EnWrite) Page[4][A&0x1FFF]=V;return;
    case 5:  if(EnWrite) Page[5][A&0x1FFF]=V;return;
    default: if(Verbose&0x02) printf("WRITE %02Xh to %04Xh\n",V,A);
  }
}


/** M_RDMEM() ************************************************/
/** Z80 emulation calls this function to read a byte from   **/
/** address A of Z80 address space.                         **/
/*************************************************************/
// byte M_RDMEM(register word A)
// { return(Page[A>>13][A&0x1FFF]); }   // moved to z80.h for inlining


/** M_PATCH() ************************************************/
/** Z80 emulation calls this function when it encounters a  **/
/** special patch command (ED FE) provided for user needs.  **/
/*************************************************************/
void M_PATCH(reg *R) { }


/** DoIn() ***************************************************/
/** Z80 emulation calls this function to read a byte from   **/
/** a given I/O port.                                       **/
/*************************************************************/
byte DoIn(register byte Port)
{
  switch(Port)
  {
    /* Joypad ports */
    case 0xC0:                                /* Used by Pit-Pot cart    */
    case 0xDC: return(JoyState&0xFF);         /* D2 U2 B1 A1 R1 L1 D1 U1 */
    case 0xC1:                                /* Used by Pit-Pot cart    */
    case 0xDD: return((JoyState>>8)|0x20);    /* G2 G1 1 RES B2 A2 R2 L2 */

    /* GameGear START button and nationalization */
    case 0x00: return(((JoyState>>6)&0x80)|(Country>1? 0x00:0x40));

    /* GameGear Serial IO */
    case 0x01: return(SIOLink);               /* GameGear SIO Link Port  */
    case 0x04: return(0x00);                  /* GameGear SIO Data       */
    case 0x05: return(0x04);                  /* GameGear SIO Status     */

    case 0x7E: return(CURLINE<256? CURLINE:255); /* Current scanline     */

    case 0xBD:                                /* Used by Sailor Moon     */
    case 0xBF:                                /* VDP Status Register     */
      Port=VDPStatus;VDPStatus&=0x3F;return(Port);

    case 0xBE:                                /* VDP Data Port           */
      if(DKey)
      {
        Port=VRAM[VAddr];
        if(RKey&&DelayedRD) RKey=0;
        else VAddr=(VAddr+1)&0x3FFF;
      }
      else
      {
        Port=PRAM[PAddr];
        PAddr=(PAddr+1)&(GameGear? 0x3F:0x1F);
      }
      return(Port);
  }

  if(Verbose&0x02) printf("READ from IO port %02Xh\n",Port);

//  sprintf(Title,"Read from an unsupported IO port");
//  sprintf(Text,"IO Port %02Xh",Port);
//  WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, Text, Title, 0, MB_OK | MB_INFORMATION );

  return(NORAM);
}


/** DoOut() **************************************************/
/** Z80 emulation calls this function to write byte V to a  **/
/** given I/O port.                                         **/
/*************************************************************/
void DoOut(register byte Port,register byte V)
{
  static byte Addr;

  switch(Port)
  {

case 0x7E: /* Sound Subsystem */
case 0x7F:
  PSGOut(V);return;

case 0x3E: /* RAM at C000h (battery backed?) */
  return;

case 0x3F: /* Lightgun/Localization */
  switch(V&0x0F)
  {
    case 5: /* Set localization */
      JoyState&=0x3FFF;
      if(Country>1) V=~V;
      JoyState|=(word)(V&0x80)<<8;
      JoyState|=(word)(V&0x20)<<9;
      break;
    default:
      JoyState|=0xC000;
      break;
  }
  return;

/* GG Serial IO */
case 0x01: SIOLink=V;return;
case 0x02:
case 0x03:
case 0x05:
case 0x06: return;

case 0xBE: /* VDP Data Port */
  VKey=1;
  if(DKey) { VRAM[VAddr]=V;VAddr=(VAddr+1)&0x3FFF;RKey=0; }
  else
  {
    byte N,R,G,B;

    PRAM[PAddr]=V;
    if(GameGear)
    {
      N=PRAM[PAddr&0xFE];
      R=(N<<4)&0xE0;
      G=N&0xE0;
      B=(PRAM[PAddr|0x01]<<4)&0xE0;
      N=PAddr>>1;
      PAddr=(PAddr+1)&0x3F;
    }
    else
    {
      R=V<<6;
      G=(V<<4)&0xC0;
      B=(V<<2)&0xC0;
      N=PAddr;
      PAddr=(PAddr+1)&0x1F;
    }
    SetColor(N,R,G,B);
  }
  return;

case 0xBD: /* Used by Sailor Moon */
case 0xBF: /* VDP Address Port */
  if(VKey) { Addr=V;VKey=0; }
  else
  {
    VKey=1;
    switch(V&0xF0)
    {
      case 0xC0:
      case 0xD0:
      case 0xE0:
      case 0xF0: PAddr=Addr&(GameGear? 0x3F:0x1F);DKey=0;break;
      case 0x80: VDPOut(V&0x0F,Addr);break;
      default:   VAddr=(((word)V<<8)+Addr)&0x3FFF;DKey=1;
                 RKey=V&0x40;break;
    }
  }
  return;

  }

  if(Verbose&0x02) printf("WRITE %02Xh to IO port %02Xh\n",V,Port);
}


/** Interrupt() **********************************************/
/** Z80 emulation calls this function periodically to check **/
/** if any interrupts are required. The function must       **/
/** return 0xFFFF for no interrupts, or an address of the   **/
/** interrupt vector (0x0038, 0x0066, etc.).                **/
/*************************************************************/
word Interrupt(void)
{
  static byte NeedVBlank = 0;
  static byte UCount = 0;
  static byte ACount = 0;
  register long int J;

  CURLINE=(CURLINE+1)%280;

  if(CURLINE==193)
  {

    /* Polling joypads */
    J=Joysticks();

    /* We preserve lightgun bits */
    JoyState=(JoyState&0xC000)|(~J&0x3FFF);

    if(GameGear)
    {
      JoyState&=(JoyState>>6)|0xFFC0; /* A single joypad */
      JoyState|=0x1FC0;               /* No RESET button */
      J&=0xFFFF;                      /* No PAUSE button */
    }
    else
    {
      JoyState&=(JoyState>>9)|0xFFEF; /* [START] = [A]   */
    }

//    /* Autofire emulation */
//    ACount=(ACount+1)&0x07;
//    if(ACount>3)
//    {
//      if(AutoA) JoyState|=0x0410;
//      if(AutoB) JoyState|=0x0820;
//    }
//
    /* If PAUSE pressed, generate NMI */
    if(J&0x10000) return(0x0066);
  }

  if(CURLINE>=193)
  { if((CURLINE<224)&&(VDPStatus&0x80)&&VBlankON&&VKey) return(0x0038); }
  else
  {
    if(!UCount&&(CURLINE>=MinLine)&&(CURLINE<=MaxLine))
    {
      if(!CURLINE||(CURLINE==16)) SetScroll();
      RefreshLine(CURLINE);
    }

    if(CURLINE==MaxLine)
    {
      /* Checking for sprite collisions */
      if(CheckSprites()) VDPStatus|=0x20;
      else               VDPStatus&=0xDF;

      /* Refreshing sprites and display */
       RefreshScreen();
    }

    /* We need VBlank at line 192 */
    if(CURLINE==192) NeedVBlank=1;

    /* Generate line interrupts */
    if(!CURLINE) LinesLeft=VDP[10];
    if(LinesLeft) LinesLeft--;
    else { LinesLeft=VDP[10];VDPStatus|=0x40; }

    /* If line interrupt is generated in line 192, */
    /* the VBlank flag is not set yet.             */
    if((VDPStatus&0x40)&&HBlankON) return(0x0038);
  }

  /* Setting VBlank flag if needed */
  if(NeedVBlank) { NeedVBlank=0;VDPStatus|=0x80; }

  return(0xFFFF);
}


/** VDPOut() *************************************************/
/** Emulator calls this function to write byte V into a VDP **/
/** register R.                                             **/
/*************************************************************/
void VDPOut(register byte R,register byte V)
{
  if(Verbose&0x20) printf("VDP[%d]=%02Xh in line %d\n",R,V,CURLINE);

  switch(R)
  {
    case 2:  ChrTab=VRAM+(((word)V<<10)&0x3800);break;
    case 5:  SprTab=VRAM+(((word)V<< 7)&0x3F00);break;
    case 6:  SprGen=VRAM+(((word)V<<11)&0x2000);break;
    case 7:  BGColor=V&0x0F;break;
    case 8:
    case 9:  VDP[R]=V;SetScroll();return;
//    default:
//       sprintf(Title,"Write to an unsupported VDP port");
//       sprintf(Text,"VDP[%d]=%02Xh",R,V);
//       WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, Text, Title, 0, MB_OK | MB_INFORMATION);

  }

  VDP[R]=V;
}


/** PSGOut() *************************************************/
/** Emulator calls this function to write byte V into PSG   **/
/** chip.                                                   **/
/*************************************************************/
void PSGOut(register byte V)
{
  static byte Buf;
  register byte N;
  register long J;

  switch(V&0xF0)
  {
    case 0xE0:
      NoiseMode=V&0x03;
      switch(V&0x03)
      {
        case 0: Freq[3]=20000;break;
        case 1: Freq[3]=10000;break;
        case 2: Freq[3]=5000;break;
        case 3: Freq[3]=Freq[2];break;
      }
      Play(3,Freq[3],Volume[3]);
      break;
    case 0x80: case 0xA0: case 0xC0:
      Buf=V;break;
    case 0x90: case 0xB0: case 0xD0: case 0xF0:
      N=(V-0x90)>>5;
      Volume[N]=(~V&0x0F); // (~V&0x0F)<<4;
      Play(N,Freq[N],Volume[N]);
      if(Verbose&0x10) printf("SOUND %d: Volume=%d\n",N,Volume[N]);
      break;
    default:
      if(Buf&0xC0)
      {
        N=(Buf-0x80)>>5;
        J=(V&0x3F)*16+(Buf&0x0F)+1;
        J=SOUNDBASE/J;
        Freq[N]=J<65536? J:0;
        if((N==2)&&(NoiseMode==3))
          Play(3,Freq[3]=Freq[2],Volume[3]);
        Play(N,Freq[N],Volume[N]);
        if(Verbose&0x10) printf("SOUND %d: Freq=%dHz\n",N,Freq[N]);
      }
  }
}


/** CheckSprites() *******************************************/
/** This function is periodically called to check for the   **/
/** sprite collisions and set appropriate bit in the VDP    **/
/** status register.                                        **/
/*************************************************************/
byte CheckSprites(void)
{
  register int Max,N1,N2,X1,X2,Y1,Y2,H;

  for(Max=0;(Max<64)&&(SprY(Max)!=208);Max++);
  H=Sprites16? 16:8;

  for(N1=0;N1<Max-1;N1++)
  {
    Y1=SprY(N1)+1;X1=SprX(N1);
    if((Y1<192)||(Y1>240))
      for(N2=N1+1;N2<Max;N2++)
      {
        Y2=SprY(N2)+1;X2=SprX(N2);
        if((Y2<192)||(Y2>240))
          if((X2>X1-8)&&(X2<X1+8)&&(Y2>Y1-H)&&(Y2<Y1+H)) return(1);
      }
  }

  return(0);
}


/** SetScroll() **********************************************/
/** Update ScrollX/ScrollY variables.                       **/
/*************************************************************/
void SetScroll(void)
{
  ScrollX=NoTopScroll&&(CURLINE<MinLine+16)? 0:256-VDP[8];
  ScrollY=VDP[9];
}


/** Play() ***************************************************/
/** Log and play sound of given frequency (Hz) and volume   **/
/** (0..255) via given channel (0..3).                      **/
/*************************************************************/
//void Play(int C,int F,int V)
//{
//  Sound(C,F,V);
//}
