/*
 ****************************************************************************
 * preferenceswindow.cpp
 *
 * Created on: 2010-5-17
 *     Author: 贺辉
 *    License: LGPL
 *    Comment:
 *
 *
 *    =============================  Usage  =============================
 *|
 *|
 *    ===================================================================
 *
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 ****************************************************************************
 */

/*
 ***************************************************************************
 * Last Modified on: 2010-5-17
 * Last Modified by: 贺辉
 ***************************************************************************
 */



#include "preferenceswindow.h"
#include "ui_preferenceswindow.h"



namespace HEHUI {

PreferencesWindow::PreferencesWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PreferencesWindowClass)
{
    ui->setupUi(this);
}

PreferencesWindow::~PreferencesWindow()
{
    delete ui;
}





} //namespace HEHUI

