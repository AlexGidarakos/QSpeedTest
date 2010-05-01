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


#ifndef TARGETGROUP_H
#define TARGETGROUP_H

#include <QObject>
#include <QString>
#include <QList>


class Target : public QObject
{
    Q_OBJECT

    public:
        Target();
        Target(const Target&);
        Target& operator=(const Target&);
        QString getName();
        void setName(QString);
        QString getAddress();
        void setAddress(QString);
        void addRtt(double);
        double getRttAvg();
        double getPacketLoss();
        double getJitter();
        QString getRank();
        bool wasAlive();
        void reset();
        void ping();

    private:
        QString name;
        QString address;
        QList<double> rtt;
        double rttAvg;
        int replies;
        double packetLoss;
        double jitter;
        QString rank;

    signals:
        void message(QString);
        void changevBulletinCode(QString);
        void changeHTML(QString);
};


class TargetGroup
{
    public:
        TargetGroup();
        double rttGroupSum();
        int countTargetsAlive();
        QString name;
        int size;
        QList<Target> targets;
};

#endif // TARGETGROUP_H
