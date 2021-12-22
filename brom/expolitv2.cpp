#include "boot_rom.h"
#include "m_callback.h"
#include "m_callback.h"
#include "mainui.h"
#include "ui_mainwindow.h"

struct line_coding_struct
{
    quint baudrate{0};
    qchar stop_bits{0};
    qchar m_parity{0};
    qchar data_bits{0};
};

qbool RunProc(qstr programm, qstrl args, qbyte &output)
{
    output.clear();
    QMutex mutex;
    mutex.lock();
    QProcess *runner = new QProcess();

    //    if(!MRXCrypto::VerifyFile(programm))
    //        throwException("missing resources re-install tool !!!");

    if (runner && !(args).isEmpty())
    {
        runner->setEnvironment(QProcess::systemEnvironment());
        runner->setProcessChannelMode(QProcess::MergedChannels);

        runner->setWorkingDirectory(QFileInfo(programm).absolutePath());


        runner->start(programm, args);

        if (runner->waitForStarted())
        {
            if (!runner->waitForReadyRead(-1)) {}
            if (!runner->waitForFinished(-1)) {}
        }

        output.append(runner->readAllStandardOutput() + runner->readAllStandardError());

        while (runner->state() != QProcess::NotRunning)
        {
            //if (mCallback::ThreadTerminated())
            {
                // runner->kill();
                // qthrow("terminated");
            }
        }
    }

    mutex.unlock();
    return output.size();
}

qbool boot_rom::LibUsbFilterDriver(qshort vid, qshort pid)
{
    QTemporaryDir tdir;
    if (!tdir.isValid())
        return 0;
    qstr dir = tdir.path();

    qstr programm = qstr("%0/mrx_usb_filter_%1.exe").arg(dir, mCallback::IsWow64()?"64" : "32");
    qbyte data = mCallback::ReadFile(qstr(":/res/res/win_%0").arg(mCallback::IsWow64()?"64" : "32"));
    qInfo() << "lib_path" << programm;

    data = qUncompress(data);
    if(!mCallback::WriteFile(programm, data))
        return 0;

    qInfo() << "lib_path" << programm;

    qstrl usb_dev_list = {};
    qstr bootrom_dev_path = {};

    //list
    {
        qstrl cmds = qstrl() << "list" << "--class=Ports";
        QByteArray output = {};
        if(!RunProc(programm, cmds, output))
        {
            qInfo() << qstr("libusb:list fail output:%1").arg(output.data());
            return 0;
        }

        QTextStream stream(&output);
        while (!stream.atEnd())
        {
            qstr line = stream.readLine();
            if (!line.size())
                continue;

            //            qInfo() << qstr("libusb:list success output:%1").arg(line.data());

        }

        if(output.contains("libusb-win32 installer requires administrative privileges"))
        {
            // send_log_new("\nplease run software as admin and try again");
            return 0;
        }


        QTextStream st(output);
        while (!st.atEnd())
        {
            qstr line = st.readLine().toLower();

            qstr device = qstr("vid_%0&pid_%1").arg(mCallback::GetU16(vid), mCallback::GetU16(pid)).toLower();
            //            qInfo() << "devxx" << device << line;

            if (!line.contains(device, Qt::CaseInsensitive))
                continue;

            usb_dev_list << line;
        }

        if (usb_dev_list.isEmpty())
        {
            qInfo() << "Mediatek driver not detected #0";
            return 0;
        }

        for (const qstr &line : qAsConst(usb_dev_list))
        {
            if (line.contains("&rev", Qt::CaseInsensitive))
            {
                qstrl llist = line.split(' ', qstr::SkipEmptyParts);
                for (const qstr &lline : qAsConst(llist))
                {
                    if (lline.startsWith("usb\\vid_", Qt::CaseInsensitive))
                        bootrom_dev_path = lline.trimmed();
                }
            }
        }
    }

    if (bootrom_dev_path.isEmpty())
    {
        qInfo() << "Mediatek driver not detected #1";
        return 0;
    }

    //install
    {
        qstrl cmds = qstrl() << "install" << qstr("--device=%1").arg(bootrom_dev_path.replace("&","."));
        QByteArray output = {};
        if(!RunProc(programm, cmds, output))
        {
            qInfo() << qstr("libusb:install fail output:%1").arg(output.data());
            return 0;
        }

        QTextStream stream(&output);
        while (!stream.atEnd())
        {
            qstr line = stream.readLine();
            if (!line.size())
                continue;

            qInfo() << qstr("libusb:install success output:%1").arg(line.data());

        }
    }

    qInfo() << qstr("libusb:bootrom filter installed successfully");

    return 1;

}

