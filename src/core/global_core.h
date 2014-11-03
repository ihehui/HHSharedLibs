/*
 ****************************************************************************
 * global_core.h
 *
 * Created on: 2009-4-27
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
  * Last Modified on: 2010-05-07
  * Last Modified by: 贺辉
  ***************************************************************************
*/


#ifndef GLOBAL_CORE_H_
#define GLOBAL_CORE_H_

 #include "core_lib.h"

#ifdef Q_OS_WIN
#define OS_IS_WINDOWS 1
#else
#define OS_IS_WINDOWS 0
#endif


//SQLite
#ifndef LOCAL_CONFIG_DB_CONNECTION_NAME
#define LOCAL_CONFIG_DB_CONNECTION_NAME	"LOCAL_CONFIG_DB"
#endif

#ifndef LOCAL_CONFIG_DB_NAME
#define LOCAL_CONFIG_DB_NAME	".config.db"
#endif

#ifndef LOCAL_CONFIG_DB_DRIVER
#define LOCAL_CONFIG_DB_DRIVER	"QSQLITE"
#endif




//MySQL
#ifndef REMOTE_SITOY_COMPUTERS_DB_CONNECTION_NAME
#define REMOTE_SITOY_COMPUTERS_DB_CONNECTION_NAME	"SITOY_COMPUTERS_DB"
#endif

#ifndef REMOTE_SITOY_COMPUTERS_DB_DRIVER
#define REMOTE_SITOY_COMPUTERS_DB_DRIVER	"QMYSQL"
#endif

#ifndef REMOTE_SITOY_COMPUTERS_DB_SERVER_HOST
#define REMOTE_SITOY_COMPUTERS_DB_SERVER_HOST	"200.200.200.40"
#endif

#ifndef REMOTE_SITOY_COMPUTERS_DB_SERVER_PORT
#define REMOTE_SITOY_COMPUTERS_DB_SERVER_PORT	3306
#endif

#ifndef REMOTE_SITOY_COMPUTERS_DB_USER_NAME
#define REMOTE_SITOY_COMPUTERS_DB_USER_NAME	"hehui"
#endif

#ifndef REMOTE_SITOY_COMPUTERS_DB_USER_PASSWORD
#define REMOTE_SITOY_COMPUTERS_DB_USER_PASSWORD	"hehui"
#endif

#ifndef REMOTE_SITOY_COMPUTERS_DB_NAME
#define REMOTE_SITOY_COMPUTERS_DB_NAME	"sitoycomputers"
#endif



//Sitoy SQL Server
#ifndef REMOTE_SITOY_SQLSERVER_DB_CONNECTION_NAME
#define REMOTE_SITOY_SQLSERVER_DB_CONNECTION_NAME	"SITOY_SQLSERVER_DB"
#endif

#ifndef REMOTE_SITOY_SQLSERVER_DB_DRIVER
#define REMOTE_SITOY_SQLSERVER_DB_DRIVER	"QODBC"
#endif

#ifndef REMOTE_SITOY_SQLSERVER_DB_NAME
#define REMOTE_SITOY_SQLSERVER_DB_NAME	"mis"
#endif

#ifndef REMOTE_SITOY_SQLSERVER_DB_HOST_NAME
#define REMOTE_SITOY_SQLSERVER_DB_HOST_NAME	"200.200.200.2"
#endif

#ifndef REMOTE_SITOY_SQLSERVER_DB_HOST_PORT
#define REMOTE_SITOY_SQLSERVER_DB_HOST_PORT	1433
#endif

#ifndef REMOTE_SITOY_SQLSERVER_DB_USER_NAME
#define REMOTE_SITOY_SQLSERVER_DB_USER_NAME	"appuser"
#endif

#ifndef REMOTE_SITOY_SQLSERVER_DB_USER_PASSWORD
#define REMOTE_SITOY_SQLSERVER_DB_USER_PASSWORD	"apppswd"
#endif


#ifndef LANGUAGE_FILE_PREFIX
#define LANGUAGE_FILE_PREFIX	"ts_"
#endif
//const QString LANGUAGE_FILE_PREFIX = "ts_";

#ifndef RESOURCE_PATH
#define RESOURCE_PATH	":/resources"
#endif
//const QString RESOURCE_PATH = ":/resources";



#ifndef LANGUAGE_FILE_DIR
#define LANGUAGE_FILE_DIR	"translations"
#endif


#ifndef PLUGINS_MAIN_DIR
#define PLUGINS_MAIN_DIR	"plugins"
#endif

#ifndef PLUGINS_MYPLUGINS_DIR
#define PLUGINS_MYPLUGINS_DIR	"hehui"
#endif

#ifndef MYLIBS_DIR
#define MYLIBS_DIR	"lib"
#endif

/*

#ifndef APP_NAME
#define APP_NAME	"APP"
#endif
//const QString APP_NAME = "Administrator Assistant";

#ifndef APP_VERSION
#define APP_VERSION	"2009.09.04"
#endif
//const QString APP_VERSION = "2009.06.23";

#ifndef APP_ICON_PATH
#define APP_ICON_PATH	"/images/app.png"
#endif
//const QString APP_ICON = RESOURCE_PATH + "/images/app.png";

#ifndef APP_AUTHOR
#define APP_AUTHOR	"HeHui"
#endif
//const QString APP_AUTHOR = "HeHui";

#ifndef APP_AUTHOR_EMAIL
#define APP_AUTHOR_EMAIL	"hehui@hehui.org"
#endif
//const QString APP_AUTHOR_EMAIL = "hehui@hehui.org";

#ifndef APP_ORG
#define APP_ORG	"HeHui"
#endif
//const QString APP_ORG = "HeHui";

#ifndef APP_ORG_DOMAIN
#define APP_ORG_DOMAIN	"www.hehui.org"
#endif
//const QString APP_ORG_DOMAIN = "www.hehui.org";

#ifndef APP_LICENSE
#define APP_LICENSE	"LGPL"
#endif
//const QString APP_LICENSE = "LGPL";

#ifndef APP_SPLASH_IMAGE_PATH
#define APP_SPLASH_IMAGE_PATH	"/images/splash.png"
#endif
//const QString APP_SPLASH_IMAGE_PATH= RESOURCE_PATH + "/images/splash.png";

*/


namespace HEHUI {


enum DatabaseType {
        OTHER = 0,
        MYSQL = 1,
        SQLITE = 2,
        POSTGRESQL = 3,
        FIREBIRD = 4,
        DB2 = 5,
        ORACLE = 6,
        M$SQLSERVER = 7,
        M$ACCESS =8,

};

enum WindowPosition{
	Center = 0,
	BottomRight = 1
	};



}/* namespace */


#endif /* GLOBAL_CORE_H_ */
