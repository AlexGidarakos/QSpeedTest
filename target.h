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


#ifndef TARGET_H
#define TARGET_H

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
        void reset();
        QString getName();
        void setName(QString);
        QString getAddress();
        void setAddress(QString);
        void addRtt(double);
        double getRttAvg();
        QString getRttAvgAsString();
        double getPacketLoss();
        QString getPacketLossAsString();
        double getJitter();
        QString getJitterAsString();
        QString getRank();
        bool isAlive();
        void ping();

    private:
        QString name;
        QString address;
        QList<double> rtt;
        double rttSum;
        QString rttAvgAsString;
        int replies;
        double packetLoss;
        QString packetLossAsString;
        double jitterSum;
        QString jitterAsString;
        QString rank;

    signals:
        void newTestResult(QString);
};

#endif // TARGET_H
