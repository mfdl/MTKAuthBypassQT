#ifndef BOOTROM_H
#define BOOTROM_H

#include <m_defs.h>
#include "m_controller.h"
#include "iostream"
#include <windows.h>
#include <stdint.h>
#include <sys/types.h>
#include <crtdefs.h>
#include "usbdevfinder.h"

#define ME_ID_BUFFER_LEN 16
#define MAX_BUF_LEN 4096

class boot_rom
{
public:
    boot_rom()
    {
        _hcom = NULL;
    };

    ~boot_rom() {};

    boot_rom(const boot_rom&) = delete;
    boot_rom(boot_rom&&) = delete;

    boot_rom& operator=(const boot_rom&) = delete;
    boot_rom& operator=(boot_rom&&) = delete;

    bool bypass_auth();
    bool crash_preloader(bool prl = 0, qbool disable_auth = 1);
    bool connect_mtk_device();

    bool brom_set_sec_cfg(quint16 hw_code, bool secure, bool dump_pl);
private:
    bool SendDA(quint32 addr, quint32 da_len, quint32 sig_len, QByteArray da, quint16 &checksum);
    bool JumpDa(quint32 addr);

    bool brom_write8_echo(quint8 cmd);
    bool brom_write32_echo(quint32 cmd);
private:
public:
    QSharedPointer<mThreadOPT> get_opt() const;
    QSharedPointer<mThreadOPT> opt_;
    virtual void set_opt(const QSharedPointer<mThreadOPT> &opt);
public:
    bool bypass_auth_new(int comport);
public:
    bool read_finish = false;
    void* ext; //to put some data special with OS.
    void disconnect_brom();
    bool connect_brom(int port_num);
    HANDLE _hcom;
    int ret = 0;
    //return actual read bytes. timeout or cancel throw exception.
    size_t brom_get(uint8_t* data, size_t length, quint32 timeout);
    //return actual send bytes. timeout or cancel throw exception.
    size_t brom_send(uint8_t* data, size_t length, quint32 timeout);

    QString get_u8(quint8 buff);
    QString get_u16(quint16 buff);
    QString get_u32(quint32 buff);
    QString get_hex(quint8 *buff);
    QString get_u8_hex(quint8 buff);
    bool echo_data(quint32 in, int size);
    QString GetChipID(quint16 hwcode);
    quint16 da_read16(quint32 addr);
    bool find_mtk_device(qint &port_num, bool &is_pl);
    bool BRom_WriteCmd32(quint32 addr, quint32 val);
    bool BRom_ReadCmd32(quint32 addr, quint32 val, quint8 read = 0, quint32 len = 0, bool no_ack = 0);
    bool readack(qsizetype ret, bool brom);
    QByteArray GetLow16(quint16 value);
    quint8 GetNum(quint8 val);
    qbyte GetBytesFromInt(qsizetype input);
    quint32 GetIntFromBytes(qbyte input);
private:
    quint32 timeout;
    BOOL is_bootloader;
private:
    int read8(quint8* data);
    int read16(quint16* data);
    int read32(quint32* data);
    int write8(quint8 data);
    int write16(quint16 data);
    int write32(quint32 data, bool type = 0);
    int write_buffer(quint8* data, quint32 length);
    int read_buffer(quint8* data, quint32 length);

    int write_ptr(char *data, quint32 length);
    int read_ptr(char *data, quint32 length);
    int write_pattern(QByteArray buff);
    int read_pattern(QByteArray &buff, quint32 length);

    int bootrom_sv5_read_ptr(QByteArray data, quint32 length);

    bool BRom_StartCmd();
    bool brom_start_cmd();
    bool get_chip_info(quint16 &hwcode);
    bool get_extra_info();
    bool get_security_config(bool &is_secure);
protected:
    bool set_boot_mode(quint16 b_mode);
    //*********************************************************************************************
    //NEW EXPLOIT
    qbool BRomRegAccess(Config &info, quint addr, quint len, qbyte &data, qbool check = 1);
    qbool BRomDaReadWrite(Config &info, quint addr, quint len, qbyte &data, qbool check = 1);
    qbool BRomDaRead(Config &info, quint addr, quint len, qbyte &data, qbool check = 1);
    qbool BRomDaWrite(Config &info, quint addr, quint len, qbyte &data, qbool check = 1);
	qbool mtk_new_expolit(Config &info, qbool dump_emi = 0);
public:
    bool dump_preloader();
    qbool UsbSendDevCtrl(qchar req_type, qchar req, qshort val, qshort idx, qbyte &buff);
    qbool BRomUpdateLineCoding(quint addr);
    qbool LibUsbFilterDriver(qshort vid, qshort pid);
public:
    qvoid vivo_remove_demo();
    QString SendATCmd(QByteArray input);
    quint8 FindPorts(QString &pinfo);
public:
    qvoid reboot_meta_mode();
};

#endif // BOOTROM_H
