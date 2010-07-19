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

#ifndef DOWNLOADGROUP_H
#define DOWNLOADGROUP_H
#include "libspeedtest_global.h"
#include "downloadhost.h"
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QTemporaryFile>

class LIBSPEEDTEST_EXPORT DownloadGroup : public QObject
{
    Q_OBJECT

    friend class Results;
    friend class Hostlist;

public:
    explicit DownloadGroup(const QString &name, QObject *parent = 0) : QObject(parent), _name(name), _speedBpsParallel(0), _file(NULL) {}
    explicit DownloadGroup(const DownloadGroup &group, QObject *parent = 0) : QObject(parent), _name(group._name), _speedBpsParallel(group._speedBpsParallel), _hosts(group._hosts), _file(NULL) {}
    DownloadGroup & operator=(const DownloadGroup &);
    void reset();
    void start();
    void cancel();
    inline void clear() { _speedBpsParallel = 0; _hosts.clear(); }
    inline QString name() const { return _name; }
    inline void setBytesDownloaded(qint64 bytes) { _speedBpsParallel = (bytes / DOWNLOADTESTSECS); }
    double speedParallel(SpeedUnit::Unit) const;
    double speedSerial(SpeedUnit::Unit) const;

private:
    QString _name;
    quint32 _speedBpsParallel;
    QList<DownloadHost> _hosts;
    int _progress;
    QTimer _progressTimer;
    QTimer _stopTimer;
    QNetworkAccessManager _manager;
    QList<QNetworkReply*> _data;
    QTemporaryFile *_file;
    quint16 _hostNext;

signals:
    void result(QString);
    void progressRangeChanged(int, int);
    void progressValueChanged(int);
    void finished();

private slots:
    inline void _slotNewProgress() { _progress += _progressTimer.interval(); emit progressValueChanged(_progress); }
    inline void _slotWriteData() { QNetworkReply *data = qobject_cast<QNetworkReply*>(sender()); if(data) _file->write(data->readAll()); }
    void _slotParallelDownloadFinished();
    void _slotNextHost();
};
#endif // DOWNLOADGROUP_H
