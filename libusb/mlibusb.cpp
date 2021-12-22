#include "mlibusb.h"
#include "mfdproc.h"

MLibUSB::MLibUSB() :
    use_usbk(0)
{
    filter_bootrom();
}

MLibUSB::~MLibUSB(){}

bool MLibUSB::BRom_UsbCtrl(uint8_t requesttype, uint8_t request, uint16_t value, uint16_t index, QByteArray &buff, qbool reset)
{
    qsizetype ret = 0;

    if (this->use_usbk)
    {
        if(!lusb_api.isLoaded())
            return ENOFILE;

        typedef int LIBUSB_CALL (*libusb_control_transfer_t)(libusb_device_handle *dev_handle,
                                                             uint8_t request_type, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
                                                             unsigned char *data, uint16_t wLength, unsigned int timeout);
        libusb_control_transfer_t libusb_ctrl_transfer = (libusb_control_transfer_t)lusb_api.resolve("libusb_control_transfer");
        if(!libusb_ctrl_transfer)
            return NULL;

        typedef const char * LIBUSB_CALL (*libusb_error_name_t)(int errcode);
        libusb_error_name_t libusb1_error_name = (libusb_error_name_t)lusb_api.resolve("libusb_error_name");
        if(!libusb1_error_name)
            return NULL;

        if (!this->_devh)
            return 0;

        USBCtrlPkt pkt = {};
        pkt.bmRequestType = requesttype;
        pkt.bRequest = request;
        pkt.wValue = quint16_le(value);
        pkt.wIndex = quint16_le(index);
        pkt.wLength = quint16_le(buff.size());

        QByteArray PKT((char*)&pkt, sizeof(pkt));
        qInfo().noquote().nospace() << qstr("MLibUSB::BRom_UsbCtrl1{PKT:%0}").arg(PKT.toHex().data());

        if(!buff.size() && index != 0xa)
        {
            buff.append((char*)&pkt, sizeof(pkt));
            pkt.wLength = sizeof(pkt);
        }

        qsizetype ret = libusb_ctrl_transfer(_devh,
                                             pkt.bmRequestType,
                                             pkt.bRequest,
                                             pkt.wValue,
                                             pkt.wIndex,
                                             (uchar*)buff.data(),
                                             pkt.wLength,
                                             0x00000000);


        qInfo().noquote().nospace() << qstr("MLibUSB::BRom_UsbCtrl1{OUT_DATA:%0}").arg(buff.toHex().data());

        qInfo().noquote().nospace() << QString("MLibUSB::BRom_UsbCtrl1{ret:%0:%1}").arg(QString::number(ret), libusb1_error_name(ret));

        return 1;
    }
    else
    {
        if(!OpenUSBDevice())
            return 0;

        USBCtrlPkt pkt = {};
        pkt.bmRequestType = requesttype;
        pkt.bRequest = request;
        pkt.wValue = quint16_le(value);
        pkt.wIndex = quint16_le(index);
        pkt.wLength = quint16_le(buff.size());

        QByteArray PKT((char*)&pkt, sizeof(pkt));
        qInfo().noquote().nospace() << qstr("MLibUSB::BRom_UsbCtrl0{PKT:%0}").arg(PKT.toHex().data());

        ret = usb_control_msg(this->hnd,
                              pkt.bmRequestType,
                              pkt.bRequest,
                              pkt.wValue,
                              pkt.wIndex,
                              buff.data(),
                              pkt.wLength,
                              0x00000000);

        qInfo().noquote().nospace() << qstr("MLibUSB::BRom_UsbCtrl0{OUT_DATA:%0}").arg(buff.toHex().data());

        qInfo().noquote().nospace() << QString("MLibUSB::BRom_UsbCtrl0{ret:%0:%1}").arg(QString::number(ret), usb_strerror());

        if(!CloseUSBDevice(reset))
            return 0;

        return (ret == 0 || ret == buff.size() || ret);
    }
}

