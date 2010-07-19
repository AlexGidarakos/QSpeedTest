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
    CONFIG += staticlib
    COPYCOMMAND = $$_PRO_FILE_PWD_/../docs/*.txt \
        ../bin
    COPYCOMMAND = cp \
        $$COPYCOMMAND
    DESTDIR = ../bin
}

TARGET = speedtest
TEMPLATE = lib
DEFINES += LIBSPEEDTEST_LIBRARY
SOURCES += \
    pinggroup.cpp \
    hostinfo.cpp \
    downloadgroup.cpp \
    results.cpp \
    pinghost.cpp \
    hostlist.cpp \
    externs.cpp \
    preferences.cpp \
    testcontroller.cpp \
    downloadhost.cpp
HEADERS += \
    libspeedtest_global.h \
    pinggroup.h \
    hostinfo.h \
    externs.h \
    downloadgroup.h \
    results.h \
    downloadhost.h \
    pinghost.h \
    hostlist.h \
    preferences.h \
    testcontroller.h
OTHER_FILES += \
    default.hostlist.ini \
    default.preferences.ini
RESOURCES += \
    resources.qrc

QMAKE_POST_LINK = $$COPYCOMMAND
