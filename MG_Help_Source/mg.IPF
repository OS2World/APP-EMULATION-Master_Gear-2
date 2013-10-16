.**********************************************************
.*
.* IPF for MasterGear/2
.*
.**********************************************************
:userdoc.
:title.MasterGear/2 Ver 1.0.5 released Aug. 9, 1998
.*
.**********************************************************
.*
:h1.About MasterGear/2

:p.
MasterGear is a portable emulator of the old Sega Master System, written in C by Marat Fayzullin.
You can play most Master System games with it.  As an added bonus Sega Game Gear games can also
be played because the Game Gear is really a shrunk down version of the Master System with some minor
modifications. The latest news, code and binaries for MasterGear can be found at the MasterGear Homepage&colon.
.br
:hp4.http&colon.//www.komkon.org/fms/MG/:ehp4.
:p.
MasterGear/2 is the OS/2 version, ported by Darrell Spice Jr.
MasterGear/2 can be found at the SpiceWare Homepage(along with other emulators for OS/2) at&colon.
.br
:hp4.http&colon.//www.geocities.com/~spiceware:ehp4.
.br
or
.br
:hp4.http&colon.//www.geocities.com/SiliconValley/Pines/2281:ehp4.
:p.
If you are having problems with MasterGear/2 please visit the SpiceWare Homepage and
make sure you're running the lastest release before sending mail. OS/2 version
comments should be mailed to Darrell at :hp4.spice@ibm.net:ehp4..
.*
.**********************************************************
.*
:h1.What's New
:p.
New features in version 1.0.5 of MasterGear/2.
:ul.
:li.Performance improvements.
:li.Workaround for black-bitmap problem which showed up in FixPack 5.
:li.MasterGear now begins in ROMS directory, if present.
:li.Improved throttle routine for faster machines.
:li.Fixed window resize problem, occured if only Master System games had been played,
the default window size was bigger than 1, and the oscilliscope was turned on using the O key.
:li.Source code now available, to hopefully drive new emulator ports &colon.-)
:eul.
.*
.**********************************************************
.*
:h1.Installation
:p.
To install MasterGear/2, just unzip the archive in your OS/2 games directory.  The directory :hp4.MG:ehp4. will
be created, along with the files and proper subdirectory structure.
:p.
You may install MasterGear/2 in a directory (or path) that containes spaces, such as :hp4.D&colon.\OS2 Games\MG:ehp4., without any problems.
.*
.**********************************************************
.*
:h1.Running Master/2
:p.
Run MasterGear/2 as you would any other program.
:p.
After starting MasterGear/2, a title screen will show for about 5 seconds.  Next a file open dialog will be displayed so that a cartridge can be selected for
play.  Cartridges are located in the ROMS directory. Install any additional cartridge images you wish here.
:p.
MasterGear/2 may also be started by dropping a cartridge onto it's icon.  In this case, the file open dialog will not appear.
:note. You must supply your own cartridge images, none are included with MasterGear/2.
.*
.**********************************************************
.*
:h1.ROM Images
:p.
Please note that ROM images have a copyright.  You should only download games
that you already own.  Do not bother to ask the authors of MasterGear where
game images may be found, you will not get a reply.
:p.
The file extension of the ROM images must be .GG for Game Gear or .SMS for
Master System in order for the window size and other machine specific items to
be initialized correctly. Game Gear games run in a 160 x 144 window and Master
System games run at 256 x 192.  Game Gear games have a wider range of colors
to choose from than Master System games.  If a .SMS game looks to small for the
window, rename it to .GG, and conversely if a .GG game looks to big for the
window, rename it to .SMS.  If the colors don't look correct, then try the
other extension.
:p.
Cartridge images should be a multiple of 32K (32768).  Some images may have
a 512 byte header, and MasterGear/2 has been modified to ignore the
header if it finds one.  Games will load faster if they don't have the 512
byte header.
:p.
After starting MasterGear/2, a file open dialog will be displayed so that a
cartridge can be selected for play.  Of course, if you drop a cartridge onto
MasterGear/2 to start it, the open dialog will not be displayed.
There are several ways to get a ROM image:
:ol.
:li.It's possible to build a device that plugs into the printer port
of a PC and reads the data off the cartridge (See the MasterGear home
page for more information)
:li.Search around the net and find some ROM images (.SMS or .GG files) to download
:eol.
:hp8.:warning.
It is illegal to have ROM images of games that you do not
actually own!
:ewarning.:ehp8.
.*
.**********************************************************
.*
:h2.Known Problems
:p.
This section contains information about known problems with some cartridges:
:ul.
:li.:hp4.GP Rider:ehp4.
:ul compact.
:li.The road sometimes gets distorted. Garbage when selecting the motorcycle.
:li.Use -delay to fix it, at least partially. :hp8.Option not yet enabled for OS/2:ehp8.
:eul.
:li.:hp4.XMen:ehp4.
:ul compact.
:li.Use -vperiod 7500. :hp8.Option not yet enabled for OS/2:ehp8.
:eul.
:li.:hp4.Altered Beast:ehp4.
:ul compact.
:li.Restarts at the title.
:li.Use -vperiod 8500. :hp8.Option not yet enabled for OS/2:ehp8.
:eul.
:li.:hp4.Dynamite Headdy:ehp4.
:ul compact.
:li.Screen sometimes blinks.
:li.Use -vperiod 10000.:hp8.Option not yet enabled for OS/2:ehp8.
:eul.
:li.:hp4.Galaxy Force:ehp4.
:ul.
:li.Some garbage at the top of a screen in the Green World.
:eul.
:li.:hp4.Alex Kidd Lost Stars:ehp4.
:ul.
:li.Palette gets reset to zeroes.
:eul.
:li.:hp4.Surfing Ninjas:ehp4.
:ul.
:li.Garbage at the title screens.
:eul.
:li.:hp4.Sonic Drift 2:ehp4.
:ul.
:li.Screen sometimes blinks.
:eul.
:li.:hp4.Mickey Mouse GG:ehp4.
:ul.
:li.Some problems after being killed in the second level.
:eul.
:li.:hp4.Mahjong GG:ehp4.
:ul.
:li.Distorted screen.
:eul.
:li.:hp4.Astro Warrior:ehp4.
:ul.
:li.Remains of blown cannons are distorted
:eul.
:eul.
.*
.**********************************************************
.*
:h1.Sound
:p.
The only requirement for sound support is MMPM/2, DART
is not required.  Sound support does impose a performance penalty.  It may be desirable to turn it
off on slower systems.  Sound can be disabled by turning off the volume in the
:link reftype=hd viewport dependent res=103.
settings dialog.
:elink.
.*
.**********************************************************
.*
:h1.DIVE
:p.
MasterGear/2 requires your video card to support DIVE.
:note. If the colors are incorrect, or the game image is offset you may need to either
disable EnDIVE support, such as with a Matrox Millenium card, or update your video drivers.
.*
.**********************************************************
.*
:h1.Menu Bar
:p.
The menu bar allows you to select options for MasterGear/2.
:p.
:artwork align=center name='titlescreen.bmp'.
:artlink.
:link reftype=hd viewport dependent res=200 x=6 y=232 cx=32 cy=32.
:link reftype=hd viewport dependent res=201 x=42 y=232 cx=31 cy=32.
:link reftype=hd viewport dependent res=202 x=78 y=232 cx=32 cy=32.
:link reftype=hd viewport dependent res=203 x=114 y=232 cx=32 cy=32.
:eartlink.
:p.
Select the buttons to view what they do.


