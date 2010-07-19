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

#ifndef HOSTLIST_H
#define HOSTLIST_H
#include "libspeedtest_global.h"
#include "results.h"
#include <QtCore/QSettings>
#include <QtCore/QThread>
#include <QtCore/QFile>

class LIBSPEEDTEST_EXPORT Hostlist : public QSettings
{
    Q_OBJECT

public:
    explicit Hostlist(QSettings::Format format, QSettings::Scope scope, const QString &company, const QString &program, Results &results, QObject *parent = 0) : QSettings(format, scope, company, program, parent), _results(results) { _results._cpuCores = QThread::idealThreadCount(); }
    bool initOk();
    inline QString version() const { return _version; }
    inline QString comment() const { return _comment; }
    inline QString contactUrl() const { return _contactUrl; }

private:
    Results &_results;
    QString _version;
    QString _comment;
    QString _contactUrl;
    inline void _clear() { _version.clear(); _comment.clear(); _contactUrl.clear(); sync(); }
    inline bool _exists() const { return QFile::exists(fileName()); }
    bool _updateIsAvailable();
    bool _downloadOk(const QString &);
    bool _isValid(const QString &);
    bool _restoreEmbeddedOk();
    void _load();

signals:
    void message(QString);
};
#endif // HOSTLIST_H
