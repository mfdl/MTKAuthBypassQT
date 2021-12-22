#ifndef MTHREAD_DEFS_H
#define MTHREAD_DEFS_H

#include <QtCore>
#include <QThread>
#include "m_tcb.h"
#include <QObject>
#include <QtWidgets>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define qnull_hnd ((HANDLE) (LONG_PTR)-1)
#ifdef __cplusplus
}
#endif

#define qnull nullptr
#define qzero 0

using namespace std;

typedef unsigned __LONG32 qdword;

typedef void qvoid;
typedef bool qbool;
typedef int qint;
typedef unsigned int quint;
typedef unsigned long long qlong;
typedef signed char qschar;
typedef unsigned char qchar;
typedef unsigned short qshort;

typedef quint32_be quint_be;
typedef quint16_be qshort_be;

typedef QByteArray qbyte;
typedef QString qstr;
typedef QStringList qstrl;
typedef QFile qfile;
typedef QMessageBox qmsg;

typedef int APKey; //the only identification for GUI.

const int AP_STOP_FLAG = 9876;

static int ret = 0;

enum mThreadFunc
{
    DoMTKSecBypass,
    DoMTKCrashPL,
    DoMTKDumpPreloader,
    DoMTKVivoDemoRemove,
    DoMTKRebootMETA
};

typedef struct ScatterInfo
{
    QString	name[0xff];
    QString	path[0xff];
    boolean	isdl[0xff];
    quint16	pcnt;
} ScatterInfo;

typedef struct mThreadOPT
{
    mThreadFunc option;
    int *stop_flag;
    QString savepath;
    bool reboot;
}mThreadOPT;

#define APP_PATH QDir::currentPath()


typedef struct Config
{
    uint16_t ptr0{};
    uint16_t ptr1{};
    uint16_t ptr_usbdl{};
    uint16_t ptr_da{};
    quint pl_addr{};
    qbyte payload{};

    uint16_t hwcode;
    uint32_t wdg_addr;
    uint32_t payload_addr;
    uint32_t uart_base;
    int var0;
    int var1;
    QString payloadname;
}Config;


typedef struct Ext4FileInfo
{
    QString root;
    QString name;
    quint64 addr;
    quint64 flen;
    quint16 wipe;
    quint16 isok;
    QByteArray buff;
}Ext4FileInfo;
//************************************************

#define MTK_CONNECT_TIMEOUT 80*1000

#define QSHARED_PTR QSharedPointer
#define SUNIQUE_PTR std::unique_ptr

#define SEC_MAGIC 0x58881688

#define SCSKC_MAGIC  "1376"

struct gfh_header
{
    uint32_t m_magic_ver;
    uint16_t m_size;
    uint16_t m_type;
};

struct gfh_file_info
{
    gfh_header  m_gfh_hdr;
    char        m_id[12];
    uint32_t    m_file_ver;
    uint16_t    m_file_type;
    uint8_t     m_flash_dev;
    uint8_t     m_sig_type;
    uint32_t    m_loader_addr;
    uint32_t    m_file_len;
    uint32_t    m_max_size;
    uint32_t    m_content_offset;
    uint32_t    m_sig_len;
    uint32_t    m_jump_offset;
    uint32_t    m_attr;
};

//struct image_info_t
//{
//    QString     file_name;
//    uint64_t    start_addr;
//    uint64_t    length;
//};

struct mtk0_hdr_t
{
    uint32_t    magic;
    uint32_t    file_length;
    uint8_t     file_name[0x20];
    uint64_t    unused;
    uint32_t    magic2;
    uint32_t    hdr_length;
};

#define QSHARED_PTR QSharedPointer
#define SUNIQUE_PTR std::unique_ptr

//mtk sec
#define SEC_CFG_SBC_EN 0x00000001
#define SEC_CFG_SLA_EN 0x00000002
#define SEC_CFG_DAA_EN 0x00000004

enum MTKDASendMethod
{
    mtk_AutoBoot,
    mtk_CustomDaBoot,
    mtk_CustomBrandBoot,
};

struct SCardInfo
{
    QByteArray skey = "";
    QByteArray uuid = "";
    bool cardok = false;
};

struct MTKMetaNetworkData
{
    QString IMEI[2] = {};
    QString BtMac = "ffffffffffffff0";
    QString WiMac = "ffffffffffffff0";
    QString SNNum = "ffffffffffffff0";
    QByteArray APDB = "";
    QByteArray MDDB[2] = {};
    bool multi_sim = false;
    bool imei_csum = false;
};

#define MP0B_001_MAGIC "545f90d0e0e1654aaa74d3ce"

#define PHONEBOOK "/data/data/com.android.providers.contacts/databases/contacts2.db"
#define MMSSMS_DB "/data/data/com.android.providers.telephony/databases/mmssms.db"
#define PhDbHelper "use this site to convert .db file to vcf file \n http://gsmrecovery.ru/db2vc/index.php?lang=en"

#define RIDLen 16

