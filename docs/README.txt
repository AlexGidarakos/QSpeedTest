QSpeedTest is a GUI tool written in C++ and Qt for evaluating the
latency and speed of an Internet or LAN access by pinging and
concurrently downloading test files from a customized list of target
servers. It is currently hosted on SourceForge.net as free software,
published under the GNU General Public License version 3 and is
distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.


CONTENTS
1. Source code availability
2. Requirements for Windows users
3. Build instructions
4. Support and contact info


1. Source code availability
As per the requirements of GPLv3, QSpeedTest's source code is available to anyone via HTTP downloading from the project's page in SourceForge:
    https://sourceforge.net/projects/qspeedtest/files
    http://qspeedtest.svn.sourceforge.net/viewvc/qspeedtest
Should the aforementioned URLs permanently cease to work, you can contact me (see section 4) and ask for the sources. I will do my best to provide you with a copy via electronic mail.


2. Requirements for Windows users
In order to run, this program requires some of the Qt DLLs present in the system, either in the same folder as the program or in a folder included in the PATH variable. For your convenience, a suitable compressed archive with these DLLs is available in 7z and ZIP format here:
    https://sourceforge.net/projects/qspeedtest/files/qspeedtest-win32-required-libraries


3. Build instructions
Provided that you have installed the necessary Qt development libraries and build tools, all you have to do in order to build binaries from the source code is to navigate to a directory containing the source and enter the two following commands:
    qmake
    make
If you feel more comfortable working with an IDE (Integrated Development Environment), I recommend installing the complete Qt SDK by Nokia, which contains all the necessary libraries and build tools, as well as the excellent Qt Creator IDE. You can install the SDK either via your distribution's repositories (if you are using a Linux distro) or from binaries available in Nokia's Qt homepage:
    http://qt.nokia.com
In Qt Creator, you only need to load QSpeedTest's main project file (qspeedtest.pro) and use the command under "Build" menu


4. Support and contact info
For more information about QSpeedTest, visit its homepage:
    http://qspeedtest.sourceforge.net
and the official phpBB forum:
    https://sourceforge.net/apps/phpbb/qspeedtest
You may also contact the author via e-mail at:
    parsifal@binarynotion.net
