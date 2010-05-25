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


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(PROGRAMNAME + " " + PROGRAMVERSION);
    ui->spinBoxPingsPerTarget->setValue(PINGSPERTARGET);
    centerOnDesktop();
}


int MainWindow::parallelThreads()
{
    return ui->spinBoxParallelThreads->value();
}


bool MainWindow::speedTestEnabled()
{
    return ui->checkBoxSpeedTest->isChecked();
}


void MainWindow::pushButtonStopEnable(bool value)
{
    ui->pushButtonStop->setEnabled(value);
}


void MainWindow::centerOnDesktop()
{
    QDesktopWidget *desktop = QApplication::desktop();
    int screenWidth, width, screenHeight, height, x, y;
    QSize windowSize;

    screenWidth = desktop->width();
    screenHeight = desktop->height();
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
    ui->pushButtonStart->setEnabled(false);
    ui->pushButtonStop->setEnabled(true);
    ui->pushButtonExit->setEnabled(false);
    ui->pushButtonCopyvBulletinCode->setEnabled(false);
    ui->pushButtonCopyHTML->setEnabled(false);
    ui->checkBoxSpeedTest->setEnabled(false);
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
    ui->checkBoxSpeedTest->setEnabled(true);
    ui->spinBoxParallelThreads->setEnabled(true);
    ui->spinBoxPingsPerTarget->setEnabled(true);

    if(testAborted)
    {
        this->updateLogMessages(trUtf8("Test aborted"));
        this->updateTestResults(trUtf8("Test aborted"));
    }
    else
    {
        ui->pushButtonCopyvBulletinCode->setEnabled(true);
        ui->pushButtonCopyHTML->setEnabled(true);
    }
}


void MainWindow::on_pushButtonCopyvBulletinCode_clicked()
{
    emit pushButtonCopyvBulletinCodeClicked();
}


void MainWindow::on_pushButtonCopyHTML_clicked()
{
    emit pushButtonCopyHTMLClicked();
}


void MainWindow::on_pushButtonStop_clicked()
{
    STOPBENCHMARK = true;
}


void MainWindow::on_spinBoxPingsPerTarget_valueChanged(int value)
{
    PINGSPERTARGET = value;
}
