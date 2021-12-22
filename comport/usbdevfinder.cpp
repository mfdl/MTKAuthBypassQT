#include "usbdevfinder.h"
#include "m_callback.h"
#include <QRandomGenerator>

#include "ui_mainui.h"
#include "mainui.h"

using namespace USBCore;

USBFinder *USBFinder::instance_ = qnull;

USBFinder *USBFinder::Instance()
{
    return instance_;
}

USBFinder::USBFinder()
{
    qInfo().nospace() << "ctor of " << this;

    instance_  =  this;
    if(whnd())
        this->w_hnd = qnull;
}

USBFinder::~USBFinder()
{
    qInfo().nospace() << "dtor of " << this;
    stop();
}

qbool USBFinder::OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
    qbool found = 0;
    if (wParam == DBT_DEVICEARRIVAL)
    {
        qInfo() << "usb device inserted.";
        PDEV_BROADCAST_DEVICEINTERFACE iface = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
        if (!iface)
            return 0;
        switch(iface->dbcc_devicetype )
        {
            case DBT_DEVTYP_DEVICEINTERFACE:
            {
                QString devPath = QString::fromStdWString((LPCTSTR)iface->dbcc_name);
                qInfo() << "devPath " << devPath;

                found = FindUsbDevice(this->devinfo);
                qInfo() << "usb device inserted." << (bool)found;
            } break;
        }
    }
    else if(DBT_DEVICEREMOVECOMPLETE == wParam)
    {
        qInfo() << "usb device removed.";
    }

    return found;
}

