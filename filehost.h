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


#ifndef FILEHOST_H
#define FILEHOST_H

#include <QUrl>


class FileHost : public QObject
{
    Q_OBJECT

    public:
        FileHost();
        FileHost(QString, QUrl);
        FileHost(const FileHost&);
        FileHost& operator=(const FileHost&);
        void downloadTest();
        QString name;
        QUrl url;
        qint64 bytesDownloaded;

    private slots:
        void updateBytes(qint64);

    signals:
        void message(QString);
};

#endif // FILEHOST_H