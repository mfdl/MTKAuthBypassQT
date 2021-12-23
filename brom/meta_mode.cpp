#include "boot_rom.h"
#include "boot_rom_cmd.h"
#include "usb_defs.h"
#include "m_callback.h"
#include <QSerialPortInfo>

qvoid boot_rom::reboot_meta_mode()
{
    LibUsbFilterDriver(0xe8d, 0x3);

    qbool is_pl = 0;
    qint portnumber = {};
    if(!find_mtk_device(portnumber, is_pl))
        return;

    send_log_normal("Connecting to BootROM......\n");

    if (!connect_brom(portnumber))
        return;

    if(!brom_start_cmd())
        return;

    quint16 hcode = 0;
    if(!get_chip_info(hcode))
        return;

    if(is_pl)
    {
        crash_preloader();
    }

    get_extra_info();

    bool secure = 0;
    if(!get_security_config(secure))
        return;

    bool ok = brom_set_sec_cfg(hcode, secure, 0);

    if(ok)
    {
        send_log_succes("Disable security success!");
    }
    else
    {
        send_log_fail("Disable security failed!");
        disconnect_brom();
        return;
    }

    send_log_normal("Connecting to BootROM...\n");

    if(!brom_start_cmd())
        return;

    send_log_normal("Send PARA_v1 regs...\n");

    if(!brom_write8_echo(0xDA))
        return;
    if(!write32(3, 1))
        return;
    if(!write32(0, 1))
        return;
    if(!write32(1, 1))
        return;
    quint16 ack = {};
    read16(&ack);
    if(!this->write8(1))
        return;
    read16(&ack);

    if(!brom_write8_echo(0xDA))
        return;
    if(!write32(2, 1))
        return;
    if(!write32(0, 1))
        return;
    if(!write32(1, 1))
        return;
    read16(&ack);
    if(!this->write8(1))
        return;
    read16(&ack);

    send_log_normal("Switch to meta mode...\n");

    brom_write8_echo(BROM_CMD_JUMP_BL);
    read16(&ack);

    disconnect_brom();

    send_log_succes("set META_BOOT mode success!");
}