quint32 MLibUSB::Write(qbyte input, quint32 size)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int LIBUSB_CALL (*libusb_bulk_transfer_t)(libusb_device_handle *dev_handle,
                                                      unsigned char endpoint, unsigned char *data, int length,
                                                      int *actual_length, unsigned int timeout);
    libusb_bulk_transfer_t usb1_write = (libusb_bulk_transfer_t)lusb_api.resolve("libusb_bulk_transfer");
    if(!usb1_write)
        return NULL;

    if(_devh == NULL) {
        return 0;
    }

    qint tranfserred = 0;
    if (usb1_write(_devh, out_address, (quint8*)input.data(), input.length(), &tranfserred, 1000))
    {
        throw (QString("Write to usbdevice error"));
    }

    input.clear();
    return tranfserred;
}

quint32 MLibUSB::Read(qbyte &output, quint32 size)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    if(_devh == NULL) {
        return 0;
    }

    typedef int LIBUSB_CALL (*libusb_bulk_transfer_t)(libusb_device_handle *dev_handle,
                                                      unsigned char endpoint, unsigned char *data, int length,
                                                      int *actual_length, unsigned int timeout);
    libusb_bulk_transfer_t usb1_read = (libusb_bulk_transfer_t)lusb_api.resolve("libusb_bulk_transfer");
    if(!usb1_read)
        return NULL;

    int length = 0;
    int cant = size;

    if (cant == 0)
        size = read_size;

    int tries = 5;
    while (size)
    {
        usb1_read(_devh, in_address, (quint8*)output.data(), size, &length, 30000);
        size -= length;

        if (tries-- == 0)
        {
            if (cant != 0)
                throw(QString("usb read timeout, transferred: %1/%2").arg(cant-size).arg(cant));
            else
                break;
        }
    }

    return output.size();
}