:h2 res=200.New Game
:p.
:artwork name='mng.bmp'.
:p.
Brings up cartridge selection dialog.  The button represents a stack of game cartridges.
:Note. Games may also be drag and dropped onto MasterGear/2
:h2 res=201.MasterGear/2 Settings
:p.
:artwork name='mdb.bmp'.
:p.
Brings up the
:link reftype=hd viewport dependent res=103.
settings dialog
:elink.
for MasterGear/2.
:h2 res=202.Cartridge Help
:p.
:artwork name='mch.bmp'.
:p.
Call up online help for current game cartridge.
:note.In order for this to work, you must have the game documentation stored in
the DOCS directory as specified in the
:link reftype=hd viewport dependent res=102.
Online Documentation
:elink.
section.
:h2 res=203.MasterGear/2 Help
:p.
:artwork name='msh.bmp'.
:p.
Calls up the documentation which you are currently reading.

.*
.**********************************************************
.*
:h1 res=103.Settings Dialog
:p.
:artwork align=center name='settings.bmp'.
:artlink.
:link reftype=hd viewport dependent res=212 x=268 y=52 cx=239 cy=283.
:link reftype=hd viewport dependent res=211 x=4 y=52 cx=261 cy=212.
:link reftype=hd viewport dependent res=210 x=4 y=276 cx=261 cy=58.
:eartlink.
:p.
The settings dialog lets you control various aspects of MasterGear.  This display
shows the default settings for the parameters.  The settings are saved when
you exit MasterGear/2 in the file :hp4.MG.INI:ehp4..  Select any area for more information.
:p.
The :hp4.OK:ehp4. button
will save the changes, :hp4.Cancel:ehp4. will ignore the changes (except for joystick
recalibration), and :hp4.Help:ehp4. will bring you to this section of the help file.
:h2 res=210.Joystick Settings
:p.
The joystick calibration information will be reset by selecting the recalibrate button.
Once back in MasterGear/2 the joystick will need to be moved to all 4 corners to complete
the recalibration process.
:h2 res=211.Audio Settings
:p.
:hp4.Volume:ehp4. controls the overall sound level.  By selecting :hp1.off:ehp1.
the sound support routines are disabled.  This may be desirable on slower systems.
:p.
:hp4.Frequency:ehp4. controls the playback frequency used by MasterGear/2.  The higher
the frequency the better the sound and the more CPU time required by the emulator.
If you have a slower system, you may choose a lower frequency instead of turning off
sound support.
:p.
:hp4.Buffer Size:ehp4. sets the size of the audio buffer.  If it's set too small
there will be noticable distortion in the sound.  If it's too large then rapidly
changing sound effects may not be heard.
:p.
:hp4.Oscilloscope:ehp4. determines the initial status of the oscilloscope view.  Check this box if you wish the Oscilloscope to be displayed.
:h2 res=212.Video Settings
:p.
:hp4.System:ehp4. sets MasterGear/2 for NTSC or PAL frame rates.  NTSC runs at 60fps
while PAL runs at 50fps.
:p.
:hp4.Frames Per Second:ehp4. sets the inital frame rate as well as the minimum and maximum
frame rates that MasterGear will be allowed to shoot for.  It may take a few seconds for
the emulation to settle out after changing these settings.
:p.
:hp4.Initial Window Size:ehp4. allows you to select the size of MasterGear's window. The :hp4.MS:ehp4. value sets the
screen size for Master System games while the :hp4.GG:ehp4. value sets the size for Game Gear games.