HRESULT CALLBACK USBFinder::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DEVICECHANGE:
            if(USBFinder::Instance()->OnDeviceChange(wParam, lParam))
            {
                USBFinder::Instance()->found = true;
                PostQuitMessage(0);
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

ATOM USBFinder::RegisterWindowClass(USBDevInfo &devinfo)
{
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= &USBFinder::WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= static_cast<HINSTANCE>(GetModuleHandle(0));
    wcex.hIcon			= NULL;
    wcex.hCursor		= NULL;
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= reinterpret_cast<LPCWSTR>(devinfo.notihnd.utf16());
    wcex.hIconSm		= NULL;

    return RegisterClassEx(&wcex);
}

qbool USBFinder::WaitForUsbDevice(USBDevInfo &devinfo)
{
    qInfo().noquote() << "USBFinder::WaitForUsbDevice";

    if (!RegisterWindowClass(devinfo))
        return 0;

    const wchar_t *win_name = reinterpret_cast<LPCWSTR>(devinfo.notihnd.utf16());
    this->w_hnd = CreateWindow(win_name
                               , L"T", WS_OVERLAPPEDWINDOW
                               , CW_USEDEFAULT, 0
                               , CW_USEDEFAULT, 0, NULL, NULL
                               , GetModuleHandle(NULL), NULL);

    if (!whnd())
        return 0;

    this->devinfo = devinfo;
    this->found = 0;

    HDEVNOTIFY hDevNotify = {0};
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    for (QVector<QUuid>::iterator it = devinfo.guids.begin();
         it !=  devinfo.guids.end(); ++it)
    {
        const GUID &dev_uid = *it;
        NotificationFilter.dbcc_classguid = dev_uid;
        hDevNotify = RegisterDeviceNotification(whnd(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
        if(!hDevNotify)
        {
            qInfo().noquote() << "Can't register device notification: ";
            return 0;
        }
    }


    QElapsedTimer timer;
    timer.start();

    MSG msg = {};
    while (true)
    {
        if(!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // there weren't any messages
        }
        else
        {
            if (msg.message == WM_QUIT)
                break;
        }

        if((timer.elapsed() == this->devinfo.timeout)
                || mCallback::get_instance()->maingui()->Loader()->isStop())
            stop();

        //QColor color = QColor::fromRgb(QRandomGenerator::global()->bounded(0 + timer.elapsed(), 250));
        auto elapsed = (timer.elapsed() / 1000.0)*2;
        emit mCallback::get_instance()->signal_set_prg_val(elapsed);

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterDeviceNotification(hDevNotify);

    qbool device_found = USBFinder::Instance()->found;
    devinfo = this->devinfo;
    this->found = device_found;
    return device_found;
}

void USBFinder::stop()
{
    qInfo().noquote() << "USBFinder::stop";

    if(whnd())
    {
        SendMessage(whnd(), WM_DESTROY, 0, 0);
    }
}

bool USBFinder::getDevProp(HDEVINFO hDevInfo, SP_DEVINFO_DATA devInfo, qdword prop, qstr &result)
{
    qdword size = 0;
    ::SetupDiGetDeviceRegistryProperty(hDevInfo, &devInfo, prop, qnull, qnull, qzero, &size);
    qstr buff(qint(size / sizeof(wchar_t)), Qt::Uninitialized);
    if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devInfo, prop, 0, (qchar*)buff.data(), size, qnull))
    {
        if (buff.contains(QChar::Null))
            result = buff.split(QChar::Null).at(0).simplified();

        if (buff.contains(QStringLiteral("\\0022")))
            result = buff.split(QStringLiteral("\\0022")).at(0).simplified();
    }

    return result.size();
}

qbool USBFinder::FindUsbDevice(USBDevInfo &devinfo)
{
    qInfo().noquote() <<__PRETTY_FUNCTION__;

    const GUID guid = devinfo.guids.at(0); //GUID_DEVCLASS_PORTS
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&guid, 0, 0, DIGCF_PRESENT);
    if (hDevInfo == qnull_hnd)
        return 0;

    SP_DEVINFO_DATA spDevInfo = {0};
    for (qdword idx = 0;; idx++)
    {
        ZeroMemory(&spDevInfo, sizeof(SP_DEVINFO_DATA));
        spDevInfo.cbSize = sizeof(SP_DEVINFO_DATA);
        if (!SetupDiEnumDeviceInfo(hDevInfo, idx, &spDevInfo))
            break;

        qstr hwid = {};
        if (!getDevProp(hDevInfo, spDevInfo, SPDRP_HARDWAREID, hwid))
            continue;

        qbool found = 0;
        for (QVector<qstr>::iterator it = devinfo.filter.begin();
             it !=  devinfo.filter.end(); ++it)
        {
            const qstr &filter = *it;
            found = hwid.startsWith(filter, Qt::CaseInsensitive);
            if (found)
                break;
        }

        if (found)
        {
            getDevProp(hDevInfo, spDevInfo, SPDRP_FRIENDLYNAME, devinfo.PortName);
            getDevProp(hDevInfo, spDevInfo, SPDRP_MFG, devinfo.Manufacturer);
            getDevProp(hDevInfo, spDevInfo, SPDRP_SERVICE, devinfo.USBDriver);

            if (devinfo.PortName.contains("Preloader", Qt::CaseInsensitive))
                devinfo.BootMode = "BootLoader";
            else if(devinfo.PortName.contains("Lg", Qt::CaseInsensitive))
                devinfo.BootMode = "BootLoader";
            //                devinfo.BootMode = "LG Preloader";
            else if(devinfo.PortName.contains("USB Port", Qt::CaseInsensitive))
                devinfo.BootMode = "BootRom";

            SP_DRVINFO_DATA spDrvInfo = {0};
            SP_INTERFACE_DEVICE_DATA spDrvInfoDet = {0};
            spDrvInfo.cbSize = sizeof(SP_DRVINFO_DATA);
            if (::SetupDiBuildDriverInfoList(hDevInfo, &spDevInfo, SPDIT_COMPATDRIVER))
            {
                if (SetupDiEnumDriverInfo(hDevInfo, &spDevInfo, SPDIT_COMPATDRIVER, 0, &spDrvInfo))
                {
                    unsigned high = (unsigned)(spDrvInfo.DriverVersion >> 32);
                    unsigned low = (unsigned)(spDrvInfo.DriverVersion & 0xffffffffULL);
                    devinfo.DriverVersion = qstr().sprintf("%d.%d.%d.%d",
                                                           (high >> 16),
                                                           (high & 0xffffu),
                                                           (low >> 16),
                                                           (low & 0xffffu));

                    SYSTEMTIME time = {0};
                    FileTimeToSystemTime(&spDrvInfo.DriverDate, &time);
                    devinfo.DriverDate = qstr().sprintf("%02d/%02d/%04d", time.wMonth, time.wDay, time.wYear);
                }
            }
            ::SetupDiDestroyDriverInfoList(hDevInfo, &spDevInfo, SPDIT_COMPATDRIVER);
        }

        QRegExp regex("COM(\\d+)");
        if(regex.indexIn(devinfo.PortName)!= -1)
            devinfo.PortNum = regex.cap(1).toInt();
    }

    ::SetupDiDestroyDeviceInfoList(hDevInfo);
    return devinfo.PortNum ;
}