bool MLibUSB::OpenUSBDevice()
{
    if(this->use_usbk)
    {
        qsizetype ret = 0;

        send_log_normal("Connection to libusb env...\n");

        this->_devh = qnull;
        if(lusb_api.isLoaded())
            lusb_api.unload();

        qInfo() << "MLibUSB::QLibrary:LoadLibusb1";

        QString LibPath = QString("%0/%1").arg(QDir::currentPath() , "Dynamics/usb/libusb-usbdk-1.0.dll");

        lusb_api.setFileName(LibPath);

        if(!lusb_api.load())
        {
            throw QString("MLibUSB::QLibrary:LoadLibusb1 fail error:%0")
                    .arg(lusb_api.errorString());

            return 0;
        }

        qInfo() << "MLibUSB::QLibrary:LoadLibusb1 success";

        if(!lusb_api.isLoaded())
            return 0;

        typedef int LIBUSB_CALL (*libusb_init_t)(libusb_context **ctx);
        libusb_init_t libusb1_init = (libusb_init_t)lusb_api.resolve("libusb_init");
        if(!libusb1_init)
            return NULL;

        typedef const char * LIBUSB_CALL (*libusb_error_name_t)(int errcode);
        libusb_error_name_t libusb1_error_name = (libusb_error_name_t)lusb_api.resolve("libusb_error_name");
        if(!libusb1_error_name)
            return NULL;

        typedef libusb_device_handle * LIBUSB_CALL (*usb1_open)(libusb_context *ctx, uint16_t vendor_id, uint16_t product_id);
        usb1_open usb1_open_dev = (usb1_open)lusb_api.resolve("libusb_open_device_with_vid_pid");
        if(!usb1_open_dev)
            return NULL;

        typedef libusb_device * LIBUSB_CALL (*usb1_get_dev)(libusb_device_handle *dev_handle);
        usb1_get_dev usb1_get_device = (usb1_get_dev)lusb_api.resolve("libusb_get_device");
        if(!usb1_get_device)
            return NULL;

        typedef libusb_device * LIBUSB_CALL (*usb1_get_cfg_desc)(libusb_device *dev,
                                                                 struct libusb_config_descriptor **config);
        usb1_get_cfg_desc usb1_get_cfg = (usb1_get_cfg_desc)lusb_api.resolve("libusb_get_active_config_descriptor");
        if(!usb1_get_cfg)
            return NULL;

        typedef int LIBUSB_CALL (*libusb_claim_interface_t)(libusb_device_handle *dev,
                                                            int interface_number);
        libusb_claim_interface_t libusb_claim_ifc = (libusb_claim_interface_t)lusb_api.resolve("libusb_claim_interface");
        if(!libusb_claim_ifc)
            return NULL;

        typedef int LIBUSB_CALL (*libusb_control_transfer_t)(libusb_device_handle *dev_handle,
                                                             uint8_t request_type, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
                                                             unsigned char *data, uint16_t wLength, unsigned int timeout);
        libusb_control_transfer_t libusb_ctrl_transfer = (libusb_control_transfer_t)lusb_api.resolve("libusb_control_transfer");
        if(!libusb_ctrl_transfer)
            return NULL;

        typedef int LIBUSB_CALL (*libusb_bulk_transfer_t)(libusb_device_handle *dev_handle,
                                                          unsigned char endpoint, unsigned char *data, int length,
                                                          int *actual_length, unsigned int timeout);
        libusb_bulk_transfer_t libusb_bulk_inter_transfer = (libusb_bulk_transfer_t)lusb_api.resolve("libusb_bulk_transfer");
        if(!libusb_bulk_inter_transfer)
            return NULL;


        typedef int LIBUSB_CALL (*libusb_release_interface_t)(libusb_device_handle *dev,
                                                              int interface_number);
        libusb_release_interface_t libusb_release_ifc = (libusb_release_interface_t)lusb_api.resolve("libusb_release_interface");
        if(!libusb_release_ifc)
            return NULL;

        typedef void LIBUSB_CALL (*libusb_close_t)(libusb_device_handle *dev_handle);
        libusb_close_t libusb_close_dev = (libusb_close_t)lusb_api.resolve("libusb_close");
        if(!libusb_close_dev)
            return NULL;

        typedef int LIBUSB_CALL (*libusb_exit_t)(libusb_context **ctx);
        libusb_exit_t libusb1_exit = (libusb_exit_t)lusb_api.resolve("libusb_exit");
        if(!libusb1_exit)
            return NULL;

        send_log_normal("Connection to libusb device...\n");

        ret = libusb1_init(NULL);
        if (ret < 0) {
            qDebug()<< "Error initializing libusb:"<< libusb1_error_name(ret);
            return false;
        }
        qDebug()<< "initializing libusb success! "<< libusb1_error_name(ret);

        _devh = usb1_open_dev(0, 0x0e8d, 0x3);
        if (!_devh) {
            send_log_fail(QString("libusb device open fail :%0").arg(libusb1_error_name(ret)));
            return false;
        }
        qDebug()<< "open libusbdevice  success! "<< libusb1_error_name(ret);

        libusb_device *usb_device = usb1_get_device(_devh);
        libusb_config_descriptor* desc;
        usb1_get_cfg(usb_device, &desc);
        for(int i = 0; i < desc->bNumInterfaces; i++) {
            const auto Interface = desc->Interface[i];
            for(int s = 0; s < Interface.num_altsetting; s++) {
                if(Interface.altsetting[s].bInterfaceClass != LIBUSB_CLASS_DATA)
                    continue;
                for(int e = 0; e < Interface.altsetting[s].bNumEndpoints; e++) {
                    const auto endpoint = Interface.altsetting[s].endpoint[e];
                    if(endpoint.bmAttributes & LIBUSB_TRANSFER_TYPE_BULK) {
                        if((endpoint.bEndpointAddress & LIBUSB_ENDPOINT_IN) == LIBUSB_ENDPOINT_IN) {
                            in_address = endpoint.bEndpointAddress;
                            read_size = endpoint.wMaxPacketSize;
                            qDebug()<< " in_address" << in_address << "read_size" << read_size;
                        } else if((endpoint.bEndpointAddress & LIBUSB_ENDPOINT_OUT) == LIBUSB_ENDPOINT_OUT) {
                            out_address = endpoint.bEndpointAddress;
                            write_size = endpoint.wMaxPacketSize;
                            qDebug()<< " out_address" << out_address << "write_size" << write_size;
                        }
                    }
                }
            }
        }

        //        if(libusb_kernel_driver_active(_devh, 0) == 1)
        //        {
        //            ret = libusb_detach_kernel_driver(_devh, 0);
        //            qInfo() << (QString("libusb_detach_kernel_driver0:%0").arg(libusb_error_name(ret)));
        //        }
        //        if(libusb_kernel_driver_active(_devh, 1) == 1)
        //        {
        //            ret = libusb_detach_kernel_driver(_devh, 1);
        //            qInfo() << (QString("libusb_detach_kernel_driver1:%0").arg(libusb_error_name(ret)));
        //        }

        ret = libusb_claim_ifc(_devh, 0);
        qInfo() << (QString("libusb_claim_interface0:%0").arg(libusb1_error_name(ret)));
        ret = libusb_claim_ifc(_devh, 1);
        qInfo() << (QString("libusb_claim_interface1:%0").arg(libusb1_error_name(ret)));

        send_log_red("libusb device open success!");
        return 1;
    }
    else
    {

        this->hnd = nullptr;
        if(lusb_api.isLoaded())
            lusb_api.unload();

        QString LibPath;
        if(is_64bit())
            LibPath = QString("%0/%1").arg(QDir::currentPath() , "Dynamics/usb/libusb0_x64.dll");
        else
            LibPath = QString("%0/%1").arg(QDir::currentPath() , "Dynamics/usb/libusb0_x86.dll");


        lusb_api.setFileName(LibPath);

        if(!lusb_api.load())
        {
            throw QString("MLibUSB::QLibrary:Load fail error:%0")
                    .arg(lusb_api.errorString());

            return false;
        }

        qInfo() << "MLibUSB::QLibrary:Load success";

        if(!lusb_api.isLoaded())
            return false;

        //
        bool found = false;
        struct usb_bus *bus;
        struct usb_device *dev;
        struct usb_dev_handle *handle = 0;

        usb_init();
        usb_find_busses();
        usb_find_devices();

        for (bus = usb_get_busses(); bus; bus = bus->next)
        {
            for (dev = bus->devices; dev; dev = dev->next)
            {
                if (dev->descriptor.idVendor == 0xe8d && dev->descriptor.idProduct == 0x3)
                {
                    found = true;
                    handle = usb_open(dev);
                    if (!handle)
                    {
                        qInfo().noquote().nospace() << qstr("MLibUSB::usb_open{:%0}").arg(usb_strerror());
                        return false;
                    }
                }
            }
        }

        if (!found)
        {
            send_log_red("libusb filter driver not detected");
            return false;
        }

        this->hnd = handle;
        return (found && this->hnd);
    }
}