.*
.**********************************************************
.*
:h1 res=102.Online Game Documentation
:p.
Support for online documention is part of the OS/2 version of MasterGear.  For this
feature to function, there must be a text doc file for the game located in the DOCS
subdirectory of the MG directory.  If the file is not present a warning dialog
will appear.

:p.
The filename must be the same as the cartridge filename
loaded (the .GG or .SMS), with a .DOC extension replacing the original extension.
Doc files are currently displayed using the 'E' editor in OS/2.
They are best viewed using a non-proportional font such as SYSTEM VIO, SYSTEM
MONOSPACED, or COURIER.  Proportional fonts may cause game instructions to
be difficult to read. The directory structure is as follows:
:xmp.
&dot.&dot.&dot.(path)&dot.&dot.&dot.\MG\DOCS\SONIC1.DOC*
               |    |ECHO.DOC*
               |    |PACMANIA.DOC*
               |     etc.
               |
               \ROMS\SONIC1.GG*
               |    |ECHO.GG*
               |    |PACMANIA.SMS*
               |     etc.
               |
               |mg.exe
               |mg.inf
               |joystick.clb
               |etc.
:exmp.

* denotes user installed files
.*
.**********************************************************
.*
:h1.Frame Rate Routines
:p.
The frame rate routines work by establishing a target frame rate, dependent
upon the performance of your hardware.  Your CPU speed, Video Hardware performance, and the use
of sound support all directly effect the performance of MasterGear/2.
:p.
Instead of the traditional show 1 skip n,
the routines in the OS/2 version of MasterGear are :hp1.how many frames to skip out of 16.:ehp1.
This gives a more even spread in the target frame rates.  For instance, under the traditional method
the target frame rates for NTSC are 60, 30, 20, 15, 12, 10, 9, 8, 7, and 6fps.  The
new target frame rates are for NTSC are 60, 56, 53, 49, 45, 41, 38, 34, 30, 26, 23, 19, 15,
11, 8, and 4fps.
Note the big difference in the values between 60 and 30 for each method, the better spread of rates helps performance on faster machines.
For PAL the new target rates are 50, 47, 44, 41, 38, 34, 31, 28, 25, 22, 19, 16, 13, 9, 6, and 3.
:p.
The auto-adjustment mode of the frame rate routines are on by default.
When in auto-adjust mode, MasterGear/2 will analyze the performance of your system
once per second, and adjust the target frame rate accordingly.  By using the
<-/_> and <+/=> keys, you can set the minimum target skip rate.
:p.
By pressing <A> you can toggle the auto adjust mode on or off.  When off, use the
<-/_> and <+/=> keys to manually set your target frame rate.
:p.
Faster machines may exceed 60 (or 50) fps.  If this happens, MasterGear/2 will active throttle routines so that
the game play is not too fast.
:note.If the emulation achieves its target frame rate, even if it's not 60 (or 50) fps, then
MasterGear/2 is running at 100% the speed of a real Sega game system.
.*
.**********************************************************
.*
:h1 res=101.Status Box
:p.
The Status Box shows the performance information of MasterGear/2.
The performance information shows the current frame rate, target rate, minimum
target rate, % speed of a real Sega system, and window resolution.
:p.
For example, if the following is shown:
:xmp.
29fps 30tA 4m 98% 160 x 144
:exmp.
Then MasterGear/2 is currently achieving 29 frames per second, with a target of
30 fps, Auto adjust is turned on(the A following the 30t), a minimum target
rate of 4fps, running at 98% the speed of a real Game Gear, with a window resolution
of 160 x 144.
:p.
If the throttle routines are active, the Status Box text will change slightly.
The text line will show current frame rate, throttle value, % speed of a real
Sega, and window resolution.
:p.
Throttle active example:
:xmp.
60fps 2TA 100% 256 x 192
:exmp.
This shows that MasterGear/2 is currently achieving 60 frames per second, with a
throttle rate of 2, Auto adjust is turned on(the A following the 2T)running at
100% the speed of a real Master System, with a window resolution of 256 x 192.
.*
.**********************************************************
.*
:h1.Controllers
:p.
Since you cannot directly plug in Sega style controllers to a
PC, different ways of obtaining values for the Joystick
are implemented in MasterGear/2.
:p.
PC style joysticks can emulate the Sega joysticks.
:p.
The keyboard can emulate the Joysticks as well as provide control of MasterGear/2.
:note.Research is being conducted on implementing Light Gun support through
the use of the mouse.  If you have any technical information on the Sega Light
Gun please e-mail me at :hp4.spice@ibm.net:ehp4..
.**********************************************************
.*
:h2.Joysticks
:p.
Joystick support requires the joystick divers to be loaded.  If your system
does not already have the drivers installed, unzip the joystick.zip archive
included with MasterGear/2, and run install.cmd.  Your system will need to be rebooted
after installing the drivers for them to work.
:p.
The joysticks will calibrate themselves as the games are played.  MasterGear/2
uses the SpiceWare Joystick Routines(freely available on the SpiceWare homepage at
:hp4.http&colon.//www.geocities.com/~spiceware:ehp4.
for those OS/2 programmers
out there!), and will save the calibration information in the file JOYSTICK.CLB
so it can be reloaded.  If you ever change joysticks, or wish to recalibrate
the ones you have, just delete this file.
:note.By using the keyboard controls, MasterGear/2 can be used without joysticks.
.*
.**********************************************************
.*
.*:h2.Mouse
.*:p.
.*By default, the mouse is used to emulate light gun 1.  Due to the way that
.*the mouse is used under OS/2, the mouse buttons cannot emulate the light gun's trigger.
.*Instead, use the :hp4. &dot. >:ehp4. key as the light gun's trigger.
.*
.*See
.*:link reftype=hd viewport dependent res=103.
.*Run-Time Parameters
.*:elink.
.*if you do not wish to use the mouse as the first light gun.
.*
.**********************************************************
.*
:h2.Keyboard
:p. The keyboard is mapped to emulate the switches and controllers of the Master System/Game Gear.
:xmp.
CNTL-1  Set Screen To 256 x 192 (Master System) or 160 x 144 (Game Gear)
CNTL-2 thru CNTL-9 - multiply the base window size by the number pressed
(the window may also be scaled to any size,
 just grab &amp. move the window borders)

