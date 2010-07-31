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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtGui/QMainWindow>
#include <QtCore/QDateTime>
#include "ui_mainwindow.h"
#include "dialogpreferences.h"
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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow() { delete _ui; }
    inline void showProgressBar() { _ui->progressBar->reset(); _ui->progressBar->setVisible(true); }
    inline void hideProgressBar() { _ui->progressBar->setVisible(false); }

    DialogPreferences d1;
    DialogAbout d2;

private:
    Ui::MainWindow *_ui;
    void _centerOnDesktop();
    void closeEvent(QCloseEvent *);

signals:
    void pushButtonStartClicked();
    void pushButtonStopClicked();
    void copyReport(ReportFormat::Format);
    void saveReport(ReportFormat::Format);
    void exitWhileStillTesting();

public slots:
    inline void slotLog(QString message) { _ui->comboBoxLog->insertItem(0, QDateTime::currentDateTime().toString("hh:mm:ss.zzz: ") + message); _ui->comboBoxLog->setCurrentIndex(0); }
    inline void slotResult(QString result) { _ui->plainTextEditResults->appendPlainText(result); _ui->plainTextEditResults->moveCursor(QTextCursor::NoMove); }
    inline void slotShowProgressBar(int min, int max) { _ui->progressBar->setRange(min, max); _ui->progressBar->reset(); _ui->progressBar->setVisible(true); }
    inline void slotUpdateProgressBar(int value) { _ui->progressBar->setValue(value); }
    inline void slotHideProgressBar() { _ui->progressBar->setVisible(false); }
    void slotTestFinished(bool);

private slots:
    inline void on_pushButtonExit_clicked() { close(); }
    inline void on_actionExit_triggered() { close(); }
    inline void on_actionSavePlainText_triggered() { emit saveReport(ReportFormat::PlainText); }
    inline void on_actionSaveBbCode_triggered() { emit saveReport(ReportFormat::BbCode); }
    inline void on_actionSaveHtml_triggered() { emit saveReport(ReportFormat::Html); }
    inline void on_actionCopyPlainText_triggered() { emit copyReport(ReportFormat::PlainText); }
    inline void on_actionCopyBbCode_triggered() { emit copyReport(ReportFormat::BbCode); }
    inline void on_actionCopyHtml_triggered() { emit copyReport(ReportFormat::Html); }
    inline void on_actionClearLog_triggered() { _ui->comboBoxLog->clear(); slotLog(trUtf8("Log cleared")); }
    inline void on_actionPreferences_triggered() { d1.myShow(); }
    inline void on_actionAboutQSpeedTest_triggered() { d2.show(); }
    inline void on_actionAboutQt_triggered() { qApp->aboutQt(); }
    inline void _slotEnablePushButtonStartStop(bool value) { _ui->pushButtonStartStop->setEnabled(value); }
    inline void _slotSetProgressBarRange(int min, int max) { _ui->progressBar->setRange(min, max); }
    inline void _slotUpdateProgressBar(int value) { _ui->progressBar->setValue(value); }
    void on_actionCopyLog_triggered();
    void on_pushButtonStartStop_clicked();
};
#endif // MAINWINDOW_H

/*
public:
    void showProgressBar();
    void hideProgressBar();
    void updateProgressBar(quint8);
*/