bool MLibUSB::CloseUSBDevice(qbool reset)
{
    if(!lusb_api.isLoaded())
        return 0;

    if(this->use_usbk)
    {
        if(!_devh)
            return 0;

        typedef int LIBUSB_CALL (*libusb_release_interface_t)(libusb_device_handle *dev,
                                                              int interface_number);
        libusb_release_interface_t libusb_release_ifc = (libusb_release_interface_t)lusb_api.resolve("libusb_release_interface");
        if(!libusb_release_ifc)
            return NULL;

        typedef void LIBUSB_CALL (*libusb_close_t)(libusb_device_handle *dev_handle);
        libusb_close_t libusb_close_dev = (libusb_close_t)lusb_api.resolve("libusb_close");
        if(!libusb_close_dev)
            return NULL;

        typedef int LIBUSB_CALL (*libusb_exit_t)(libusb_context **ctx);
        libusb_exit_t libusb1_exit = (libusb_exit_t)lusb_api.resolve("libusb_exit");
        if(!libusb1_exit)
            return NULL;

        libusb_release_ifc(_devh, 0);
        libusb_release_ifc(_devh, 1);
        libusb_close_dev(_devh);
        libusb1_exit(NULL);

    }
    else
    {
        if(!this->hnd)
            return 0;

        qsizetype ret = 0;
        if(reset)
        {
            //            ret = usb_reset_ex(this->hnd, USB_RESET_TYPE_RESET_PORT);
            qInfo().noquote().nospace() << QString("MLibUSB::usb_reset{ret:%0:%1}").arg(QString::number(ret), usb_strerror());
        }

        ret = usb_close(this->hnd);
        qInfo().noquote().nospace() << QString("MLibUSB::usb_close{ret:%0:%1}").arg(QString::number(ret), usb_strerror());
        return (ret == 0);
    }
}

