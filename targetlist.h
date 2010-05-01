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


#ifndef TARGETLIST_H
#define TARGETLIST_H

#include <QObject>
#include <QSettings>
#include <QList>
#include "targetgroup.h"


class TargetList : public QObject
{
    Q_OBJECT

    public:
        explicit TargetList(QObject *parent = 0);
        ~TargetList();
        void purge();
        bool isUpdateAvailable();
        bool downloadList();
        bool load();
        bool restoreEmbedded();
        bool init();
        QString version;
        QString comment;
        QString contactURL;
        int numberOfGroups;
        int numberOfTargets;
        QList<TargetGroup> groups;

    private:
        QSettings *settings;

    signals:
        void message(QString);

    public slots:

};

#endif // TARGETLIST_H
