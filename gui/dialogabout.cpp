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

#include "dialogabout.h"
#include "externs.h"

DialogAbout::DialogAbout(QWidget *parent) : QDialog(parent), _ui(new Ui::DialogAbout)
{
    _ui->setupUi(this);
    setWindowTitle(trUtf8("About") + ' ' + PROGRAMNAME);
    _ui->labelProgramNameVersion->setText(QString("<p style=\"color: gray;\">%1 %2</p>").arg(PROGRAMNAME).arg(PROJECTVERSION));
    _ui->labelCopyright->setText(QString("&copy; 2010 <a href=\"%1\">%2</a>").arg(PROJECTCOMPANYURL).arg(PROJECTCOMPANY));
    _ui->labelDescription->setText(trUtf8("QSpeedTest is a C++/Qt crossplatform utility for evaluating the performance of your Internet access/ISP, by running automated ping and speed tests. It is also free software, distributed under the GPLv3 license."));
    _ui->labelHomepage->setText(QString("<a href=\"%1\">%2</a>").arg(PROJECTURL).arg(trUtf8("Homepage")));
    _ui->labelDownloadPage->setText(QString("<a href=\"%1\">%2</a>").arg(PROJECTDOWNLOADURL).arg(trUtf8("Download page")));
}
