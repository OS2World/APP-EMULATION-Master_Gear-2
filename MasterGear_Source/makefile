# makefile
# Created by IBM WorkFrame/2 MakeMake at 22:58:17 on 3 Jan 1998
#
# The actions included in this make file are:
#  Compile::Resource Compiler
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind

.SUFFIXES: .c .obj .rc .res 

.all: \
    E:\C_Projects\MasterGear_Source\mg.exe

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s %|dpfF.RES

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DOS_2 /DLSB_FIRST /Tdp /O /Gm /G5 /Gf /Gi /Oi /Fo"%|dpfF.obj" /C %s

E:\C_Projects\MasterGear_Source\mg.exe: \
    E:\C_Projects\MasterGear_Source\joyos2.obj \
    E:\C_Projects\MasterGear_Source\z80.obj \
    E:\C_Projects\MasterGear_Source\dsp.obj \
    E:\C_Projects\MasterGear_Source\mg.obj \
    E:\C_Projects\MasterGear_Source\os2term.obj \
    E:\C_Projects\MasterGear_Source\sms.obj \
    E:\C_Projects\MasterGear_Source\sndos2.obj \
    E:\C_Projects\MasterGear_Source\termos2.res \
    {$(LIB)}mmpm2.lib
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
     /B" /exepack:2 /st:65535 /pmtype:pm"
     /FeE:\C_Projects\MasterGear_Source\mg.exe 
     mmpm2.lib 
     E:\C_Projects\MasterGear_Source\joyos2.obj
     E:\C_Projects\MasterGear_Source\z80.obj
     E:\C_Projects\MasterGear_Source\dsp.obj
     E:\C_Projects\MasterGear_Source\mg.obj
     E:\C_Projects\MasterGear_Source\os2term.obj
     E:\C_Projects\MasterGear_Source\sms.obj
     E:\C_Projects\MasterGear_Source\sndos2.obj
<<
    rc.exe E:\C_Projects\MasterGear_Source\termos2.res E:\C_Projects\MasterGear_Source\mg.exe

E:\C_Projects\MasterGear_Source\termos2.res: \
    E:\C_Projects\MasterGear_Source\termos2.rc \
    {$(INCLUDE)}mg.ico \
    {$(INCLUDE)}dialog.h \
    {$(INCLUDE)}os2res.h

E:\C_Projects\MasterGear_Source\sndos2.obj: \
    E:\C_Projects\MasterGear_Source\sndos2.c \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}DSP.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}sndos2.h

E:\C_Projects\MasterGear_Source\sms.obj: \
    E:\C_Projects\MasterGear_Source\sms.c \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}Z80.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}DSP.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}SMS.h

E:\C_Projects\MasterGear_Source\os2term.obj: \
    E:\C_Projects\MasterGear_Source\os2term.c \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}Z80.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}DSP.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}SMS.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}sndos2.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}os2term.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}os2res.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}joyos2.h

E:\C_Projects\MasterGear_Source\mg.obj: \
    E:\C_Projects\MasterGear_Source\mg.c \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}Z80.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}SMS.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}sndos2.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}os2term.h

E:\C_Projects\MasterGear_Source\dsp.obj: \
    E:\C_Projects\MasterGear_Source\dsp.c \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}DSP.h

E:\C_Projects\MasterGear_Source\z80.obj: \
    E:\C_Projects\MasterGear_Source\z80.c \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}Z80.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}Tables.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}CodesCB.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}CodesXCB.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}CodesED.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}CodesXX.h \
    {E:\C_Projects\MasterGear_Source;$(INCLUDE);}Codes.h
