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
#endif // Q_WS_MAC
#endif // Q_WS_WIN
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


void MainWindow::showClipboardConfirmation(QString value)
{
    ui->labelCopyToClipboard->setText(value);
    QTimer::singleShot(4 * 1000, ui->labelCopyToClipboard, SLOT(clear()));
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


void MainWindow::enablePushButtonStartStop()
{
    ui->pushButtonStartStop->setEnabled(true);
}


void MainWindow::updateLogMessages(QString message)
{
    ui->plainTextEditLogMessages->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ") + message);
}


void MainWindow::updateTestResults(QString message)
{
    ui->plainTextEditTestResults->appendPlainText(message);
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


void MainWindow::on_pushButtonStartStop_clicked()
{
    if(!ui->pushButtonStartStop->text().compare(trUtf8("Start")))
    {
        ui->pushButtonStartStop->setText(trUtf8("Stop"));
        ui->pushButtonExit->setEnabled(false);
        ui->pushButtonCopyVbCode->setEnabled(false);
        ui->pushButtonCopyHtmlCode->setEnabled(false);
        ui->comboBoxTestMode->setEnabled(false);
        ui->spinBoxParallelThreads->setEnabled(false);
        ui->spinBoxPingsPerTarget->setEnabled(false);
        ui->plainTextEditTestResults->clear();
        STOPBENCHMARK = false;
        emit pushButtonStartClicked();
    }
    else
    {
        MUTEX.lock();
        STOPBENCHMARK = true;
        MUTEX.unlock();
        emit pushButtonStopClicked();
    }
}


void MainWindow::updateButtons(bool testAborted)
{
    ui->pushButtonStartStop->setText(trUtf8("Start"));
    ui->pushButtonStartStop->setEnabled(true);
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
        ui->pushButtonCopyVbCode->setEnabled(true);
        ui->pushButtonCopyHtmlCode->setEnabled(true);
    }
}


void MainWindow::on_pushButtonCopyVbCode_clicked()
{
    emit pushButtonCopyVbCodeClicked();
}


void MainWindow::on_pushButtonCopyHtmlCode_clicked()
{
    emit pushButtonCopyHtmlCodeClicked();
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