bool MLibUSB::is_64bit()
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

bool MLibUSB::RunProc(qstr programm, qstrl args, QByteArray &output)
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
            //            if (mCallback::ThreadTerminated())
            //            {
            //                runner->kill();
            //                throwException("terminated");
            //            }
        }
    }

    mutex.unlock();
    return output.size();
}

bool MLibUSB::filter_bootrom()
{
    return 1;
    append_log("filtering bootrom port to driver3...\n");

    QString tmp = QDir::currentPath() +"/Dynamics/tmp";
    QDir dir(tmp);
    if (!dir.exists())
        dir.mkpath(".");

    if(!dir.exists())
        return false;

    QString path = QString("%0").arg(dir.path());

    QFile infile;
    if (is_64bit())
        infile.setFileName(QString("%0/%1").arg(QDir::currentPath() , "Dynamics/drv/win_64"));
    else
        infile.setFileName(QString("%0/%1").arg(QDir::currentPath() , "Dynamics/drv/win_32"));

    if (!infile.open(QFile::ReadOnly))
    {
        send_log_red(QString("err1:%0\nPath%1:").arg(infile.errorString()).arg(infile.fileName()));
        return false;
    }

    QByteArray data = qUncompress(infile.readAll());
    infile.close();

    QString programm = QString("%0/usb_filter.exe").arg(path);
    QFile outfile(programm);
    if (!outfile.open(QFile::WriteOnly | QFile::Truncate))
    {
        send_log_red(QString("err1:%0\nPath:%1").arg(outfile.errorString()).arg(outfile.fileName()));
        return false;
    }

    outfile.write(data);
    outfile.close();

    QStringList usb_dev_list = {};
    QString bootrom_dev_path = {};

    //list
    {
        QStringList cmds = QStringList() << "list" << "--class=Ports";
        QByteArray output = {};
        if(!RunProc(programm, cmds, output))
        {
            throw QString("\nlibusb:list fail output:%1").arg(output.data());
            outfile.remove();
            return false;
        }

        qInfo() << QString("libusb:list success output:%1").arg(output.data());

        if(output.contains("libusb-win32 installer requires administrative privileges"))
        {
            throw QString("%0:\nplease run software as admin and try again").arg(output.data());
            outfile.remove();
            return false;
        }


        QTextStream st(output);
        while (!st.atEnd())
        {
            QString line = st.readLine();
            if (!line.contains("vid_0e8d&pid_0003", Qt::CaseInsensitive))
                continue;

            usb_dev_list << line;

        }

        if (usb_dev_list.isEmpty())
        {
            throw QString("Mediatek driver not detected #0");
            outfile.remove();
            return false;
        }

        for (QString line : usb_dev_list)
        {
            if (line.contains("&rev", Qt::CaseInsensitive))
            {
                QStringList llist = line.split(' ', QString::SkipEmptyParts);
                for (QString lline : llist)
                {
                    if (lline.startsWith("usb\\vid_", Qt::CaseInsensitive))
                        bootrom_dev_path = lline.trimmed();
                }
            }
        }
    }

    if (bootrom_dev_path.isEmpty())
    {
        throw QString("Mediatek driver not detected #1");
        outfile.remove();
        return false;
    }

    //install
    {
        QStringList cmds = QStringList() << "install" << QString("--device=%1").arg(bootrom_dev_path.replace("&","."));
        QByteArray output = {};
        if(!RunProc(programm, cmds, output))
        {
            throw QString("libusb:install fail output:%1").arg(output.data());
            outfile.remove();
            return false;
        }

        qInfo() << QString("libusb:install success output:%1").arg(output.data());
    }

    qInfo() << QString("libusb:bootrom filter installed successfully");

    //send_log_info(QString("MTK BootRom Filtered at %0").arg(bootrom_dev_path));

    outfile.remove();

    return true;

}

