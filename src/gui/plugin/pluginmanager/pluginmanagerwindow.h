/*
 ****************************************************************************
 * pluginmanagerwindow.h
 *
 * Created on: 2010-5-10
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
 * Last Modified on: 2010-5-20
 * Last Modified by: 贺辉
 ***************************************************************************
 */

#ifndef PLUGINMANAGERWINDOW_H
#define PLUGINMANAGERWINDOW_H

#include <QWidget>
//#include <QDataWidgetMapper>


#include "plugininfomodel.h"

#include "../../guilib.h"

namespace Ui {
class PluginManagerWindowUI;
}

namespace HEHUI {


class GUI_LIB_API PluginManagerWindow: public QWidget {
    Q_OBJECT

public:
    PluginManagerWindow(QWidget *parent = 0);
    ~PluginManagerWindow();

public slots:
    void on_pushButtonDetails_clicked(bool checked);
    void on_pushButtonLoad_clicked();
    void on_pushButtonUnload_clicked();

private slots:
    void slotUpdateUI(const QModelIndex &index);

protected:
    //	bool eventFilter(QObject *object, QEvent *event);

private:
    Ui::PluginManagerWindowUI *ui;

    PluginInfoModel *pluginInfoModel;

    //QDataWidgetMapper *dataWidgetMapper;

};

} //namespace HEHUI

#endif // PLUGINMANAGERWINDOW_H
