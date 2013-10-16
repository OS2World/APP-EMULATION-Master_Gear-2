/** Z80: portable Z80 emulator *******************************/
/**                                                         **/
/**                           Z80.h                         **/
/**                                                         **/
/** This file contains declarations relevant to emulation   **/
/** of Z80 CPU.                                             **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994,1995,1996            **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef Z80_H
#define Z80_H

#define INTERRUPTS             /* Compile interrupts code    */
/* #define DEBUG */            /* Compile debugging version  */
/* #define LSB_FIRST */        /* Compile for low-endian CPU */

#define S_FLAG      0x80
#define Z_FLAG      0x40
#define H_FLAG      0x10
#define P_FLAG      0x04
#define V_FLAG      0x04
#define N_FLAG      0x02
#define C_FLAG      0x01

/*************************************************************/
/** NOTICE: sizeof(byte)=1 and sizeof(word)=2               **/
/*************************************************************/
typedef unsigned char byte;
typedef unsigned short word;
typedef signed char offset;

/*************************************************************/
/** #define LSB_FIRST for machines where least signifcant   **/
/** byte goes first.                                        **/
/*************************************************************/
typedef union
{
#ifdef LSB_FIRST
  struct { byte l,h; } B;
#else
  struct { byte h,l; } B;
#endif
  word W;
} pair;

typedef struct
{
  pair AF,BC,DE,HL,IX,IY,PC,SP;
  pair AF1,BC1,DE1,HL1;
  byte IFF,I;
} reg;

/** Interrupts ***********************************************/
/** Interrupt-related variables.                            **/
/*************************************************************/
#ifdef INTERRUPTS
extern int  IPeriod;  /* Number of commands between internal interrupts */
extern byte IntSync;  /* Generate internal interrupts if IntSync==1     */
extern byte IFlag;    /* If IFlag==1, generate interrupt and set to 0   */
#endif

/** Debugging ************************************************/
/** Switches to turn tracing on and off in DEBUG mode.      **/
/*************************************************************/
#ifdef DEBUG
extern byte Trace;    /* Tracing is on if Trace==1  */
extern word Trap;     /* When PC==Trap, set Trace=1 */
#endif

/** Other ****************************************************/
/** Other variables to control emulator's behaviour.        **/
/*************************************************************/
extern byte TrapBadOps; /* When 1, print warnings of illegal Z80 ops */
extern byte CPURunning; /* When 0, execution terminates              */

/** ResetZ80() ***********************************************/
/** This function can be used to reset the register struct  **/
/** before starting execution with Z80(). It sets the       **/
/** registers to their supposed initial values.             **/
/*************************************************************/
void ResetZ80(reg *Regs);

/** Z80() ****************************************************/
/** This function will run Z80 code until CPURunning        **/
/** becomes 0. Registers are initialized from Regs. The PC  **/
/** at which emulation stopped is returned by this function.**/
/*************************************************************/
word Z80(reg Regs);

/** M_RDMEM() ************************************************/
/** Z80 emulation calls this function to read a byte from   **/
/** address A of Z80 address space.                         **/
/************************************ TO BE WRITTEN BY USER **/
extern byte *Page[];
// byte M_RDMEM(word A);
byte inline M_RDMEM(register word A)
{ return(Page[A>>13][A&0x1FFF]); }


/** M_WRMEM() ************************************************/
/** Z80 emulation calls this function to write byte V to    **/
/** address A of Z80 address space.                         **/
/************************************ TO BE WRITTEN BY USER **/
void M_WRMEM(word A,byte V);


/** DoIn() ***************************************************/
/** Z80 emulation calls this function to read a byte from   **/
/** a given I/O port.                                       **/
/************************************ TO BE WRITTEN BY USER **/
byte DoIn(byte Port);

/** DoOut() **************************************************/
/** Z80 emulation calls this function to write byte V to a  **/
/** given I/O port.                                         **/
/************************************ TO BE WRITTEN BY USER **/
void DoOut(byte Port,byte Value);

/** M_PATCH() ************************************************/
/** Z80 emulation calls this function when it encounters a  **/
/** special patch command (ED FE) provided for user needs.  **/
/** For example, it can be called to emulate BIOS calls,    **/
/** such as disk and tape access. Replace it with an empty  **/
/** macro for no patching.                                  **/
/************************************ TO BE WRITTEN BY USER **/
void M_PATCH(reg *R);

/** Debug() **************************************************/
/** This function should exist if DEBUG is #defined. If     **/
/** Trace==1, it is called after each Z80 command executed  **/
/** by the emulator and given pointer to the register file. **/
/************************************ TO BE WRITTEN BY USER **/
#ifdef DEBUG
void Debug(reg *R);
#endif

/** Interrupt() **********************************************/
/** This function should exist if INTERRUPTS is #defined.   **/
/** Z80 emulation calls it periodically to check if system  **/
/** hardware requires any interrupts. The function must     **/
/** return an address of the interrupt vector (0x0038,      **/
/** 0x0066, etc.) or 0xFFFF for no interrupt.               **/
/************************************ TO BE WRITTEN BY USER **/
#ifdef INTERRUPTS
word Interrupt(void);
#endif

#endif /* Z80_H */