qbyte boot_rom::GetBytesFromInt(qsizetype input)
{
    QVector<qsizetype> bytes = {};
    bytes.push_back(input);
    qbyte res = {};
    res.append((char*)bytes.data(), bytes.size()*sizeof(input));
    return res;
}

quint32 boot_rom::GetIntFromBytes(qbyte input)
{
    QVector<quint32> res = {};
    for (int i = 0; i < input.size()/sizeof(quint32); ++i)
        res.push_back(*(quint32*)(input.data()+i*sizeof(quint32)));

    return *res.data();
}

qbool boot_rom::mtk_new_expolit(Config &info, qbool dump_emi)
{
    qInfo().noquote().nospace() << QString("boot_rom::BRomRunNewExploit");

    if(dump_emi)
        send_log_normal("Dumping preloader from memory...\n");
    else
        send_log_normal("Disable BRom protection...\n");

    qbyte ptr_read = {};
    if(!BRomDaRead(info, info.ptr_usbdl, 0x4, ptr_read))
        return 0;

    qInfo().noquote().nospace() << QString("boot_rom::ptr_send(%0)").arg(ptr_read.toHex().data());

    if(!BRomDaReadWrite(info, info.pl_addr, info.payload.size(), info.payload))
        return 0;

    qbyte pl_add = GetBytesFromInt(info.pl_addr);
    quint ptr_send = GetIntFromBytes(ptr_read) + 8;
    qInfo().noquote().nospace() << QString("boot_rom::pl_add(%0)").arg(pl_add.toHex().data());
    qInfo().noquote().nospace() << QString("boot_rom::ptr_send(%0)").arg(mCallback::GetU32(ptr_send));

    if(!BRomDaReadWrite(info, ptr_send, 4, pl_add, 0))
        return 0;

    qInfo().noquote().nospace() << QString("boot_rom::send paylaod ok");
    quint ack = {};
    if(!read32(&ack))
        return 0;

    if(!dump_emi)
    {
        if(ack != 0xa1a2a3a4)
        {
            send_log_fail(qstr("Bypass Payload ACK err:(0x%1)").arg(mCallback::GetU32(ack)));
            return 0;
        }
        else
        {
            //send_log_info("Bypass Payload ACK = 0xa1a2a3a4");
        }
    }
    else
    {
        if(ack != 0xc1c2c3c4)
        {
            send_log_fail(qstr("Dump Payload ACK err:(0x%1)").arg(mCallback::GetU32(ack)));
            return 0;
        }
        else
        {            
            //send_log_info("Dump Payload ACK = 0xc1c2c3c4");

            quint emilen = 0;
            if(!read32(&emilen))
                return 0;

            qint prg_val = 0x00;
            quint blk_sz = 0x1000;
            quint maxlen = qFromBigEndian(emilen);
            qint max_prg = maxlen/blk_sz;

            qbyte prl_buff = {};
            while (max_prg != -1)
            {
                qint rlen = ((max_prg)?blk_sz : (maxlen - prg_val));
                qbyte sec = {};
                qInfo() << "read_prl " << rlen << prl_buff.size();
                if(!read_pattern(sec, rlen))
                    return 0;

                prl_buff.append(sec);

                prg_val += blk_sz;
                max_prg--;
            }

            qbyte search("MTK_BLOADER_INFO");
            qsizetype idx = prl_buff.indexOf(search);
            if (idx != -1)
            {
                qstr emi_name = prl_buff.mid(idx + 0x1b, 0x30);
                qstr rndm = qstr("%0").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd hh_mm_ss")).replace(" ","_").toUtf8();

                qInfo() << "eminame " <<  prl_buff.mid(idx + 0x1b, 0x30);
                qstr path = APP_PATH + "/" + rndm + "_" + emi_name;
                mCallback::WriteFile(path, prl_buff);

                send_log_normal("Preloader file saved to \n");
                send_log_info(path);

                send_log_normal(qstr("Bloader Info Version = (%0)").arg(prl_buff.mid(idx, 0x1b).data()));
                QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
            }
            else
            {
                //send_log_fail("failed to dump preloader");
                qstr rndm = qstr("%0").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd hh_mm_ss")).replace(" ","_").toUtf8();
                qstr path = APP_PATH + "/" + rndm + "_preloader.bin";
                mCallback::WriteFile(path, prl_buff);

                send_log_normal("Preloader file saved to \n");
                send_log_info(path);

                send_log_normal(qstr("Bloader Info Version = (%0)").arg(prl_buff.mid(idx, 0x1b).data()));
                QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
            }
        }
    }

    return (ack == 0xa1a2a3a4 || ack == 0xc1c2c3c4);
}

