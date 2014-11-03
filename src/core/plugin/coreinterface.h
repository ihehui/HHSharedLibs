/*
 ****************************************************************************
 * coreinterface.h
 *
 * Created on: 2010-4-10
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
  * Last Modified on: 2010-05-12
  * Last Modified by: 贺辉
  ***************************************************************************
*/




#ifndef COREINTERFACE_H_
#define COREINTERFACE_H_

#include <QObject>

#include "abstractplugininterface.h"
#include "../core_lib.h"


namespace HEHUI {

class CORE_LIB_API CoreInterface : public AbstractPluginInterface{
    public:

        virtual ~CoreInterface(){}

//        virtual QString name() const = 0;
//        virtual QString version() const = 0;
//        virtual QString vendor() const = 0;
//        virtual QString author() const = 0;
//        virtual QString url() const = 0;
//        virtual QString description() const = 0;
//        virtual QString license() const = 0;
//
//        virtual bool isSingle() = 0;

        //Initialization
        virtual bool initialize(QObject *parent = 0) = 0;


        //Unload the plugin
//        virtual bool unload() = 0;


    };


}

Q_DECLARE_INTERFACE(HEHUI::CoreInterface, "org.hehui.Interface.CoreInterface/2012.1")



#endif /* COREINTERFACE_H_ */
