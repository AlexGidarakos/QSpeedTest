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

#ifndef DIALOGABOUT_H
#define DIALOGABOUT_H
#include "ui_dialogabout.h"
#include <QtGui/QDialog>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>

namespace Ui
{
    class DialogAbout;
}

class DialogAbout : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAbout(QWidget *parent = 0);
    ~DialogAbout() { delete _ui; }

private:
    Ui::DialogAbout *_ui;

private slots:
    inline void on_labelCopyright_linkActivated(QString link) { QDesktopServices::openUrl(QUrl(link)); }
    inline void on_labelHomepage_linkActivated(QString link) { QDesktopServices::openUrl(QUrl(link)); }
    inline void on_labelDownloadPage_linkActivated(QString link) { QDesktopServices::openUrl(QUrl(link)); }
    inline void on_pushButtonOK_clicked() { close(); }
};
#endif // DIALOGABOUT_H
