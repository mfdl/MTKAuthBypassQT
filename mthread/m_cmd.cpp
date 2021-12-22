#include "m_cmd.h"
#include "brom/boot_rom.h"

m_setting::m_setting(){}

mSetting::~mSetting(){}

m_cmd::m_cmd(APKey key):CMD(key){}

m_cmd::~m_cmd(){}

mThreadOPT m_setting::opt() const
{
    return _opt;
}

void m_setting::setOpt(const mThreadOPT &opt)
{
    _opt = opt;
}

mThreadOPT m_cmd::opt() const
{
    return _opt;
}

void m_cmd::setOpt(const mThreadOPT &opt)
{
    _opt = opt;
}

QSharedPointer<CMD> m_setting::CreateCommand(APKey key)
{
    QSharedPointer<m_cmd> cmd(new m_cmd(key));
    cmd->setOpt(opt());
    return cmd;
}

void m_setting::set_stop_flag(int *stop_flag)
{
    QString m_stop_flag(QString().sprintf("0x%08x", stop_flag).toLower());
    qInfo() << QString("mSetting::set_stop_flag:%0").arg(m_stop_flag);
    _opt.stop_flag = stop_flag;
}

void m_cmd::exec()
{
    QSharedPointer<mThreadOPT> m_opt(new mThreadOPT(opt()));

    qInfo() << "m_opt! " << m_opt->option;
    QSharedPointer<boot_rom> mtk(new boot_rom());

    if(m_opt->option == DoMTKSecBypass)
    {
        mtk->bypass_auth();
    }
    if(m_opt->option == DoMTKCrashPL)
    {
        mtk->crash_preloader(1, 0);
    }
    if(m_opt->option == DoMTKDumpPreloader)
    {
        mtk->dump_preloader();
    }
    if(m_opt->option == DoMTKVivoDemoRemove)
    {
        mtk->vivo_remove_demo();
    }
    if(m_opt->option == DoMTKRebootMETA)
    {
        mtk->reboot_meta_mode();
    }
}