ENTER   START button (only in GameGear mode)
S       START button (only in GameGear mode)
TAB     RESET button (only in MasterSystem mode)
BACKSPC PAUSE button (only in MasterSystem mode)

<ESC>   Quit MasterGear/2
N       Load a new game (or drag and drop ROM images onto window)
P       Pause toggle
O       Display/Hide Oscilloscope


FRAME SKIP ADJUSTMENT:
A       Toggle Auto Frame Rate Adjustment option on/off
- _     Either 1) decrease miminum target frame rate or
               2) decrease target rate
= +     Either 1) increase minimum target frame rate or
               2) increase target rate
        option 1 happens if the auto frame rate adjustment option is on


JOYSTICKS&colon.
Cursor Pad - Player 1's Joystick
&dot. > #   Player 1's Fire Button 1
/ ? &ae.   Player 1's Fire Button 2


T       Player 2's Joystick UP
F       Player 2's Joystick Left
G       Player 2's Joystick Down
H       Player 2's Joystick Right
Z       Player 2's Fire Button 1
X       Player 2's Fire Button 2
:exmp.

.*
.**********************************************************
.*
:h1.Performance Tips
:p.
There are many things to do which can improve the performance of MasterGear.
:ul.
:li.Use a lower sound frequency.  Setting the frequency to 8000 or 11025 will use less CPU.  Or ...
:li.Turn off sound.  There is a performance hit for using sound.
:li.Use a lower color depth.  If you are running in 24bit color, reduce your color depth to 16bit.  A 256 color screen is even faster,
however there is currently a problem with color-cycling games (such as the Sonic series) that causes the 256 color screen to run
real slow when the colors are being cycled.
:li.Do not display the oscilloscope.  If the oscilloscope is turned off then there are fewer pixels to update on the screen.
:li.Use window size 1.
:li.Purchase a better video card.  My father updated his video card to a Matrox Mystique and went from 8fps to 32fps.
:li.Purchase a faster CPU.  If you already have a good video card, then perhaps your CPU isn't up to the task.
:eul.
.*
.**********************************************************
.*
:h1.Acknowledgements
:p.
The following people have had a part in bringing MasterGear to you:
:table cols='17 50'.
:row.
:c.Marat Fayzullin
:c.Maintained the core code, and the DOS and Unix versions
:row.
:c.Tim Fisken
:c.SVGALib drivers for Linux
:row.
:c.Gareth Moore
:c.Maintained the Acorn version
:row.
:c.Darrell Spice Jr.
:c.Maintained the OS/2 version
:row.
:c.Ian Spielman
:c.Improved Unix/X drivers
:row.
:c.John Stiles
:c.Maintained the MacIntosh version
:row.
:c.Amit J. Patel
:c.Author of SimBlob, for the black-bitmap bug workaround.
:etable.
.*
.**********************************************************
.*
:h1.MasterGear/2 Version History
:p.
:ul.