bool MLibUSB::LoadLibUSB()
{
    if(lusb_api.isLoaded())
        lusb_api.unload();

    QString LibPath;
    if(is_64bit())
        LibPath = QString("%0/%1").arg(QDir::currentPath() , "ASPT/mrx/lib/libusb0_x64.dll");
    else
        LibPath = QString("%0/%1").arg(QDir::currentPath() , "ASPT/mrx/lib/libusb0_x86.dll");


    lusb_api.setFileName(LibPath);

    if(!lusb_api.load())
    {
        qInfo() << QString("MLibUSB::QLibrary:Load fail error:%0")
                   .arg(lusb_api.errorString());

        return false;
    }

    qInfo() << "MLibUSB::QLibrary:Load success";

    return lusb_api.isLoaded();
}

bool MLibUSB::sendUsbTransfer(uint8_t value)
{
    append_log("Perform LibUsb control msg...");

    if(lusb_api.isLoaded())
        lusb_api.unload();

    QString LibPath;
    if(is_64bit())
        LibPath = QString("%0/%1").arg(QDir::currentPath() , "Dynamics/usb/libusb0_x64.dll");
    else
        LibPath = QString("%0/%1").arg(QDir::currentPath() , "Dynamics/usb/libusb0_x86.dll");


    lusb_api.setFileName(LibPath);

    if(!lusb_api.load())
    {
        throw QString("MLibUSB::QLibrary:Load fail error:%0")
                .arg(lusb_api.errorString());

        return false;
    }

    qInfo() << "MLibUSB::QLibrary:Load success";

    if(!lusb_api.isLoaded())
        return false;

    //
    bool found = false;
    struct usb_bus *bus;
    struct usb_device *dev;
    struct usb_dev_handle *handle = 0;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == 0xe8d && dev->descriptor.idProduct == 0x3)
            {
                found = true;
                handle = usb_open(dev);
                if (!handle)
                {
                    send_log_red(QString("err:%0").arg(QString("LIBUSB FILTER DRIVER NOT DETECTED").toLower()));
                    return false;
                }
                if (usb_control_msg(handle, 0xa1, 0, 0, value, 0, 0, 0) != 0)
                {
                    usb_close(handle);
                    {
                        send_log_red(QString("err:%0").arg(QString::fromUtf8((const char*)usb_strerror())));
                        return false;
                    }
                }
                qInfo() << "usb_control_msg ok ";
                usb_close(handle);
                break;
            }
        }
    }

    if (!found)
    {
        send_log_red(QString("err:%0").arg(QString("LIBUSB FILTER DRIVER NOT DETECTED").toLower()));
        return false;
    }

    send_log_red("passed!");
    return true;
}

usb_dev_handle *MLibUSB::usb_open(struct usb_device *dev)
{
    if(!lusb_api.isLoaded())
        return NULL;

    typedef usb_dev_handle* __stdcall (*QDLibCall)(struct usb_device *dev);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_open");
    if(!dll_call)
        return NULL;

    return dll_call(dev);
}

int MLibUSB::usb_close(usb_dev_handle *dev)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_close");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev);
}

