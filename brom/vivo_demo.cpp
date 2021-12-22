#include "boot_rom.h"
#include "ui_mainwindow.h"
#include "boot_rom_cmd.h"
#include "usb_defs.h"
#include <QtSerialPort/QSerialPortInfo>

quint8 boot_rom::FindPorts(QString &pinfo)
{
    QMutex mutex;
    mutex.lock();

    QElapsedTimer timer = {};

    timer.start();

    while (!(timer.elapsed() >= 80000))
    {
        Q_FOREACH(QSerialPortInfo info, QSerialPortInfo::availablePorts())
        {
            QString desc = info.description().toLower();
            qInfo() << "desc " << desc;
            if (desc.contains("vcom"))
                pinfo.append(info.portName().toLower());
        }

        if(pinfo.size())
            break;

        if(mainui()->Loader()->isStop())
            break;
    }

    mutex.unlock();
    return pinfo.size();
}

vd boot_rom::vivo_remove_demo()
{
    USBCore::USBDevInfo devinfo = {};
    USBCore::USBFinder *usb(new USBCore::USBFinder());

    devinfo.timeout = 60000;
    devinfo.notihnd = QUuid::createUuid().toString().mid(1, 8);

    //    devinfo.filter.push_back("USB\\VID_0E8D&PID_0003");//[BOOTROM_VCOM]
    devinfo.filter.push_back("USB\\VID_0E8D&PID_2000");//[PRELOADER_VCOM]
    devinfo.filter.push_back("USB\\VID_0E8D&PID_2001");//[DA_HIGHSPEED_VCOM]
    devinfo.filter.push_back("USB\\VID_0E8D&PID_2006");//[META_VCOM]
    devinfo.filter.push_back("USB\\VID_22D9&PID_0006");//[OPPO_PRELOADER]
    devinfo.filter.push_back("USB\\VID_1004&PID_6000");//[COMPOSITE_PRELOADER_VCOM + COMPOSITE_DA_VCOM] + LG

    devinfo.guids.push_back(QUuid("{4d36e978-e325-11ce-bfc1-08002be10318}"));//GUID_DEVINTERFACE_COMPORT
    devinfo.guids.push_back(QUuid("{a5dcbf10-6530-11d2-901f-00c04fb951ed}"));//GUID_DEVINTERFACE_USB_DEVICE
    devinfo.guids.push_back(QUuid("{86e0d1e0-8089-11d0-9ce4-08003e301f73}"));//GUID_DEVINTERFACE_COMPORT

    send_log_normal("Waiting for mtk usb device... ");

    if (!usb->WaitForUsbDevice(devinfo))
    {
        send_log_fail("device not found!");
        return;
    }

    qint portnumber = devinfo.PortNum;

    send_log_succes("ok");

    send_log_normal("BootMode : ");
    send_log_info(qstr("%0[COM%1]").arg(devinfo.BootMode).arg(devinfo.PortNum));
    send_log_normal("USBDriver : ");
    send_log_fail(qstr("%0:%1:%2").arg(devinfo.USBDriver, devinfo.DriverVersion, devinfo.DriverDate));

    qInfo().noquote().nospace() << "USBDevInfo::PortName:" << devinfo.PortName;
    qInfo().noquote().nospace() << "USBDevInfo::Manufacturer:" << devinfo.Manufacturer;
    qInfo().noquote().nospace() << "USBDevInfo::USBDriver:" << devinfo.USBDriver;
    qInfo().noquote().nospace() << "USBDevInfo::DriverVersion:" << devinfo.DriverVersion;
    qInfo().noquote().nospace() << "USBDevInfo::DriverDate:" << devinfo.DriverDate;
    qInfo().noquote().nospace() << "USBDevInfo::PortNum:" << devinfo.PortNum;

    send_log_normal("Sync with device started...\n");

    if(!connect_brom(portnumber))
        return;

    qint8 retry = 4;
    while(retry > 0)
    {

        QByteArray ready = {};
        ret = read_pattern(ready, 5);
        if(ready != "READY")
            return;

        qInfo() << "retry READY." << retry;
        --retry;
    }

    send_log_info("Synchronous ready ack ok!");

    send_log_normal("Switch to diag mode...\n");

    if(!write_pattern("FACTORYM"))
        return;

    if(!write_pattern(QByteArray::fromHex("040000000100000000000000")))
        return;

    QByteArray ack = {};
    ret = read_pattern(ack, 13);
    if(ack != "READYMYROTCAF")
        return;

    write_pattern(QByteArray::fromHex("6200000000005C000000"));
    // BRom_DAResetWDT();
    write_pattern(QByteArray::fromHex("8012000000005C0000003B7D95000080318065B0831148C883009000"));

    disconnect_brom();

    send_log_normal("wait for diag mode ...\n");

    Sleep(20000);

    QString port = {};
    if(!FindPorts(port))//0xe8d,0x2000-0x200A,0x2006
        return;

    uint portnum = 0;
    QRegExp regex("com(\\d+)");
    if(regex.indexIn(port)!= -1)
        portnum = regex.cap(1).toInt();

    send_log_normal("Sync with diag started...\n");

    if(!connect_brom(portnum))
        return;

    send_log_normal("Reading device information...\n");

    QString devid = SendATCmd("AT+ETST=1\r");
    if(!devid.isEmpty())
    {
        send_log_normal("●Device\t");
        send_log_red(devid);
    }

    QString imei1 = SendATCmd("AT+EGMR=0,7\r");
    if(!imei1.isEmpty())
    {
        send_log_normal(" ●IMEI1\t");
        send_log_info(imei1);
    }

    QString imei2 = SendATCmd("AT+EGMR=0,10\r");
    if(!imei2.isEmpty())
    {
        send_log_normal(" ●IMEI2\t");
        send_log_info(imei2);
    }

    this->timeout = 10000;
    //vivo
    QString vivo_os =  SendATCmd("AT+BKPROP=0,ro.vivo.os.build.display.id\r");
    QString vivo_screen =  SendATCmd("AT+BKPROP=0,persist.vivo.phone.panel_type\r");
    QString vivo_nfc_chip =  SendATCmd("AT+BKPROP=0,persist.vendor.vivo.nfc.chip.type\r");
    QString vivo_sys_ver =  SendATCmd("AT+BKPROP=0,ro.vivo.system.product.version\r");
    QString vivo_hw_ver =  SendATCmd("AT+BKPROP=0,ro.vivo.system.hardware.version\r");
    //vivo
    QString manufac =  SendATCmd("AT+BKPROP=0,ro.product.manufacturer\r");
    QString model =  SendATCmd("AT+BKPROP=0,ro.product.model\r");
    QString platform =  SendATCmd("AT+BKPROP=0,ro.board.platform\r");
    QString release =  SendATCmd("AT+BKPROP=0,ro.build.version.release\r");
    QString buildid = SendATCmd("AT+BKPROP=0,ro.build.id\r");
    QString display_id =  SendATCmd("AT+BKPROP=0,ro.build.display.id\r");
    QString product =  SendATCmd("AT+BKPROP=0,ro.build.product\r");
    QString marketname =  SendATCmd("AT+BKPROP=0,ro.product.marketname\r");
    QString hwc =  SendATCmd("AT+BKPROP=0,ro.boot.hwc\r");

    QString sec_patch = SendATCmd("AT+BKPROP=0,ro.build.version.security_patch\r");
    QString region = SendATCmd("AT+BKPROP=0,ro.product.locale.region\r");
    QString branch = SendATCmd("AT+BKPROP=0,ro.mediatek.version.branch\r");
    QString incremental = SendATCmd("AT+BKPROP=0,ro.build.version.incremental\r");

    if(!manufac.isEmpty())
    {
        send_log_normal("● ManfcID\t");
        send_log_info(manufac);
    }
    if(!model.isEmpty())
    {
        send_log_normal("● Model\t");
        send_log_info(model);
    }
    if(!hwc.isEmpty())
    {
        send_log_normal("● HWC\t");
        send_log_info(hwc);
    }
    if(!marketname.isEmpty())
    {
        send_log_normal("● Name\t");
        send_log_info(marketname);
    }
    if(!platform.isEmpty())
    {
        send_log_normal("● Platform\t");
        send_log_succes(platform);
    }
    if(!release.isEmpty())
    {
        send_log_normal("● Version\t");
        send_log_succes(release);
    }
    if(!display_id.isEmpty())
    {
        send_log_normal("● Fw Ver\t");
        send_log_info(display_id);
    }
    if(!buildid.isEmpty())
    {
        send_log_normal("● BuildID\t");
        send_log_info(buildid);
    }
    if(!product.isEmpty())
    {
        send_log_normal("● Product\t");
        send_log_info(product);
    }
    if(!incremental.isEmpty())
    {
        send_log_normal("● IncVer\t");
        send_log_info(incremental);
    }

    if(!release.isEmpty())
    {
        send_log_normal("● Android\t");
        send_log_info(release);
    }

    if(!sec_patch.isEmpty())
    {
        send_log_normal("● S Patch\t");
        send_log_info(sec_patch);
    }
    if(!region.isEmpty())
    {
        send_log_normal("● Region\t");
        send_log_succes(region);
    }

    //vivo
    if(!vivo_os.isEmpty())
    {
        send_log_normal("● VivoOs\t");
        send_log_succes(vivo_os);
    }

    if(!vivo_screen.isEmpty())
    {
        send_log_normal("● Screen\t");
        send_log_info(vivo_screen);
    }
    if(!vivo_nfc_chip.isEmpty())
    {
        send_log_normal("● NFC\t");
        send_log_succes(vivo_nfc_chip);
    }
    if(!vivo_sys_ver.isEmpty())
    {
        send_log_normal("● SYSVer\t");
        send_log_info(vivo_sys_ver);
    }
    if(!vivo_hw_ver.isEmpty())
    {
        send_log_normal("● HWVer\t");
        send_log_succes(vivo_hw_ver);
    }

    send_log_normal("Reparing device security...\n");

    QString bkrid = SendATCmd("AT+BKRID=1\r");
    if(!bkrid.isEmpty())
    {
        send_log_succes(QString("ID:%0").arg(bkrid));
    }

    send_log_normal("Remove demo security...\n ");

    this->timeout = 10000;

    qInfo() << SendATCmd("AT+BKFEATURE=1,0\r");
    qInfo() << SendATCmd("AT+BKZSJ=0\n");
    qInfo() << SendATCmd("AT+BKRT=1\r\n");
    qInfo() << SendATCmd("AT+BKRTALL=1\r");
    qInfo() << SendATCmd("AT+BKFEATURE=1,0\r");
    qInfo() << SendATCmd("AT+BKRTALL=1\r");

    send_log_succes("all ok please reboot your device manually.");
}

QString boot_rom::SendATCmd(QByteArray input)
{
    qInfo() << __PRETTY_FUNCTION__;

    if (!this->_hcom)
        return QString();

    ::PurgeComm(this->_hcom, PURGE_RXCLEAR);

    qInfo() << "write_pattern " << input;

    if (!write_pattern(input))
        return QString();

    qInfo() << "brom_get ";

    QByteArray read(0xff, Qt::Uninitialized);
    this->brom_get((quint8*)read.data(), read.size() , 1000); //should async read + no exception , readall because the return len is not expected.
    qInfo() << "read " << read;
    if(read.contains("ERROR") || read.contains("NULL"))
        return QString();

    if(read.contains('"'))
        read = read.split('"').at(1).trimmed();
    else if(read.contains('\r\n'))
        read = read.split('\r\n').at(1).trimmed();
    else if(read.contains('\r\nOK\r\n'))
        read = qstr(read.split('\r\nOK\r\n').at(0)).split("\r\n").at(0).toLatin1();

    qInfo() << "read2 " << read.simplified().toUpper();

    return read.simplified().toUpper();
}
