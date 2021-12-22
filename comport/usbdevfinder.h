#ifndef USBFinder_H
#define USBFinder_H

#include <m_defs.h>
#include <devguid.h>
#include <dbt.h>
#include <setupapi.h>
#include <QUuid>

namespace USBCore
{
typedef struct USBDevInfo
{
    QVector<QUuid> guids;
    QVector<qstr> filter;
    qlong timeout;
    qstr notihnd;
    qstr BootMode;
    qstr PortName;
    qstr Manufacturer;
    qstr USBDriver;
    qstr DriverVersion;
    qstr DriverDate;
    qint PortNum;
} USBDevInfo;

class USBFinder
{
public:
    USBFinder();
    ~USBFinder() ;
    USBFinder(const USBFinder&) = delete;
    USBFinder(USBFinder&&) = delete;
    USBFinder& operator=(const USBFinder&) = delete;
    USBFinder& operator=(USBFinder&&) = delete;
public:
    qbool WaitForUsbDevice(USBDevInfo &devinfo);
private:
    qbool FindUsbDevice(USBDevInfo &info);
    qbool getDevProp(HDEVINFO hDevInfo, SP_DEVINFO_DATA sinfo, qdword prop, qstr &result);
private:
    static HRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    qbool OnDeviceChange(WPARAM wParam, LPARAM lParam);
    ATOM RegisterWindowClass(USBDevInfo &devinfo);
private:
    USBDevInfo devinfo;
    HWND w_hnd;
    qbool found;
protected:
    mutable QMutex m_mutex;
    static USBFinder *Instance();
private:
    virtual HWND whnd() const final
    {
        return this->w_hnd;
    }
private:
    void stop();
    static USBFinder *instance_;
};
}

#endif // USBFinder_H
