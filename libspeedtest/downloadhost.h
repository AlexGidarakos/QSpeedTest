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

#ifndef DOWNLOADHOST_H
#define DOWNLOADHOST_H
#include "libspeedtest_global.h"
#include <QtCore/QString>
#include "externs.h"

class LIBSPEEDTEST_EXPORT DownloadHost
{
public:
    explicit DownloadHost(const QString &name, const QString &url) : _name(name), _url(url), _speedBps(0) {}
    inline void reset() { _speedBps = 0; }
    inline QString name() const { return _name; }
    inline QString url() const { return _url; }
    inline void setBytesDownloaded(qint64 bytes) { _speedBps = (bytes / DOWNLOADTESTSECS); }
    double speed(SpeedUnit::Unit) const;

private:
    QString _name;
    QString _url;
    quint32 _speedBps;
};
#endif // DOWNLOADHOST_H
