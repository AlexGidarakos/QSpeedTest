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


#include "dialogabout.h"
#include "ui_dialogabout.h"
#include "externs.h"
#include <QUrl>
#include <QDesktopServices>


DialogAbout::DialogAbout(QWidget *parent) : QDialog(parent), ui(new Ui::DialogAbout)
{
    ui->setupUi(this);
    setWindowTitle(trUtf8("About %1").arg(PROGRAMNAME));
    ui->labelVersion->setText(trUtf8("Version number: ") + PROGRAMVERSION);
    ui->labelAuthor->setText(trUtf8("Author: ") + PROGRAMAUTHOR);
    ui->labelHomepage->setText("<a href=\"" + PROGRAMURL + "\">" + trUtf8("Visit homepage") + "</a>");
    ui->labelForum->setText("<a href=\"" + PROGRAMDISCUSSURL + "\">" + trUtf8("Visit official forum") + "</a>");
}


DialogAbout::~DialogAbout()
{
    delete ui;
}


void DialogAbout::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);

    switch(e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;

        default:
            break;
    }
}


void DialogAbout::on_pushButtonOK_clicked()
{
    close();
}


void DialogAbout::on_labelHomepage_linkActivated(QString link)
{
    QUrl url(link);

    QDesktopServices::openUrl(url);
}


void DialogAbout::on_labelForum_linkActivated(QString link)
{
    QUrl url(link);

    QDesktopServices::openUrl(url);
}


void DialogAbout::on_pushButtonAboutQt_clicked()
{
    qApp->aboutQt();
}
