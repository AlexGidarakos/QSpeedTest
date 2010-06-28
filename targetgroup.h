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


#include <QtCore/QString>
#include <QtCore/QList>
#include "target.h"
#include "plaintarget.h"


class TargetGroup
{
    public:
        TargetGroup() { size = 0; }
        void reset();
        void addTarget(const Target &target) { targets.append(target); size++; }
        double getRttSum();
        QString getRttSumAsString();
        double getRttAvg();
        QString getRttAvgAsString();
        double getPacketLossAvg();
        QString getPacketLossAvgAsString();
        QString getRank();
        int getTargetsAlive();
        QString getName() { return name; }
        void setName(QString value) { name = value; }
        int getSize() { return size; }
        void setSize(int value) { size = value; }
        QList<Target> targets;
        QList<PlainTarget> plainTargets;

    private:
        double rttSum;
        QString rttSumAsString;
        QString rttAvgAsString;
        double packetLossAvg;
        QString packetLossAvgAsString;
        QString rank;
        int targetsAlive;
        QString name;
        int size;
};


#endif // TARGETGROUP_H
