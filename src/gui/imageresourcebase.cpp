/*
 ****************************************************************************
 * imageresourcebase.cpp
 *
 * Created on: 2009-11-6
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
 * Last Modified on: 2010-05-14
 * Last Modified by: 贺辉
 ***************************************************************************
 */





#include <QFile>
#include <QPixmap>

#include "imageresourcebase.h"

namespace HEHUI {

ImageResourceBase::ImageResourceBase(QObject *parent)
    :QObject(parent)
{
    // TODO Auto-generated constructor stub

}

ImageResourceBase::~ImageResourceBase() {
    // TODO Auto-generated destructor stub
}

QIcon ImageResourceBase::createIcon(const QString &iconFileName, const QString &defaultIconName,  QIcon::Mode mode){
    QString file;
    if(QFile::exists(iconFileName)){
        file = iconFileName;
    }else if(QFile::exists(defaultIconName)){
        file = defaultIconName;
    }else{
        file = QString(":/resources/images/emptyicon.png");
    }


    if(mode == QIcon::Disabled){
        QIcon icon;
        QSize size = QImage(file).size();
        QPixmap pixmap = QIcon(file).pixmap(size, mode);
        icon.addPixmap(pixmap);
        return icon;
    }

    return QIcon(file);

}


QIcon ImageResourceBase::emptyIcon()
{
    static const QIcon empty_icon(QLatin1String(":/resources/images/emptyicon.png"));
    return  empty_icon;
}






}
