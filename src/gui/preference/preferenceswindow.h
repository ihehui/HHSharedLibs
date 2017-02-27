/*
 ****************************************************************************
 * preferenceswindow.h
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






#ifndef PREFERENCESWINDOW_H
#define PREFERENCESWINDOW_H

#include <QWidget>



namespace Ui
{
class PreferencesWindowClass;
}

namespace HEHUI
{


class PreferencesWindow : public QWidget
{
    Q_OBJECT

public:
    PreferencesWindow(QWidget *parent = 0);
    ~PreferencesWindow();

private:
    void loadPreferenceRootItems();

private:
    Ui::PreferencesWindowClass *ui;



};

} //namespace HEHUI

#endif // PREFERENCESWINDOW_H
