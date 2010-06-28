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
TARGET = qspeedtestgui
TEMPLATE = app
INCLUDEPATH = .
SOURCES += \
    mainwindow.cpp \
    dialogabout.cpp \
    qspeedtest.cpp \
    targetlist.cpp \
    targetgroup.cpp \
    filehost.cpp \
    testresults.cpp \
    target.cpp \
    plaintarget.cpp \
    main_qspeedtestgui.cpp
HEADERS += mainwindow.h \
    dialogabout.h \
    qspeedtest.h \
    externs.h \
    targetlist.h \
    targetgroup.h \
    filehost.h \
    testresults.h \
    target.h \
    plaintarget.h
FORMS += mainwindow.ui \
    dialogabout.ui
RESOURCES += resources/resources.qrc