int MLibUSB::usb_get_string(usb_dev_handle *dev, int index, int langid, char *buf,
                            size_t buflen)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, int index, int langid, char *buf, size_t buflen);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_get_string");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, index, langid, buf, buflen);
}

int MLibUSB::usb_get_string_simple(usb_dev_handle *dev, int index, char *buf,
                                   size_t buflen)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, int index, char *buf,
                                       size_t bufle);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_get_string_simple");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, index, buf, buflen);
}

int MLibUSB::usb_get_descriptor_by_endpoint(usb_dev_handle *udev, int ep,
                                            unsigned char type, unsigned char index,
                                            void *buf, int size)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *udev, int ep,
                                       unsigned char type, unsigned char index,
                                       void *buf, int size);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_get_descriptor_by_endpoint");
    if(!dll_call)
        return ENOFILE;

    return dll_call(udev, ep, type, index, buf, size);
}

int MLibUSB::usb_get_descriptor(usb_dev_handle *udev, unsigned char type,
                                unsigned char index, void *buf, int size)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *udev, unsigned char type,
                                       unsigned char index, void *buf, int size);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_get_descriptor");
    if(!dll_call)
        return ENOFILE;

    return dll_call(udev, type, index, buf, size);
}

int MLibUSB::usb_bulk_write(usb_dev_handle *dev, int ep, char *bytes, int size,
                            int timeout)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, int ep, char *bytes, int size,
                                       int timeout);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_bulk_write");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, ep, bytes, size, timeout);
}

int MLibUSB::usb_bulk_read(usb_dev_handle *dev, int ep, char *bytes, int size,
                           int timeout)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, int ep, char *bytes, int size,
                                       int timeout);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_bulk_read");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, ep, bytes, size, timeout);
}

int MLibUSB::usb_interrupt_write(usb_dev_handle *dev, int ep, char *bytes, int size,
                                 int timeout)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, int ep, char *bytes, int size,
                                       int timeout);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_interrupt_write");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, ep, bytes, size, timeout);
}

int MLibUSB::usb_interrupt_read(usb_dev_handle *dev, int ep, char *bytes, int size,
                                int timeout)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, int ep, char *bytes, int size,
                                       int timeout);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_interrupt_read");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, ep, bytes, size, timeout);
}

int MLibUSB::usb_control_msg(usb_dev_handle *dev, int requesttype, int request,
                             int value, int index, char *bytes, int size,
                             int timeout)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, int requesttype, int request,
                                       int value, int index, char *bytes, int size,
                                       int timeout);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_control_msg");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, requesttype, request, value, index, bytes, size, timeout);
}

int MLibUSB::usb_set_configuration(usb_dev_handle *dev, int configuration)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, int configuration);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_set_configuration");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, configuration);
}

int MLibUSB::usb_claim_interface(usb_dev_handle *dev, int interface)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, int interface);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_claim_interface");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, interface);
}

int MLibUSB::usb_release_interface(usb_dev_handle *dev, int interface)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, int interface);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_release_interface");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, interface);
}

int MLibUSB::usb_set_altinterface(usb_dev_handle *dev, int alternate)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, int alternate);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_set_altinterface");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, alternate);
}

int MLibUSB::usb_resetep(usb_dev_handle *dev, unsigned int ep)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, unsigned int ep);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_resetep");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, ep);
}

int MLibUSB::usb_clear_halt(usb_dev_handle *dev, unsigned int ep)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, unsigned int ep);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_clear_halt");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, ep);
}

int MLibUSB::usb_reset(usb_dev_handle *dev)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_reset");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev);
}

int MLibUSB::usb_reset_ex(usb_dev_handle *dev, unsigned int reset_type)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, unsigned int reset_type);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_reset_ex");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, reset_type);
}

char *MLibUSB::usb_strerror(void)
{
    if(!lusb_api.isLoaded())
        return NULL;

    typedef char* __stdcall (*QDLibCall)(void);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_strerror");
    if(!dll_call)
        return NULL;

    return dll_call();
}

