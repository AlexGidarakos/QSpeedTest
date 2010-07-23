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

#include "qspeedtest.h"
#include <QtCore/QTranslator>

int main(int argc, char *argv[])
{
    QSpeedTest app(argc, argv);
//    QTranslator trQSpeedTest;
//    QTranslator trLibspeedtest;

//    trQSpeedTest.load(":/qspeedtest/translations/qspeedtest_el");
//    app.installTranslator(&trQSpeedTest);
//    trLibspeedtest.load(":/libspeedtest/translations/libspeedtest_el");
//    app.installTranslator(&trLibspeedtest);
//    app.init();    // only when MainWindow is dynamically created!

    return app.exec();
}
