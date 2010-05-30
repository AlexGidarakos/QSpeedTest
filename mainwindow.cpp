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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "externs.h"
#include <QDesktopWidget>
#include <QDateTime>
#include <QClipboard>
#include <QTimer>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->splitter->setSizes(QList<int>() << 50 << 200);
    setWindowTitle(PROGRAMNAME + " " + PROGRAMVERSION);
    ui->spinBoxPingsPerTarget->setValue(PINGSPERTARGET);
    centerOnDesktop();
#ifndef Q_WS_WIN
#ifdef Q_WS_MAC
    ui->plainTextEditLogMessages->setFont(QFont("Courier New", 12));
    ui->plainTextEditTestResults->setFont(QFont("Courier New", 12));
#else
    ui->plainTextEditLogMessages->setFont(QFont("Courier New", 9));
    ui->plainTextEditTestResults->setFont(QFont("Courier New", 9));
#endif
#endif
}


int MainWindow::parallelThreads()
{
    return ui->spinBoxParallelThreads->value();
}


bool MainWindow::pingTestEnabled()
{
    return (ui->comboBoxTestMode->currentIndex() != 2);
}


bool MainWindow::downloadTestEnabled()
{
    return (ui->comboBoxTestMode->currentIndex() != 1);
}


void MainWindow::pushButtonStopEnable(bool value)
{
    ui->pushButtonStop->setEnabled(value);
}


void MainWindow::centerOnDesktop()
{
    int screenWidth, width, screenHeight, height, x, y;
    QSize windowSize;

    screenWidth = QApplication::desktop()->width();
    screenHeight = QApplication::desktop()->height();
    windowSize = size();
    width = windowSize.width();
    height = windowSize.height();
    x = (screenWidth - width) / 2;
    y = (screenHeight - height) / 2;
    y -= 50;
    move(x, y);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);

    switch(e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;

        default:
            break;
    }
}


void MainWindow::enablePushButtonStart()
{
    ui->pushButtonStart->setEnabled(true);
}


void MainWindow::updateLogMessages(QString message)
{
    ui->plainTextEditLogMessages->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ") + message);
}


void MainWindow::updateTestResults(QString message)
{
    ui->plainTextEditTestResults->appendPlainText(message);
}


void MainWindow::updateVbCode(QString code)
{
    vbCode += code;
}


void MainWindow::updateHtmlCode(QString code)
{
    htmlCode += code;
}


void MainWindow::on_pushButtonExit_clicked()
{
    close();
}


void MainWindow::on_pushButtonAbout_clicked()
{
    DialogAbout *dialog = new DialogAbout(this);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}


void MainWindow::on_pushButtonStart_clicked()
{
    vbCode.clear();
    htmlCode.clear();
    ui->pushButtonStart->setEnabled(false);
    ui->pushButtonStop->setEnabled(true);
    ui->pushButtonExit->setEnabled(false);
    ui->pushButtonCopyVbCode->setEnabled(false);
    ui->pushButtonCopyHtmlCode->setEnabled(false);
    ui->comboBoxTestMode->setEnabled(false);
    ui->spinBoxParallelThreads->setEnabled(false);
    ui->spinBoxPingsPerTarget->setEnabled(false);
    ui->plainTextEditTestResults->clear();
    emit pushButtonStartClicked();
}


void MainWindow::updateButtons(bool testAborted)
{
    ui->pushButtonStart->setEnabled(true);
    ui->pushButtonStop->setEnabled(false);
    ui->pushButtonExit->setEnabled(true);
    ui->comboBoxTestMode->setEnabled(true);
    ui->spinBoxParallelThreads->setEnabled(true);
    ui->spinBoxPingsPerTarget->setEnabled(true);

    if(testAborted)
    {
        this->updateLogMessages(trUtf8("Test aborted"));
        this->updateTestResults(trUtf8("Test aborted"));
    }
    else
    {
        htmlCode.replace('&', "&amp;");
        htmlCode.replace("&amp;nbsp", "&nbsp");
        ui->pushButtonCopyVbCode->setEnabled(true);
        ui->pushButtonCopyHtmlCode->setEnabled(true);
    }
}


void MainWindow::on_pushButtonCopyVbCode_clicked()
{
    QApplication::clipboard()->setText(vbCode);
    ui->labelCopyToClipboard->setText(trUtf8("<p style=\"align: center; color: green;\">vBulletin code copied to clipboard!</p>"));
    QTimer::singleShot(4 * 1000, ui->labelCopyToClipboard, SLOT(clear()));
}


void MainWindow::on_pushButtonCopyHtmlCode_clicked()
{
    QApplication::clipboard()->setText(htmlCode);
    ui->labelCopyToClipboard->setText(trUtf8("<p style=\"align: center; color: green;\">HTML code copied to clipboard!</p>"));
    QTimer::singleShot(4 * 1000, ui->labelCopyToClipboard, SLOT(clear()));
}


void MainWindow::on_pushButtonStop_clicked()
{
    MUTEX.lock();
    STOPBENCHMARK = true;
    MUTEX.unlock();
}


void MainWindow::on_comboBoxTestMode_currentIndexChanged(int index)
{
    if(index == 2)
    {
        ui->spinBoxPingsPerTarget->setEnabled(false);
        ui->spinBoxParallelThreads->setEnabled(false);
    }
    else
    {
        ui->spinBoxPingsPerTarget->setEnabled(true);
        ui->spinBoxParallelThreads->setEnabled(true);
    }
}


void MainWindow::on_spinBoxPingsPerTarget_valueChanged(int value)
{
    PINGSPERTARGET = value;
}
