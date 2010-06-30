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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtGui/QMainWindow>
#include "dialogabout.h"
#include "externs.h"


namespace Ui
{
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int parallelThreads();
    bool pingTestEnabled();
    bool downloadTestEnabled();
    void showStatusBarMessage(QString);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    void centerOnDesktop();

signals:
    void pushButtonStartClicked();
    void pushButtonStopClicked();
    void pushButtonHtmlCodeClicked(bool);
    void pushButtonVbCodeClicked(bool);

private slots:
    void on_comboBoxTestMode_currentIndexChanged(int index);
    void on_spinBoxPingsPerTarget_valueChanged(int value) { PINGSPERTARGET = value; }
    void enablePushButtonStartStop();
    void updateLogMessages(QString);
    void updateTestResults(QString);
    void benchmarkFinished(bool benchmarkCompleted);
    void on_pushButtonStartStop_clicked();
    void on_pushButtonAbout_clicked();
    void on_pushButtonExit_clicked() { close(); }
    void on_pushButtonHtmlCode_clicked() { emit pushButtonHtmlCodeClicked(true); }
    void on_pushButtonVbCode_clicked() { emit pushButtonVbCodeClicked(false); }
};


#endif // MAINWINDOW_H
