#include "m_callback.h"

mCallback *mCallback::instance_ = NULL;
MainUI *mCallback::mainwin = NULL;

mCallback::mCallback(MainUI *window, QObject *parent)
    :  QObject(parent), main_window_(window)
{
    qDebug().nospace() << "ctor of " << this;

    instance_  =  this;

    connect(this, &mCallback::signal_send_log, main_window_, &MainUI::slot_get_log);
    connect(this, &mCallback::signal_set_prg_val, main_window_, &MainUI::slot_set_prg_val);
    connect(this, &mCallback::signal_set_prg_format, main_window_, &MainUI::slot_set_prg_format);

    mainwin = main_window_;

}

mCallback::~mCallback()
{
    qDebug().nospace() << "dtor of " << this;

}

int mCallback::UpdateLog(const QString &text, LogInfor info)
{
    emit instance_->signal_send_log(text, info);
    return 0;
}

mCallback *mCallback::get_instance()
{
    return instance_;
}

MainUI *mCallback::maingui()
{
    return mainwin;
}

QString mCallback::GetU8(uint8_t buff)
{
    return QString("%0").arg(buff, sizeof(uint8_t)*2, 16, QLatin1Char('0')).toUpper().toUtf8();
}
QString mCallback::GetU16(uint16_t buff)
{
    return QString("%0").arg(buff, sizeof(uint16_t)*2, 16, QLatin1Char('0')).toUpper().toUtf8();
}
QString mCallback::GetU32(uint32_t buff)
{
    return QString("%0").arg(buff, sizeof(uint32_t)*2, 16, QLatin1Char('0')).toUpper().toUtf8();
}
qbyte mCallback::GetLe32(quint value, qbool hex)
{
    qbyte res = {};
    QDataStream stream(&res, QIODevice::ReadWrite);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint(value);
    return hex?res.toHex().toUpper() : res;
}

qchar mCallback::GetNum(qchar value)
{
    return value;
}
bool mCallback::IsWow64()
{
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    BOOL bIsWow64 = FALSE;
    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if( NULL != fnIsWow64Process )
    {
        if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
            return false;
    }

    return true;
}
