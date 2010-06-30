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


#ifndef PLAINTARGET_H
#define PLAINTARGET_H


#include <QtCore/QString>
#include "target.h"


class PlainTarget
{
public:
    PlainTarget() {}
    PlainTarget(const Target&);
    bool operator<(const PlainTarget &plainTarget) const { return (rtt < plainTarget.rtt); }
    QString getName() { return name; }
    QString getAddress() { return address; }
    double getRtt() { return rtt; }
    QString getRttAsString() { return rttAsString; }
    double getPacketLoss() { return packetLoss; }
    QString getPacketLossAsString() { return packetLossAsString; }
    double getJitter() { return jitter; }
    QString getJitterAsString() { return jitterAsString; }
    QString getRank() { return rank; }
    bool isAlive() { return aliveFlag; }

private:
    QString name;
    QString address;
    double rtt;
    QString rttAsString;
    bool aliveFlag;
    double packetLoss;
    QString packetLossAsString;
    double jitter;
    QString jitterAsString;
    QString rank;
};


#endif // PLAINTARGET_H
