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
along with QSpeedTest. If not, see <http://www.gnu.org/licenses/>.
*/

#include "pinghost.h"
#include <QtCore/QList>
#include <QtCore/QProcess>
#include <QTime>

PingHost::PingHost(const PingHost &host) : QObject(NULL)
{
    _name = host._name;
    _url = host._url;
    _rtt = host._rtt;
    _packetLoss = host._packetLoss;
}

PingHost & PingHost::operator=(const PingHost &host)
{
    if(this != &host)
    {
        _name = host._name;
        _url = host._url;
        _rtt = host._rtt;
        _packetLoss = host._packetLoss;
    }

    return *this;
}

double PingHost::rtt() const
{
    quint8 replies = _rtt.size();

    if(!replies) return 0.0;

    double rttSum = 0.0;

    for(int i = 0; i < replies; i++) rttSum += _rtt[i];

    return (rttSum / replies);
}

QString PingHost::rttString() const
{
    if(_rtt.size()) return QString::number(rtt(), 'f', 2) + " msec";

    return "N/A";
}

QString PingHost::packetLossString() const
{
    if(_rtt.size()) return QString::number(_packetLoss, 'f', 2) + "%";

    return "100.00%";
}

double PingHost::jitter() const
{
    quint8 replies = _rtt.size();

    if(replies < 2) return 0.0;

    double sum = 0.0;

    for(int i = 0; i < replies; i++)
    {
        if(i) sum += _rtt[i - 1] - _rtt[i];
    }

    return (sum / replies);
}

QString PingHost::jitterString() const
{
    double value = jitter();

    return ((value >= 0.00)? '+' : ' ') + QString::number(value, 'f', 2) + " msec";
}

QString PingHost::rank() const
{
    if(!_rtt.size()) return "N/A";

    double value = rtt();

    if(value <  30.0) return "A";
    if(value <  75.0) return "B";
    if(value < 125.0) return "C";
    if(value < 200.0) return "D";
    if(value < 250.0) return "E";

    return "F";
}

void PingHost::ping()
{
#ifdef Q_WS_WIN
    QString pingCmd = QString("ping -n 1 -w %1 %2").arg(PINGTIMEOUTSECS * 1000).arg(_url);
    quint8 skipLines = 3;
#else
    quint8 skipLines = 6;
#ifdef Q_OS_LINUX
    QString pingCmd = QString("ping -c 1 -W %1 %2").arg(PINGTIMEOUTSECS).arg(_url);
#else
    QString pingCmd = QString("ping -c 1 -W %1 %2").arg(PINGTIMEOUTSECS * 1000).arg(_url);
#endif // Q_OS_LINUX
#endif // Q_WS_WIN
    QByteArray contents;
    QString newRtt;
    QStringList list;
    QProcess pingProc;

    reset();
    pingProc.setProcessChannelMode(QProcess::MergedChannels);

    for(int i = 0; i < PINGSPERHOST; i++)
    {
        pingProc.start(pingCmd, QIODevice::ReadOnly);
        pingProc.waitForFinished(PINGTIMEOUTSECS * 1000);

        for(int j = 0; j < skipLines; j++) contents = pingProc.readLine().trimmed();

#ifdef Q_WS_WIN
        if(!contents.contains(QString("=").toAscii()))
            contents = pingProc.readLine().trimmed();
#endif // Q_WS_WIN

        if(contents.contains(QString("=").toAscii())) {
            newRtt = contents;
            list = newRtt.split("=");
#ifdef Q_WS_WIN
            newRtt = list[2];
            newRtt.chop(6);
#else
            list = QString(list[1]).split('/');
            newRtt = list[0];
#endif // Q_WS_WIN
            _addRtt(newRtt.toDouble());
        }
    }

    emit result(_name.leftJustified(27, ' ', true) + "    " + rttString().rightJustified(11, ' ', true) + "    " + packetLossString().rightJustified(9, ' ', true) + "    " + jitterString().rightJustified(12, ' ', true) + "    " + rank().rightJustified(3, ' ', true));
}
