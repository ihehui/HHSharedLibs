/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://www.qtsoftware.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/QFile>
#include <QIcon>
#include <QPixmap>
#include <QDebug>

#include "iconloader_p.h"



namespace HEHUI
{

/*
 QIcon createIconSet(const QString &name)
{
    QStringList candidates = QStringList()
        << (QString::fromUtf8(":/trolltech/formeditor/images/") + name)
#ifdef Q_WS_MAC
        << (QString::fromUtf8(":/trolltech/formeditor/images/mac/") + name)
#else
        << (QString::fromUtf8(":/trolltech/formeditor/images/win/") + name)
#endif
        << (QString::fromUtf8(":/trolltech/formeditor/images/designer_") + name);

    foreach (QString f, candidates) {
        if (QFile::exists(f))
            return QIcon(f);
    }

    //return QIcon();
    return emptyIcon();
}

 QIcon emptyIcon()
{
    static const QIcon empty_icon(QLatin1String(":/trolltech/formeditor/images/emptyicon.png"));
    return  empty_icon;
}
*/



QIcon createIconSet(const QString &iconFileName, const QString &defaultIconName,  QIcon::Mode mode)
{

    QString file;
    if(QFile::exists(iconFileName)) {
        file = iconFileName;
    } else if(QFile::exists(defaultIconName)) {
        file = defaultIconName;
    } else {
        file = QString(":/resources/images/emptyicon.png");
    }



    if(mode == QIcon::Disabled) {
        QIcon icon;
        QSize size = QImage(file).size();
        QPixmap pixmap = QIcon(file).pixmap(size, mode);
        icon.addPixmap(pixmap);
        return icon;
    }



    //	QImage image(file);
    //	if(!image.isNull()){
    //		icon.addPixmap(QPixmap::fromImage(image), QIcon::Disabled, QIcon::Off);
    //	}



    return QIcon(file);

}


} // namespace HEHUI


