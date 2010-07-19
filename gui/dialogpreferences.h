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

#ifndef DIALOGPREFERENCES_H
#define DIALOGPREFERENCES_H
#include "ui_dialogpreferences.h"
#include <QtGui/QDialog>

namespace Ui
{
    class DialogPreferences;
}

class DialogPreferences : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPreferences(QWidget *parent = 0) : QDialog(parent), _ui(new Ui::DialogPreferences) { _ui->setupUi(this); }
    ~DialogPreferences() { delete _ui; }
    void myShow();

private:
    Ui::DialogPreferences *_ui;

signals:
    void hostlistUrlChanged();
    void savePreferences();

private slots:
    void on_comboBoxTestMode_currentIndexChanged(int);
    void on_pushButtonDefaults_clicked();
    void on_pushButtonOk_clicked();
    inline void on_pushButtonCancel_clicked() { close(); }
};
#endif // DIALOGPREFERENCES_H