:li.:hp4.Version 1.0.5 released August 9, 1998:ehp4.
:ul.
:li.Performance improvements.
:li.Workaround for black-bitmap problem which showed up in FixPack 5.
:li.MasterGear now begins in ROMS directory, if present.
:li.Improved throttle routine for faster machines.
:li.Fixed window resize problem, occured if only Master System games had been played,
the default window size was bigger than 1, and the oscilliscope was turned on using the O key.
:li.Source code now available, to hopefully drive new emulator ports &colon.-)
:eul.
:li.:hp4.Version 1.0.4 released January 10, 1998:ehp4.
:ul.
:li.Added control panel and removed run-time parameters.
:li.Separate screen size multipliers for Master System and Game Gear.
:li.Control panel setting for oscilloscope active or not when MasterGear is first ran.
:li.New Status Box, no longer using title bar for status information.
:li.Sound is now turned off if a warning dialog box is displayed.
:li.A max FPS can now be set.
:eul.
:li.:hp4.Version 1.0.3 released August 10, 1997:ehp4.
:ol.
:li.The pause key (backspace) for Master System Games now works.
:li.New run-time parameter -t for controlling the TV rate (50 or 60 fps).
:li.New run-time parameter -v for master volume control.
:li.MasterGear/2 Help is now in the OS/2 standard format.
:li.Frame skipping routines have been modified to better support "flickering sprites"
such as when Sonic The Hedgehog has a shield.
:li.Additional keys mapped for joystick 1 to help with German keyboards.
:li.MasterGear/2 Help and Game Documentation support now work correctly when the path to the file contains a space.
:eol.
:li.:hp4.Version 1.0.2 released May 11, 1997:ehp4.
:ol.
:li.Version Numbering scheme changed per Marat's request.
:li.Optimized sound routines
:li.Fixed Sound channel 4 to be NOISE instead of Melodic (explosions and such sound
better than before)
:li.Improved the oscilloscope routines.
:li.Audio routines are no longer consuming CPU time when disabled by using the -f 0
or -b 0 run-time parameter.
:eol.
:li.:hp4.Version 1.1 released May 4, 1997:ehp4. (should really have been ver 1.0.1)
:ol.
:li.Added sound support.
:li.Added an oscilloscope for sound routines.
:li.Fixed drag &amp. drop of Game Gear games, routines assumed a 3 character extension
and GG is a 2 character extension.
:li.Fixed the joystick calibration load routine to work when only 1 joystick is
physically plugged in.
:li.Fixed joystick routines to return a "centered" value for joysticks which are
not plugged in.
:li.Fixed the screen-redraw of a non-active game window.  Used to leave fragments
of other windows on the game screen.
:li.Modified the menu bar, at some screen resolutions it was going to 2 rows for
the size 1 Game Gear window .
:li.Fixed the color problem, games should look correct now.
:li.Fixed the reversed mapping of fire 1 and 2.  Games like Sonic Spinball are
easier to play when the left fire button controls the left flipper &colon.-)
:li.Fixed black strip along left edge in scrolling games (Master System games only) to be truly the color black.
:eol.
:li.:hp4.Version 1.0 released April 20, 1997:ehp4.
:ol.
:li.initial release of MasterGear for OS/2
:eol.
:eul.
.*
.**********************************************************
.*
:h1.MasterGear/2 To Do list
:p.
The To Do's are not listed in any order.
:ul.
:li.Fix color change slow-up for 256 color screens.  Currently the problem is only
known to exist on Matrox Millenium. Please e-mail me if this problem occurs
on your video card.
:li.Add support for Full Screen DIVE.
:li.Add light gun support.
:eul.
:euserdoc.
