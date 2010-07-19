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

#ifndef PINGGROUP_H
#define PINGGROUP_H
#include "libspeedtest_global.h"
#include "pinghost.h"
#include <QtCore/QList>

inline bool lessThan(const PingHost &host1, PingHost const &host2) { return host1.rtt() < host2.rtt(); }

class LIBSPEEDTEST_EXPORT PingGroup
{
    friend class Results;
    friend class Hostlist;
    friend class TestController;

public:
    explicit PingGroup(const QString &name) : _name(name) {}
    inline QString name() const { return _name; }
    inline void sort() { qSort(_hosts.begin(), _hosts.end(), lessThan); }
    void reset();
    quint16 hostsAlive() const;
    double rttSum() const;
    QString rttSumString() const;
    double rttAverage() const;
    QString rttAverageString() const;
    double packetLossAverage() const;
    QString packetLossAverageString() const;
    QString rank() const;

private:
    QString _name;
    QList<PingHost> _hosts;
};
#endif // PINGGROUP_H