void MLibUSB::usb_init(void)
{
    if(!lusb_api.isLoaded())
        return;

    typedef void __stdcall (*QDLibCall)(void);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_init");
    if(!dll_call)
        return;

    return dll_call();
}

void MLibUSB::usb_set_debug(int level)
{
    if(!lusb_api.isLoaded())
        return;

    typedef void __stdcall (*QDLibCall)(int level);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_set_debug");
    if(!dll_call)
        return;

    return dll_call(level);
}

int MLibUSB::usb_find_busses(void)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(void);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_find_busses");
    if(!dll_call)
        return ENOFILE;

    return dll_call();
}

int MLibUSB::usb_find_devices(void)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(void);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_find_devices");
    if(!dll_call)
        return ENOFILE;

    return dll_call();
}

struct usb_device *MLibUSB::usb_device(usb_dev_handle *dev)
{
    if(!lusb_api.isLoaded())
        return NULL;

    typedef struct usb_device* __stdcall (*QDLibCall)(usb_dev_handle *dev);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_device");
    if(!dll_call)
        return NULL;

    return dll_call(dev);
}

struct usb_bus *MLibUSB::usb_get_busses(void)
{
    if(!lusb_api.isLoaded())
        return NULL;

    typedef struct usb_bus* __stdcall (*QDLibCall)(void);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_get_busses");
    if(!dll_call)
        return NULL;

    return dll_call();
}

int MLibUSB::usb_install_service_np(void)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(void);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_install_service_np");
    if(!dll_call)
        return ENOFILE;

    return dll_call();
}

int MLibUSB::usb_uninstall_service_np(void)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(void);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_uninstall_service_np");
    if(!dll_call)
        return ENOFILE;

    return dll_call();
}

int MLibUSB::usb_install_driver_np(const char *inf_file)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(const char *inf_file);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_install_driver_np");
    if(!dll_call)
        return ENOFILE;

    return dll_call(inf_file);
}

const struct usb_version *MLibUSB::usb_get_version(void)
{
    if(!lusb_api.isLoaded())
        return NULL;

    typedef struct usb_version* __stdcall (*QDLibCall)(void);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_get_version");
    if(!dll_call)
        return NULL;

    return dll_call();
}

int MLibUSB::usb_isochronous_setup_async(usb_dev_handle *dev, void **context,
                                         unsigned char ep, int pktsize)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, void **context,
                                       unsigned char ep, int pktsize);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_isochronous_setup_async");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, context, ep, pktsize);
}

int MLibUSB::usb_bulk_setup_async(usb_dev_handle *dev, void **context,
                                  unsigned char ep)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, void **context,
                                       unsigned char ep);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_bulk_setup_async");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, context, ep);
}

int MLibUSB::usb_interrupt_setup_async(usb_dev_handle *dev, void **context,
                                       unsigned char ep)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(usb_dev_handle *dev, void **context,
                                       unsigned char ep);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_interrupt_setup_async");
    if(!dll_call)
        return ENOFILE;

    return dll_call(dev, context, ep);
}

int MLibUSB::usb_submit_async(void *context, char *bytes, int size)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(void *context, char *bytes, int size);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_submit_async");
    if(!dll_call)
        return ENOFILE;

    return dll_call(context, bytes, size);
}

int MLibUSB::usb_reap_async(void *context, int timeout)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(void *context, int timeout);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_reap_async");
    if(!dll_call)
        return ENOFILE;

    return dll_call(context, timeout);
}

int MLibUSB::usb_free_async(void **context)
{
    if(!lusb_api.isLoaded())
        return ENOFILE;

    typedef int __stdcall (*QDLibCall)(void **context);
    QDLibCall dll_call = (QDLibCall)lusb_api.resolve("usb_free_async");
    if(!dll_call)
        return ENOFILE;

    return dll_call(context);
}
