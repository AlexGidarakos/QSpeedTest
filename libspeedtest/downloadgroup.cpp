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

#include "downloadgroup.h"

DownloadGroup & DownloadGroup::operator=(const DownloadGroup &group)
{
    if(this != &group)
    {
        _name = group._name;
        _speedBpsParallel = group._speedBpsParallel;
        _hosts = group._hosts;
        _file = NULL;
    }

    return *this;
}

void DownloadGroup::reset()
{
    quint16 size = _hosts.size();

    for(int i = 0; i < size; i++)
    {
        _hosts[i].reset();
    }

    _speedBpsParallel = 0;
}

void DownloadGroup::start()
{
    quint16 size = _hosts.size();

    connect(&_progressTimer, SIGNAL(timeout()), this, SLOT(_slotNewProgress()));
    connect(&_stopTimer, SIGNAL(timeout()), this, SLOT(_slotParallelDownloadFinished()));
    _stopTimer.setSingleShot(true);
    _file = new QTemporaryFile();
    _file->open();

    emit result(trUtf8("Starting parallel download of group %1, please wait approx. %2 sec:").arg(name()).arg(DOWNLOADTESTSECS));

    for(int i = 0; i < size; i++)    // Start downloading
    {
        emit result(_hosts[i].url());
        QNetworkReply *reply = NULL;
        _data.append(reply);
        _data[i] = _manager.get(QNetworkRequest(QUrl(_hosts[i].url())));
        connect(_data[i], SIGNAL(readyRead()), this, SLOT(_slotWriteData()));
    }

    _progress = 0;
    emit progressRangeChanged(0, DOWNLOADTESTSECS * 1000);
    emit progressValueChanged(0);
    _progressTimer.start(200);
    _stopTimer.start(DOWNLOADTESTSECS * 1000);
}

void DownloadGroup::cancel()
{
    _progressTimer.stop();
    _stopTimer.stop();
    disconnect(&_progressTimer, SIGNAL(timeout()), this, SLOT(_slotNewProgress()));
    disconnect(&_stopTimer, SIGNAL(timeout()), this, SLOT(_slotParallelDownloadFinished()));
    disconnect(&_stopTimer, SIGNAL(timeout()), this, SLOT(_slotNextHost()));

    for(int i = 0; i < _data.size(); i++)
    {
        if(_data[i])
        {
            disconnect(_data[i], SIGNAL(readyRead()), this, SLOT(_slotWriteData()));
            _data[i]->abort();
            delete _data[i];
            _data[i] = NULL;
        }
    }

    _data.clear();

    if(_file)
    {
        _file->close();
        _file->remove();
        delete _file;
        _file = NULL;
    }
}

double DownloadGroup::speedParallel(SpeedUnit::Unit unit) const
{
    switch(unit)
    {
    case SpeedUnit::bps:
        return (_speedBpsParallel * 8.0);
    case SpeedUnit::Bps:
        return _speedBpsParallel;
    case SpeedUnit::Kbps:
        return (_speedBpsParallel / 128.0);
    case SpeedUnit::KBps:
        return (_speedBpsParallel / 1024.0);
    case SpeedUnit::Mbps:
        return (_speedBpsParallel / 131072.0);
    case SpeedUnit::MBps:
        return (_speedBpsParallel / 1048576.0);
    default:
        return 0.0;
    }
}

double DownloadGroup::speedSerial(SpeedUnit::Unit unit) const
{
    quint16 size = _hosts.size();
    double sum = 0.0, average = 0.0;

    if(!size)
    {
        return 0.0;
    }

    for(int i = 0; i < size; i++)
    {
        sum += _hosts[i].speed(SpeedUnit::Bps);
    }

    average = sum / size;

    switch(unit)
    {
    case SpeedUnit::bps:
        return (average * 8.0);
    case SpeedUnit::Bps:
        return average;
    case SpeedUnit::Kbps:
        return (average / 128.0);
    case SpeedUnit::KBps:
        return (average / 1024.0);
    case SpeedUnit::Mbps:
        return (average / 131072.0);
    case SpeedUnit::MBps:
        return (average / 1048576.0);
    default:
        return 0.0;
    }
}

void DownloadGroup::_slotParallelDownloadFinished()
{
    for(int i = 0; i < _data.size(); i++)
    {
        if(_data[i])
        {
            disconnect(_data[i], SIGNAL(readyRead()), this, SLOT(_slotWriteData()));
        }
    }

    // Two separate loops because we want to stop recording excess bytes as soon as possible!

    for(int i = 0; i < _data.size(); i++)
    {
        if(_data[i])
        {
            _data[i]->abort();
            delete _data[i];
            _data[i] = NULL;
        }
    }

    // Here we do NOT empty the _data QList because we shall use it for the serial download test

    _progressTimer.stop();
    _stopTimer.stop();    // It has been set up with SingleShot property on, but just to be sure...
    disconnect(&_stopTimer, SIGNAL(timeout()), this, SLOT(_slotParallelDownloadFinished()));
    _file->close();
    setBytesDownloaded(_file->size());
    _file->remove();
    delete _file;
    _file = NULL;
    emit result(trUtf8("Parallel speed: %1 Mbps").arg(QString::number(speedParallel(SpeedUnit::Mbps), 'f', 2)) + '\n');

    // Prepare some things for the serial download test which is about to start
    _hostNext = 0;
    connect(&_stopTimer, SIGNAL(timeout()), this, SLOT(_slotNextHost()));
    emit result(trUtf8("Starting serial downloads from group %1").arg(name()));
    _slotNextHost();
}

void DownloadGroup::_slotNextHost()
{
    if(_hostNext)    // Means it's not the first call of this slot, stop the running download, record the results and cleanup
    {
        _progressTimer.stop();
        _stopTimer.stop();    // It has been set up with SingleShot property on, but just to be sure...
        disconnect(_data[_hostNext - 1], SIGNAL(readyRead()), this, SLOT(_slotWriteData()));
        _data[_hostNext - 1]->abort();
        delete _data[_hostNext - 1];
        _data[_hostNext - 1] = NULL;
        _file->close();
        _hosts[_hostNext - 1].setBytesDownloaded(_file->size());
        _file->remove();
        delete _file;
        _file = NULL;
        emit result(trUtf8("%1 speed: %2 Mbps").arg(_hosts[_hostNext - 1].name()).arg(QString::number(_hosts[_hostNext - 1].speed(SpeedUnit::Mbps), 'f', 2)));
    }

    if(_hostNext == _hosts.size())    // Means we are done with all the hosts and we should cleanup and exit
    {
        emit result("");
        cancel();
        emit finished();

        return;
    }

    _file = new QTemporaryFile();
    _file->open();
    _hostNext++;
    emit result(trUtf8("Downloading from %1, please wait approx. %2 sec").arg(_hosts[_hostNext - 1].name()).arg(DOWNLOADTESTSECS));
    _data[_hostNext - 1] = _manager.get(QNetworkRequest(QUrl(_hosts[_hostNext - 1].url())));
    connect(_data[_hostNext - 1], SIGNAL(readyRead()), this, SLOT(_slotWriteData()));
    _progress = 0;
    emit progressValueChanged(0);
    _progressTimer.start(200);
    _stopTimer.start(DOWNLOADTESTSECS * 1000);
}