#define dbg(str) qDebug() << str;

#define MDDBW    ui->MDDBWidget->ui->loader
#define DAW      ui->DAWidget->ui->loader
#define AuthW    ui->AuthWidget->ui->loader
#define ScatterW ui->ScatterWidget->ui->loader
#define HWMTKW   ui->HWMTKUI->ui->loader
#define MTKEmmcW ui->MTKEmmcWidget->ui->loader
#define MTKOFPW  ui->OPPOMTKUI->ui->loader
#define MTKEMIW  ui->MTKEMIWidget->ui->loader

#define Dragger TextDragger

#define qtv     QTreeView
#define qtrw    QTreeWidget
#define qmw     QMainWindow
#define qla1ch  QLatin1Char
#define vd      void
#define qfrw    QFutureWatcher
#define u8      quint8
#define u16     quint16
#define u32     quint32
#define u64     quint64
#define i8      qint8
#define i16     qint16
#define i32     qint32
#define i64     qint64
#define dbl     double
#define bl      bool
#define qstr    QString
#define qvar    QVariant
#define qstrl   QStringList
#define qstl    QStringList
#define qba     QByteArray
#define qbya    QByteArray
#define qvec    QVector
#define qt      QTimer
#define qd      QDir
#define qdi     QDirIterator
#define qf      QFile
#define qfil    QFile
#define qfi     QFileInfo
#define qfii    QFileInfo
#define qid     QIODevice
#define qios    QIODevice
#define qidRW   qid::ReadWrite
#define qidRO   qid::ReadOnly
#define qidWO   qid::WriteOnly
#define qidAp   qid::Append
#define qidTx   qid::Text
#define qel     QEventLoop
#define qm      QMap
#define qmi     QMapIterator
#define qds     QDataStream
#define qdts    QDataStream
#define qts     QTextStream
#define ql      QList
#define qcb     QComboBox
#define qs      QSettings
#define qh      QHash
#define qhi     QHashIterator
#define qre     QRegExp
#define qsp     QSerialPort
#define qspi    QSerialPortInfo
#define qmo     QMetaObject
#define qo      QObject
#define qobj    QObject
#define qdt     QDateTime
#define qfr     QFuture
#define qcc     QtConcurrent
#define qdd     QDomDocument
#define qxsr    QXmlStreamReader
#define qxsw    QXmlStreamWriter
#define qxsa    QXmlStreamAttribute
#define qde     QDomElement
#define qdn     QDomNode
#define qdnnm   QDomNamedNodeMap
#define qda     QDomAttr
#define qp      QPair
#define qch     QCryptographicHash
#define qlbl    QLabel
#define qtwi    QTableWidgetItem
#define qtw     QTableWidget
#define qstdp   QStandardPaths
#define qfd     QFileDialog
#define qdlg    QDialog
#define qlw     QListWidget
#define qtbb    QWinTaskbarButton
#define qtbp    QWinTaskbarProgress
#define qse     QShowEvent
#define qc      QColor
#define qu      QUrl
#define qpt     QPainter
#define qhv     QHeaderView
#define qw      QWidget
#define qme     QMouseEvent
#define qfm     QFontMetrics
#define qte     QTextEdit
#define qle     QLineEdit
#define qlf     QLineF
#define qft     QFont
#define qpnt    QPoint
#define qpm     QPixmap
#define qlwi    QListWidgetItem
#define qdpe    QDropEvent
#define qdee    QDragEnterEvent
#define qmmd    QMimeData
#define qpb     QPushButton
#define qchb    QCheckBox
#define qrb     QRadioButton
#define qic     QIcon
#define qpgb    QProgressBar
#define qpe     QPaintEvent
#define qmsg    QMessageBox
#define qpxm    QPixmap
#define qdir    QDir
#define qmap    QMap
#define qmai    QMapIterator
#define qdat    QDateTime
#define qtri    QTreeWidgetItem


struct UserLockPolicy
{
    u8   len;
    u8   digit;
    u8   word;
    u8   upper;
    u8   lower;
    u8   symbol;
};

enum LogInfor
{
    LogNormal,
    LogOkay,
    LogInfo,
    LogFail,
    LogRed,
    LogBold
};

#define send_log_normal(x)  emit mCallback::get_instance()->signal_send_log(qstr("%0").arg(x), LogNormal)
#define send_log_succes(x)  emit mCallback::get_instance()->signal_send_log(qstr("%0").arg(x) , LogOkay)
#define send_log_info(x)    emit mCallback::get_instance()->signal_send_log(qstr("%0").arg(x) , LogInfo)
#define send_log_fail(x)  emit mCallback::get_instance()->signal_send_log(qstr("%0").arg(x) , LogFail)
#define send_log_red(x)  emit mCallback::get_instance()->signal_send_log(qstr("%0").arg(x) , LogRed)
#define send_log_bold(x)  emit mCallback::get_instance()->signal_send_log(qstr("%0").arg(x), LogBold)

#endif // MTHREAD_DEFS_H
