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

#ifndef PREFERENCES_H
#define PREFERENCES_H
#include "libspeedtest_global.h"
#include <QtCore/QSettings>
#include <QtCore/QFile>

class LIBSPEEDTEST_EXPORT Preferences : public QSettings
{
    Q_OBJECT

public:
    explicit Preferences(QSettings::Format format, QSettings::Scope scope, const QString &company, const QString &program, QObject *parent) : QSettings(format, scope, company, program, parent) {}
    void init();

private:
    inline bool _exists() const { return QFile::exists(fileName()); }
    bool _loadOk();
    bool _restoreEmbeddedOk();

signals:
    void message(QString);
};
#endif // PREFERENCES_H
