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


# -------------------------------------------------
# Project created by QtCreator 2010-04-23T12:27:28
# -------------------------------------------------
QT += network
TARGET = QSpeedTest
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    dialogabout.cpp \
    qspeedtest.cpp \
    targetlist.cpp \
    targetgroup.cpp
HEADERS += mainwindow.h \
    dialogabout.h \
    qspeedtest.h \
    externs.h \
    targetlist.h \
    targetgroup.h
FORMS += mainwindow.ui \
    dialogabout.ui
OTHER_FILES += TODO \
    README \
    CHANGELOG \
    resources/QSpeedTest.ini
RESOURCES += resources/resources.qrc
