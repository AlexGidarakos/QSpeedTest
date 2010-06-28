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


#ifndef DIALOGABOUT_H
#define DIALOGABOUT_H


#include <QtGui/QDialog>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtGui/QApplication>


namespace Ui
{
    class DialogAbout;
}


class DialogAbout : public QDialog
{
    Q_OBJECT

    public:
        DialogAbout(QWidget *parent = 0);
        ~DialogAbout();

    protected:
        void changeEvent(QEvent *e);

    private:
        Ui::DialogAbout *ui;

    private slots:
        void on_labelHomepage_linkActivated(QString link) { QDesktopServices::openUrl(QUrl(link)); }
        void on_labelForum_linkActivated(QString link) { QDesktopServices::openUrl(QUrl(link)); }
        void on_pushButtonOK_clicked() { close(); }
        void on_pushButtonAboutQt_clicked() { qApp->aboutQt(); }
};


#endif // DIALOGABOUT_H
