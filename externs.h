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


#ifndef EXTERNS_H
#define EXTERNS_H


#include <QtCore/QString>
#include <QtCore/QMutex>


extern const QString PROJECTNAME;
extern const QString PROGRAMNAME;
extern const QString PROGRAMAUTHOR;
extern const QString PROGRAMVERSION;
extern const QString PROGRAMURL;
extern const QString PROGRAMDISCUSSURL;
extern const QString PROGRAMUPDATECHECKURL;
extern const QString TARGETLISTURL;
extern const QString TARGETLISTUPDATECHECKURL;
extern const int UPDATECHECKTIMEOUT;
extern const int PINGTIMEOUT;
extern const int DOWNLOADTESTSECS;
extern int PINGSPERTARGET;
extern int PARALLELPINGS;
extern QMutex MUTEX;
extern qint64 BYTESDOWNLOADED;
extern bool STOPBENCHMARK;


#endif // EXTERNS_H
