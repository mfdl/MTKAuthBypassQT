#ifndef APP_H
#define APP_H
#include <QApplication>
#include <QString>

class QString;

#define AppName "mtksecbypass"
#define AppPath qApp->applicationDirPath()
#define QDATE qPrintable (QDate::currentDate().toString("yyyy-MM-dd"))
#define DATETIME qPrintable (QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))

class App
{
public:
    static int IconMain;                //左上角图标
    static int IconMenu;                //下来菜单图标
    static int IconMin;                 //最小化图标
    static int IconMax;                 //最大化图标
    static int IconNormal;              //正常图标
    static int IconClose;               //关闭图标

    static bool UseTray;                //启用托盘
    static bool SaveLog;                //输出日志文件
    static bool UseStyle;               //启用样式
    static QString StyleName;           //应用程序样式
    static int FontSize;                //应用程序字体大小
    static QString FontName;            //应用程序字体名称

    static QString Title;               //标题

    static void ReadConfig();           //读取配置文件,在main函数最开始加载程序载入
    static void WriteConfig();          //写入配置文件,在更改配置文件程序关闭时调用
    static void NewConfig();            //以初始值新建配置文件
    static bool CheckConfig();          //校验配置文件

    static void WriteError(QString str);//写入错误信息
    static void WriteStartTime();       //写入启动时间
    static void NewDir(QString dirName);//新建目录
};

#endif // APP_H
