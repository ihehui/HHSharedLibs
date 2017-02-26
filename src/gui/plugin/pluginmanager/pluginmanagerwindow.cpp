/*
 ****************************************************************************
 * pluginmanagerwindow.cpp
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


#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QKeyEvent>



#include "../../../core/plugin/pluginmanager.h"

#include "pluginmanagerwindow.h"
#include "ui_pluginmanagerwindow.h"



namespace HEHUI
{


PluginManagerWindow::PluginManagerWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PluginManagerWindowUI)
{
    ui->setupUi(this);

    ui->groupBoxDetails->hide();

    pluginInfoModel = new PluginInfoModel();
    pluginInfoModel->setPluginsHash(PluginManager::instance()->getPluginsHash());

    ui->tableView->setModel(pluginInfoModel);

    //    dataWidgetMapper = new QDataWidgetMapper(this);
    //    dataWidgetMapper->setOrientation(Qt::Horizontal);
    //    dataWidgetMapper->setModel(pluginInfoModel);
    //    dataWidgetMapper->addMapping(ui.labelAuthor, 5, "text");
    //    dataWidgetMapper->addMapping(ui.labelURL, 6, "text");
    //    dataWidgetMapper->addMapping(ui.textBrowserDescription, 7);
    //    dataWidgetMapper->addMapping(ui.textBrowserLicense, 8, "html");
    //    dataWidgetMapper->toFirst();

    //    connect(ui.tableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
    //            dataWidgetMapper, SLOT(setCurrentModelIndex(const QModelIndex &)));
    //    connect(ui.tableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slotShowPluginDetails(const QModelIndex &)));
    //dataWidgetMapper->toNext();

    connect(ui->tableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(slotUpdateUI(const QModelIndex &)));

    ui->tableView->selectRow(0);

}

PluginManagerWindow::~PluginManagerWindow()
{

    pluginInfoModel->clear();
    delete pluginInfoModel;
    pluginInfoModel = 0;

    delete ui;
}

/*
bool PluginManagerWindow::eventFilter(QObject *object, QEvent *event){
    if((object == ui.tableView) && (event->type() == QEvent::KeyRelease)){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *> (event);
        if((keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down)){
                        slotShowPluginDetails(ui.tableView->currentIndex());
                        return true;
        }

        }

    return QObject::eventFilter(object, event);

}
*/

void PluginManagerWindow::on_pushButtonDetails_clicked(bool checked)
{
    if (checked) {
        ui->groupBoxDetails->show();
        slotUpdateUI(ui->tableView->currentIndex());
    } else {
        ui->groupBoxDetails->hide();
    }

}

void PluginManagerWindow::on_pushButtonLoad_clicked()
{
    QString filterString;
#ifdef Q_OS_WIN
    filterString = tr("DLL (*.dll);;All Files (*.*)");
#elif defined(Q_OS_LINUX)
    filterString = tr("SO (*.so);;All Files (*.*)");
#else
    filterString = tr("All Files (*.*)");
#endif

    QString pluginsDirPath = QCoreApplication::applicationDirPath() + QDir::separator()
                             + QString(PLUGINS_MAIN_DIR) + QDir::separator() + QString(
                                 PLUGINS_MYPLUGINS_DIR);
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select one or more plugin files to load"), pluginsDirPath, filterString);
    if(files.isEmpty()) {
        return;
    }

    foreach(QString file, files) {
        if(!QFile::exists(file)) {
            QMessageBox::critical(this, tr("Error"), tr("File '%1' does not exist!").arg(file));
            break;
        }

        QString errorString = "";
        AbstractPluginInterface *plugin = PluginManager::instance()->loadPlugin(file, &errorString);
        if(!plugin) {
            QMessageBox::critical(this, tr("Error"), tr("Can not load plugin '%1'!\n%2").arg(QFileInfo(file).fileName()).arg(errorString));
            continue;
        }

        pluginInfoModel->addPluginInfo(file, plugin);


        //TODO:Init

    }

    //pluginInfoModel->setPluginsHash(PluginManager::instance()->getPluginsHash());

    ui->tableView->selectRow(0);


}

void PluginManagerWindow::on_pushButtonUnload_clicked()
{
    qDebug("----PluginManagerWindow::on_pushButtonUnload_clicked()");

    QModelIndex index = ui->tableView->currentIndex();
    if(!index.isValid()) {
        return;
    }

    HEHUI::AbstractPluginInterface *plugin = PluginManager::instance()->getPluginsHash().values().at(index.row());
    if(!plugin) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid plug-in instance or plug-in has already been unloaded!"));
        qCritical("ERROR! Invalid plug-in instance or plug-in has already been unloaded!");
        return;
    }

    if(!PluginManager::instance()->unloadPlugin(plugin)) {
        QMessageBox::critical(this, tr("Error"), tr("Can not unload the plug-in '%1'!").arg(plugin->name()));
        qCritical("ERROR! Can not unload the plug-in '%s'!", qPrintable(plugin->name()));
        return;
    }

    pluginInfoModel->removePluginInfo(plugin);
    //    pluginInfoModel->setPluginsHash(PluginManager::instance()->getPluginsHash());

    slotUpdateUI(ui->tableView->currentIndex());

}

void PluginManagerWindow::slotUpdateUI(const QModelIndex &index)
{
    qDebug("----PluginManagerWindow::slotUpdateUI(const QModelIndex &index)");

    ui->pushButtonUnload->setEnabled(index.isValid());

    if((!index.isValid()) || ui->groupBoxDetails->isHidden()) {
        ui->labelAuthor->clear();
        ui->labelURL->clear();
        ui->textBrowserDescription->clear();
        ui->textBrowserLicense->clear();
        return;
    }

    HEHUI::AbstractPluginInterface *plugin = PluginManager::instance()->getPluginsHash().values().at(index.row());
    ui->labelAuthor->setText(plugin->author());
    ui->labelURL->setText(QString("<a href = \"%1\">%1</a>").arg(QUrl::fromUserInput(plugin->url()).toString()));
    ui->textBrowserDescription->setHtml(plugin->description());
    ui->textBrowserLicense->setHtml(plugin->license());

}










} //namespace HEHUI


