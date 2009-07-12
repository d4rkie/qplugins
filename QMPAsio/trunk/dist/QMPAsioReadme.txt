QMPAsio V1.0.7

Contents
========

QMPAsio.dll		Plugin - copy to QMP plugins directory
QMPAsioReadme.txt	This file

Installation
============

1. Launch the installer: QMPAsio.exe 

2. If no installer then copy the QMPAsio.dll module to the ...\Plugins directory of QMP.

Notes
=====

1. This module requires MS VC++ 8.0 CRT and MFC installation. If you do not have these,
   you can get the installation kit (vcredist_x86.exe) from: 
   
   http://www.microsoft.com/downloads/details.aspx?familyid=200B2FD9-AE1A-4A14-984D-389C36F85647

2. Player volume control enable is optional (default is enabled)

3. No re-sampling support. I didn't think this appropriate for performance and sound
   quality reasons. If you are "lucky" enough (as I am) to have Creative sound card which does
   not support 44.1Khz audio, you can use ASIO4ALL (http://www.asio4all.com) to
   re-sample 44.1Khz to 48Khz.

4. If you are experiencing underruns, try increasing the playback buffer size on the configuration
   page. The default is 2048 samples.

5. Currently there is no support for 64-bit data or 64-bit sound cards.

Revision History
================

1.0.7	Improved buffer synchronization with ASIO driver (no deadlocks)
	Fixed track positioning

1.0.6	Faster float-to-integer conversion

1.0.5	Fix problem skipping tracks across 2 CD drives (seamless playback)
	Force mono source to binaural output
	Fix crash when switching ASIO devices
	Added option to enable player volume control

1.0.4	Initial release

Reporting problems
==================

Best place to log a problem: http://www.quinnware.com/forum

ASIO driver/sound card problems can be more easily diagnosed if you get a copy of asiodump 
(http://roed.republika.pl/asiodump/index.html) and execute the command-line:

asiodump -l 3 > dump.txt

and attach the dump.txt file along with your report.





