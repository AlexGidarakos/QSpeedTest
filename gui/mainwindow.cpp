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

#include "mainwindow.h"
#include <QtGui/QDesktopWidget>
#include <QtGui/QClipboard>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);
    setWindowTitle(PROGRAMNAME + " " + PROJECTVERSION);
    _ui->progressBar->setVisible(false);

#ifndef Q_WS_WIN
#ifdef Q_WS_MAC
    _ui->plainTextEditResults->setFont(QFont("Courier New", 12));
#else
    _ui->plainTextEditResults->setFont(QFont("Courier New", 9));
#endif // Q_WS_MAC
#endif // Q_WS_WIN

    _centerOnDesktop();
}

void MainWindow::_centerOnDesktop()
{
    quint16 screenWidth, width, screenHeight, height, x, y;
    QSize windowSize;

    screenWidth = QApplication::desktop()->width();
    screenHeight = QApplication::desktop()->height();
    windowSize = size();
    width = windowSize.width();
    height = windowSize.height();
    x = (screenWidth - width) / 2;
    y = (screenHeight - height) / 2;
    y -= 100;
    move(x, y);
}

void MainWindow::on_pushButtonStartStop_clicked()
{
    if(!_ui->pushButtonStartStop->text().compare(trUtf8("Stop")))
    {
        emit pushButtonStopClicked();

        return;
    }

    _ui->pushButtonStartStop->setText(trUtf8("Stop"));
    _ui->pushButtonExit->setEnabled(false);
    _ui->actionSaveBbCode->setEnabled(false);
    _ui->actionSaveHtml->setEnabled(false);
    _ui->actionSavePlainText->setEnabled(false);
    _ui->actionCopyBbCode->setEnabled(false);
    _ui->actionCopyHtml->setEnabled(false);
    _ui->actionCopyPlainText->setEnabled(false);
    _ui->actionPreferences->setEnabled(false);
    _ui->plainTextEditResults->clear();
    slotUpdateLog(trUtf8("Test started"));
    emit pushButtonStartClicked();
}

void MainWindow::on_actionCopyLog_triggered()
{
    QString text;

    for(int i = _ui->comboBoxLog->count() - 1; i >= 0; i--) text += _ui->comboBoxLog->itemText(i) + "\n";

    QApplication::clipboard()->setText(text);
    slotUpdateLog(trUtf8("Log contents copied to system clipboard"));
}

void MainWindow::slotTestFinished(bool uninterrupted)
{
    _ui->pushButtonStartStop->setText(trUtf8("Start"));
    _ui->pushButtonStartStop->setEnabled(true);
    _ui->pushButtonExit->setEnabled(true);
    _ui->actionPreferences->setEnabled(true);

    if(uninterrupted)
    {
        _ui->actionCopyBbCode->setEnabled(true);
        _ui->actionCopyHtml->setEnabled(true);
        _ui->actionCopyPlainText->setEnabled(true);
        _ui->actionSaveBbCode->setEnabled(true);
        _ui->actionSaveHtml->setEnabled(true);
        _ui->actionSavePlainText->setEnabled(true);
        slotResult(trUtf8("\nTest finished"));
        slotUpdateLog(trUtf8("Test finished"));

        return;
    }

    slotUpdateLog(trUtf8("Test aborted"));
    slotResult(trUtf8("\n\nTest aborted"));
}
