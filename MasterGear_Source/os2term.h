/*************************************************************/
/**                                                         **/
/**                           os2.h                         **/
/**                                                         **/
/** This file contains OS/2 dependent subroutines and       **/
/** drivers.                                                **/
/**                                                         **/
/** Copyright (C) Darrell Spice Jr, 1997                    **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

#ifndef OS_2term
#define OS_2term

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#define INCL_DOSPROCESS
#include <os2.h>

/* DIVE required header files */
#define  _MEERROR_H_
#include <mmioos2.h>
#include <dive.h>
#include <fourcc.h>

typedef unsigned long ulong;

int OpenWindow(void);
int CloseWindow(void);
int GetCartridge(char *);
int DiveMap(void);
int DiveUnMap(void);

static int skiprate[16][16] =                   // skip(0)  show(1)
     { 1,1,1,1, 1,1,1,1,  1,1,1,1, 1,1,1,1,     //    0       16     60
       1,1,1,1, 1,1,1,1,  0,1,1,1, 1,1,1,1,     //    1       15     56.25
       1,1,1,1, 1,1,1,0,  1,1,1,1, 1,1,1,0,     //    2       14     52.5
       1,1,1,1, 0,1,1,1,  1,0,1,1, 1,1,0,1,     //    3       13     48.75
       1,1,1,0, 1,0,1,1,  1,1,0,1, 0,1,1,1,     //    4       12     45
       1,1,1,0, 1,1,0,1,  1,0,1,1, 0,1,1,0,     //    5       11     41.25
       1,1,0,1, 1,0,1,0,  1,1,0,1, 1,0,1,0,     //    6       10     37.5
       1,0,1,0, 1,0,1,0,  1,1,0,1, 0,1,0,1,     //    7        9     33.75
       1,0,1,0, 0,1,0,1,  1,0,1,0, 0,1,0,1,     //    8        8     30
       1,0,1,0, 1,0,1,0,  0,1,0,1, 0,1,0,0,     //    9        7     26.25
       1,0,0,1, 0,1,0,0,  1,0,0,1, 0,0,1,0,     //   10        6     22.5
       1,0,0,0, 1,0,0,1,  0,0,1,0, 0,1,0,0,     //   11        5     18.75
       1,0,0,0, 0,1,0,0,  0,0,0,1, 0,0,1,0,     //   12        4     15
       1,0,0,0, 0,1,0,0,  0,0,1,0, 0,0,0,0,     //   13        3     11.25
       1,0,0,0, 0,0,0,0,  0,1,0,0, 0,0,0,0,     //   14        2      7.5
       1,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0 };   //   15        1      3.75



static double fps[2][16] = {60    , 56.25 , 52.50 , 48.75,  /* target frame rates for 60fps */
                            45    , 41.25 , 37.50 , 33.75,
                            30    , 26.25 , 22.50 , 18.75,
                            15    , 11.25 ,  7.50 ,  3.75,

                            50    , 46.875, 43.75 , 40.625,  /* target frame rates for 50fps */
                            37.5  , 34.375, 31.25 , 28.125,
                            25    , 21.875, 18.75 , 15.625,
                            12.5  , 9.375 ,  6.25 ,  3.125};

#endif

