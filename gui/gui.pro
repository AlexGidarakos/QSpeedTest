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
TARGET = qspeedtest
TEMPLATE = app
DESTDIR = ../bin
INCLUDEPATH += ../libspeedtest
LIBS += -L../bin -L../libspeedtest -lspeedtest
SOURCES += \
    main.cpp \
    qspeedtest.cpp \
    mainwindow.cpp \
    dialogpreferences.cpp \
    dialogabout.cpp
HEADERS += \
    qspeedtest.h \
    mainwindow.h \
    ../libspeedtest/externs.h \
    dialogpreferences.h \
    dialogabout.h
FORMS += mainwindow.ui \
    dialogpreferences.ui \
    dialogabout.ui
OTHER_FILES += \
    logo.png \
    icons.rc \
    icons.ico
RESOURCES += \
    resources.qrc
