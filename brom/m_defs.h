#ifndef M_DEFS_H
#define M_DEFS_H

#include <QtCore>
#include <QObject>
#include <QThread>
#include <QSerialPortInfo>
#include <windows.h>
#include <iostream>
#include <iomanip>

using namespace std;

#define MAX_BUF_LEN 1024
#define RIDLength 16

#define MTK_CONNECT_TIMEOUT 80*1000

#define MAX_IMEI_DIGIT_LEN 14
#define IMEI_ARRAY_LEN 16
#define MAX_IMEI_NUMS 4
#define IMEI_MAX_LENGTH  15

//typedef struct MTKBootModeSwitchArg
//{
//    QString boot_mode;
//    QString port_name;
//    SP_BOOT_ARG_S arg;
//    uint com_p_num;
//    int *boot_stop;
//}MTKBootModeSwitchArg;

static int rett = 0;

typedef struct mtkPortInfo
{
    QString name = "";
    QString type = "";
    uint port = 0;
    QString vid = "";
    QString pid = "";

} mtkPortInfo;

#endif // M_DEFS_H
