#ifndef MCALLBACK_H
#define MCALLBACK_H

#include <QObject>
#include <QTime>
#include <QTimer>
#include <QColor>
#include "m_glob.h"
#include <stdio.h>
#include "mainui.h"
#include "ui_mainui.h"
#include "QTreeWidgetItem"

#define append_log(x, ...) \
    mCallback::UpdateLog(x, ##__VA_ARGS__)

#define mainui() \
    mCallback::get_instance()->maingui()

#define mtkui() \
    mCallback::get_instance()->maingui()->mtk_gui()

class MainUI;

class mCallback : public QObject
{
    Q_OBJECT
public:
    typedef void (__stdcall * void_callback)();
    explicit mCallback(MainUI *window, QObject *parent = nullptr);
    ~mCallback();

    void UnlockUI();

    static int __stdcall UpdateLog(const QString &text, LogInfor info = LogNormal);

signals:
    void signal_send_log(const QString &text, LogInfor info);
    void signal_set_prg_val(int val);
    void signal_set_prg_format(QString txt);
public:
    static mCallback *get_instance();
    static MainUI *maingui();
    static QString GetU8(uint8_t buff);
    static QString GetU16(uint16_t buff);
    static QString GetU32(uint32_t buff);
    static QString GetMTKChipName(QString chip_id);
    static QString GetMTKChipEvol(QString chip_id);

public:
    static bool WriteFile(QString path, QByteArray data, bool append = false)
    {
        QFile file(path);

        if (!append && file.exists())
            file.remove();

        if (!file.open(QIODevice::WriteOnly))
            return false;

        file.write(data);
        file.waitForBytesWritten(3000);
        file.close();

        return VerifyFile(file.fileName());
    }

    static QByteArray ReadFile(QString path)
    {
        quint64 startaddr = 0;
        quint32 offset = 0;
        QByteArray res;
        QFile file(path);

        if (startaddr >= (quint64)QFileInfo(path).size())
            return res;

        if (!file.open(QIODevice::ReadOnly))
            return res;

        QDataStream dts(&file);
        file.seek(startaddr);

        if (offset == 0)
            offset = file.size() - startaddr;

        char *ch = new char[offset];

        dts.readRawData(ch, offset);

        res.append(ch, offset);

        delete[] ch;

        file.close();

        return res;
    }

    static int ReadFile(QString path, quint64 startaddr, quint32 offset, QByteArray &output)
    {
        output.clear();

        QFile file(path);

        if (startaddr >= (quint64)QFileInfo(path).size())
            return 0;

        if (!file.open(QIODevice::ReadOnly))
            return 0;

        QDataStream dts(&file);
        file.seek(startaddr);

        if (offset == 0)
            offset = file.size() - startaddr;

        char *ch = new char[offset];

        dts.readRawData(ch, offset);

        output.append(ch, offset);

        delete[] ch;

        file.close();

        return output.length();
    }

    static bool VerifyFile(QString path)
    {
        return ((QFileInfo(path).size() != 0)
                && QFileInfo(path).exists());
    }
    static qbyte GetLe32(quint value, qbool hex = 0);
    static qchar GetNum(qchar value);
    static bool IsWow64();
private:
    MainUI *main_window_;
    static MainUI *mainwin;
    static mCallback *instance_;
};

#endif // MCALLBACK_H
