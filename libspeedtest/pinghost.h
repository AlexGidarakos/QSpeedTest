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

#ifndef PINGHOST_H
#define PINGHOST_H
#include "libspeedtest_global.h"
#include "externs.h"
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

class LIBSPEEDTEST_EXPORT PingHost : public QObject
{
    Q_OBJECT

public:
    explicit PingHost(QObject *parent = 0) : QObject(parent), _packetLoss(100.0) {}
    explicit PingHost(const QString &name, const QString &url, QObject *parent = 0) : QObject(parent), _name(name), _url(url), _packetLoss(100.0) {}
    PingHost(const PingHost &);
    PingHost & operator=(const PingHost &);
    double rtt() const;
    QString rttString() const;
    QString packetLossString() const;
    double jitter() const;
    QString jitterString() const;
    QString rank() const;
    void ping();
    inline bool operator<(const PingHost & host) { return rtt() < host.rtt(); }
    inline void reset() { _rtt.clear(); _packetLoss = 100.0; }
    inline QString name() const { return _name; }
    inline QString url() const { return _url; }
    inline bool isAlive() const { return _rtt.size(); }
    inline double packetLoss() const { return _packetLoss; }

private:
    inline void _addRtt(double rtt) { _rtt.append(rtt); _packetLoss -= (100.0 / PINGSPERHOST); }

    QString _name;
    QString _url;
    QList<double> _rtt;
    double _packetLoss;

signals:
    void result(QString);
};
#endif // PINGHOST_H