qbool boot_rom::BRomUpdateLineCoding(quint addr)
{
    qbyte get_linecode = {0x7, Qt::Uninitialized};
    if(!UsbSendDevCtrl(0xa1, 0x21, 0x00, 0x00, get_linecode))
        return 0;

    qbyte set_linecode = {};
    set_linecode.append(get_linecode);
    set_linecode.append(mCallback::GetNum(0x00));
    set_linecode.append(mCallback::GetLe32(addr));
    set_linecode.append(mCallback::GetNum(0x00));

    if(!UsbSendDevCtrl(0x21, 0x20, 0x00, 0x00, set_linecode))
        return 0;

    qbyte reset = {0x7, Qt::Uninitialized};
    if(!UsbSendDevCtrl(0x80, 0x06, 0x0200, 0x00, reset))
        return 0;

    return 1;
}

qbool boot_rom::BRomDaRead(Config &info, quint addr, quint len, qbyte &data, qbool check)
{
    return BRomDaReadWrite(info, addr, len, data, check);
}

qbool boot_rom::BRomDaWrite(Config &info, quint addr, quint len, qbyte &data, qbool check)
{
    return BRomDaReadWrite(info, addr, len, data, check);
}

qbool boot_rom::BRomRegAccess(Config &info, quint addr, quint len, qbyte &data, qbool check)
{
    qint write = data.size()?1:0;

    if(!brom_write8_echo(0xDA))
        return 0;
    if(!write32(write, 1))
        return 0;
    if(!write32(addr, 1))
        return 0;
    if(!write32(len, 1))
        return 0;
    quint16 ack = {};
    read16(&ack);
    if(ack != 0x00)
    {
        send_log_fail(qstr("BRomRegAccess:CMDDAErr0(0x%1)").arg(mCallback::GetU16(ack)));
        send_log_normal("if you're using libusb0 please try again");
        return 0;
    }

    if(write)
    {
        if(!this->write_pattern(data))
            return 0;
    }
    else
    {
        if(!this->read_pattern(data, len))
            return 0;
    }

    if(check)
    {
        quint16 ack = {};
        read16(&ack);
        if(ack != 0x00)
        {
            send_log_fail(qstr("BRomRegAccess:CMDDAErr1(0x%1)").arg(mCallback::GetU16(ack)));
            return 0;
        }
    }

    return 1;
}

qbool boot_rom::BRomDaReadWrite(Config &info, quint addr, quint len, qbyte &data, qbool check)
{
    qInfo().noquote().nospace() << QString("boot_rom::BRomDaReadWrite");

    qbyte null = {};
    if(!BRomRegAccess(info, 0x00, 0x1, null))
        return 0;

    qInfo().noquote().nospace() << QString("boot_rom::BRomDaReadWrite:BRom_ReadCmd32");

    if(!BRom_ReadCmd32(info.wdg_addr + 0x50, 0x1, 0x1, 0x4))
        return 0;

    for (int i = 0; i < 3; i++)
    {
        if(!BRomUpdateLineCoding(info.ptr_da + 8 - 3 + i))
            return 0;
    }

    if(addr < 0x40)
    {
        for (int i = 0; i < 4; i++)
        {
            if(!BRomUpdateLineCoding(info.ptr_da - 6 + (4 - i)))
                return 0;
        }

        if(!BRomRegAccess(info, addr, len, data, check))
            return 0;

        return 1;
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            if(!BRomUpdateLineCoding(info.ptr_da - 5 + (3 - i)))
                return 0;
        }

        if(!BRomRegAccess(info, addr - 0x40, len, data, check))
            return 0;

        return 1;
    }
}
