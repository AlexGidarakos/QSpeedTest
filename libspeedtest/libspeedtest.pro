# Copyright 2010 Aleksandros Gidarakos
# This file is part of QSpeedTest.
# QSpeedTest is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# QSpeedTest is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with QSpeedTest.  If not, see <http://www.gnu.org/licenses/>.
QT += network
QT -= gui
win32 { 
    COPYCOMMAND = $$_PRO_FILE_PWD_/../docs/*.txt \
        ../bin
    COPYCOMMAND = copy \
        $$replace(COPYCOMMAND, "/", "\\")
    DLLDESTDIR = ../bin
}
else { 
    COPYCOMMAND = $$_PRO_FILE_PWD_/../docs/*.txt \
        ../bin
    COPYCOMMAND = cp \
        $$COPYCOMMAND
    DESTDIR = ../bin
}
TARGET = speedtest
TEMPLATE = lib
DEFINES += LIBSPEEDTEST_LIBRARY
SOURCES += testresults.cpp \
    downloadtarget.cpp \
    pingtarget.cpp \
    pinggroup.cpp \
    targetlist.cpp \
    hostinfo.cpp
HEADERS += libspeedtest_global.h \
    testresults.h \
    downloadtarget.h \
    pingtarget.h \
    pinggroup.h \
    targetlist.h \
    hostinfo.h
RESOURCES += resources/resources.qrc
QMAKE_POST_LINK = $$COPYCOMMAND
OTHER_FILES += resources/default.targets.ini
