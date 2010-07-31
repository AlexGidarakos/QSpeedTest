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

#include "dialogpreferences.h"
#include "externs.h"
#include <QtCore/QUrl>
#include <QtGui/QMessageBox>

void DialogPreferences::myShow()
{
    quint8 number;

    switch(TESTMODE)
    {
        case TestMode::Info:
            number = 0;
            break;
        case TestMode::Ping:
            number = 1;
            break;
        case TestMode::Download:
            number = 2;
            break;
        case TestMode::All:
            number = 3;
            break;
        default:
            number = 3;
    }

    _ui->comboBoxTestMode->setCurrentIndex(number);
    _ui->spinBoxPingsPerHost->setValue(PINGSPERHOST);
    _ui->spinBoxPingThreads->setValue(PINGTHREADS);
    _ui->spinBoxPingTimeout->setValue(PINGTIMEOUTSECS);
    _ui->spinBoxDownloadTestSecs->setValue(DOWNLOADTESTSECS);
    _ui->lineEditHostlistUrl->setText(HOSTLISTURL);
    show();
}

void DialogPreferences::on_pushButtonOk_clicked()
{
    bool changed = false;
    quint8 number;
    QString string;

    string = _ui->lineEditHostlistUrl->text();

    if(!QUrl(string).isValid())
    {
        QMessageBox::critical(NULL, trUtf8("Error"), trUtf8("You have entered an invalid URL."));

        return;
    }

    number = _ui->comboBoxTestMode->currentIndex();

    switch(number)
    {
        case 0:
            number = TestMode::Info;
            break;
        case 1:
            number = TestMode::Ping;
            break;
        case 2:
            number = TestMode::Download;
            break;
        case 3:
            number = TestMode::All;
            break;
        default:
            number = TestMode::All;
    }

    if(number != TESTMODE)
    {
        changed = true;
        TESTMODE = (TestMode::Mode) number;
    }

    number = _ui->spinBoxPingsPerHost->value();

    if(number != PINGSPERHOST)
    {
        changed = true;
        PINGSPERHOST = number;
    }

    number = _ui->spinBoxPingThreads->value();

    if(number != PINGTHREADS)
    {
        changed = true;
        PINGTHREADS = number;
    }

    number = _ui->spinBoxPingTimeout->value();

    if(number != PINGTIMEOUTSECS)
    {
        changed = true;
        PINGTIMEOUTSECS = number;
    }

    number = _ui->spinBoxDownloadTestSecs->value();

    if(number != DOWNLOADTESTSECS)
    {
        changed = true;
        DOWNLOADTESTSECS = number;
    }

    if(string != HOSTLISTURL)
    {
        changed = true;
        HOSTLISTURL = string;
        emit hostlistUrlChanged();
    }

    if(changed)
    {
        emit savePreferences();
    }

    close();
}

void DialogPreferences::on_pushButtonDefaults_clicked()
{
    _ui->comboBoxTestMode->setCurrentIndex(3);
    _ui->spinBoxPingsPerHost->setValue(PINGSPERHOSTDEFAULT);
    _ui->spinBoxPingThreads->setValue(PINGTHREADSDEFAULT);
    _ui->spinBoxPingTimeout->setValue(PINGTIMEOUTSECSDEFAULT);
    _ui->spinBoxDownloadTestSecs->setValue(DOWNLOADTESTSECSDEFAULT);
    _ui->lineEditHostlistUrl->setText(HOSTLISTURLDEFAULT);
}

void DialogPreferences::on_comboBoxTestMode_currentIndexChanged(int index)
{
    switch(index)
    {
        case 0:
            _ui->spinBoxPingsPerHost->setEnabled(false);
            _ui->spinBoxPingThreads->setEnabled(false);
            _ui->spinBoxPingTimeout->setEnabled(false);
            _ui->spinBoxDownloadTestSecs->setEnabled(false);
            break;
        case 1:
            _ui->spinBoxPingsPerHost->setEnabled(true);
            _ui->spinBoxPingThreads->setEnabled(true);
            _ui->spinBoxPingTimeout->setEnabled(true);
            _ui->spinBoxDownloadTestSecs->setEnabled(false);
            break;
        case 2:
            _ui->spinBoxPingsPerHost->setEnabled(false);
            _ui->spinBoxPingThreads->setEnabled(false);
            _ui->spinBoxPingTimeout->setEnabled(false);
            _ui->spinBoxDownloadTestSecs->setEnabled(true);
            break;
        case 3:
            _ui->spinBoxPingsPerHost->setEnabled(true);
            _ui->spinBoxPingThreads->setEnabled(true);
            _ui->spinBoxPingTimeout->setEnabled(true);
            _ui->spinBoxDownloadTestSecs->setEnabled(true);
            break;
        default:
            _ui->spinBoxPingsPerHost->setEnabled(true);
            _ui->spinBoxPingThreads->setEnabled(true);
            _ui->spinBoxPingTimeout->setEnabled(true);
            _ui->spinBoxDownloadTestSecs->setEnabled(true);
    }
}
