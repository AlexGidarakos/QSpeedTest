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


#ifndef PINGTARGET_H
#define PINGTARGET_H


#include "libspeedtest_global.h"
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>


const quint8 PINGTIMEOUT = 1;
extern QMutex MUTEX;
extern bool STOPBENCHMARK;


class LIBSPEEDTEST_EXPORT PingTarget : public QObject {
    Q_OBJECT

public:
    PingTarget();
    PingTarget(const PingTarget&, QObject *parent = 0);
    PingTarget& operator=(const PingTarget&);
    void reset();
    QString getName() const { return name; }
    void setName(QString value) { name = value; }
    QString getAddress() const { return address; }
    void setAddress(QString value) { address = value; }
    void addRtt(double);
    double getRttAvg() const;
    QString getRttAvgAsString() const;
    double getPacketLoss() const { return packetLoss; }
    QString getPacketLossAsString() const;
    double getJitter() const;
    QString getJitterAsString() const;
    QString getRank() const;
    bool isAlive() const { return replies; }
    void ping();

private:
    QString name;
    QString address;
    QList<double> rtt;
    double rttSum;
    int replies;
    double packetLoss;
    double jitterSum;
    QString rank;

signals:
    void newTestResult(QString);
};


#endif // PINGTARGET_H
