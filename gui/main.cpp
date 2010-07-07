/*
Copyright 2010 Aleksandros Gidarakos


This file is part of QSpeedTest.

QSpeedTest is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QSpeedTest is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QSpeedTest.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <QtCore/QString>
#include "qspeedtest.h"


const QString PROJECTNAME = "QSpeedTest";
const QString PROGRAMNAME = "QSpeedTest";
const QString PROGRAMAUTHOR = "parsifal";
const QString PROGRAMVERSION = "r38";
const QString PROGRAMURL = "https://sourceforge.net/projects/qspeedtest/files/";
const QString PROGRAMDISCUSSURL = "https://sourceforge.net/apps/phpbb/qspeedtest";
const QString PROGRAMUPDATECHECKURL = "http://qspeedtest.sourceforge.net/updates/qspeedtest.version";


int main(int argc, char *argv[]) {
    QSpeedTest qSpeedTest(argc, argv);

    return qSpeedTest.exec();
}
