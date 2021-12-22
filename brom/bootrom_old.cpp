#include "boot_rom.h"
#include "boot_rom_cmd.h"
#include "meta_cmd.h"
#include "m_callback.h"

QString boot_rom::get_u8(quint8 buff)
{
    return QString("%0").arg(buff, sizeof(quint8)*2, 16, QLatin1Char('0'));
}
QString boot_rom::get_u8_hex(quint8 buff)
{
    bool is_num = false;
    QByteArray hex = QByteArray::fromHex(QString("%0").arg(buff, sizeof(quint8)*2, 16, QLatin1Char('0')).toUtf8()).toHex();
    auto value = hex.toDouble(&is_num);
    if(is_num)
        return QByteArray::fromHex(hex);

    return hex;
}
QString boot_rom::get_u16(quint16 buff)
{
    return QString("%0").arg(buff, sizeof(quint16)*2, 16, QLatin1Char('0'));
}
QString boot_rom::get_u32(quint32 buff)
{
    return QString("%0").arg(buff, sizeof(quint32)*2, 16, QLatin1Char('0'));
}
QString boot_rom::get_hex(quint8* buff)
{
    QByteArray arry = QByteArray::fromRawData((char*)buff, strlen((char*)buff)).toHex().toUpper();
    return QString("%0").arg(arry.data());
}

QString mCallback::GetMTKChipName(QString chip_id)
{
    // Dimensity //
    if(chip_id.startsWith("MT688"))
        return "Dimensity 800/1000/1000L/1000+/1000C";
    if(chip_id.startsWith("MT6891Z/MT6893"))
        return "Dimensity 1100/1200";
    if(chip_id.startsWith("MT6889Z"))
        return "Dimensity 1000+";
    if(chip_id.startsWith("MT6889"))
        return "Dimensity 1000";
    if(chip_id.startsWith("MT6885Z"))
        return "Dimensity 1000L";
    if(chip_id.startsWith("MT6883Z"))
        return "Dimensity 1000C";
    if(chip_id.startsWith("MT6875"))
        return "Dimensity 820";
    if(chip_id.startsWith("MT6853V"))
        return "Dimensity 800U";
    if(chip_id.startsWith("MT6873"))
        return "Dimensity 800";
    if(chip_id.startsWith("MT6853"))
        return "Dimensity 720/800U 5G";
    if(chip_id.startsWith("MT6833"))
        return "Dimensity 700";

    if(chip_id.startsWith("MT6877"))
        return "Dimensity 900 5G";
    // Helio //
    if(chip_id.startsWith("MT6761D"))
        return "Helio A20";
    if(chip_id.startsWith("MT6761"))
        return  "Helio A22";
    if(chip_id.startsWith("MT6762D"))
        return "Helio A25";
    if(chip_id.startsWith("MT6762G"))
        return "Helio G25";
    if(chip_id.startsWith("MT6765G"))
        return "Helio G35";
    if(chip_id.startsWith("MT6769V"))
        return "Helio G70";
    if(chip_id.startsWith("MT6769T"))
        return "Helio G80";
    if(chip_id.startsWith("MT6769Z"))
        return "Helio G85";
    if(chip_id.startsWith("MT6785V"))
        return "Helio G90";
    if(chip_id.startsWith("MT6785V"))
        return "Helio G90T";
    if(chip_id.startsWith("MT6785V"))
        return "Helio G95";
    if(chip_id.startsWith("MT6755"))
        return  "Helio P10";
    if(chip_id.startsWith("MT6755M"))
        return "Helio P10 M";
    if(chip_id.startsWith("MT6755T"))
        return "Helio P15";
    if(chip_id.startsWith("MT6755S"))
        return "Helio P18";
    if(chip_id.startsWith("MT6757"))
        return "Helio P20";
    if(chip_id.startsWith("MT6762"))
        return  "Helio P22";
    if(chip_id.startsWith("MT6763V"))
        return "Helio P23";
    if(chip_id.startsWith("MT6763T"))
        return "Helio P23";
    if(chip_id.startsWith("MT6757CD"))
        return "Helio P25";
    if(chip_id.startsWith("MT6758"))
        return  "Helio P30";
    if(chip_id.startsWith("MT6765"))
        return  "Helio P35";
    if(chip_id.startsWith("MT6771"))
        return  "Helio P60";
    if(chip_id.startsWith("MT6768"))
        return  "Helio P65";
    if(chip_id.startsWith("MT6771T"))
        return "Helio P70";
    if(chip_id.startsWith("MT6771V"))
        return "Helio P70";
    if(chip_id.startsWith("MT6779"))
        return  "Helio P90";
    if(chip_id.startsWith("MT6779V"))
        return "Helio P90";
    if(chip_id.startsWith("MT6795"))
        return  "Helio X10";
    if(chip_id.startsWith("MT6795T"))
        return "Helio X10 T";
    if(chip_id.startsWith("MT6797"))
        return  "Helio X20";
    if(chip_id.startsWith("MT6797M"))
        return "Helio X20 M";
    if(chip_id.startsWith("MT6797D"))
        return "Helio X23";
    if(chip_id.startsWith("MT6797T"))
        return "Helio X25";
    if(chip_id.startsWith("MT6797X"))
        return "Helio X27";
    if(chip_id.startsWith("MT6799"))
        return "Helio X30";
    if(chip_id.startsWith("MT6799"))
        return "Helio X30";
    if(chip_id.startsWith("MT6781"))
        return "Helio G96";

    return QString();
}

QString mCallback::GetMTKChipEvol(QString chip_id)
{
    if(chip_id == "MT6799")
        return QString("Whitney");
    if(chip_id == "MT0598")
        return QString("ELBRUS");
    if(chip_id == "MT6757")
        return QString("olympus");
    if(chip_id == "MT6757D")
        return QString("kibo+");
    if(chip_id == "MT6759")
        return QString("Alaska");
    if(chip_id == "MT6570")
        return QString("rushmore");
    if(chip_id == "MT6763")
        return QString("Bianco");
    if(chip_id == "MT6758")
        return QString("vinson");
    if(chip_id == "MT6739")
        return QString("zion");
    if(chip_id == "MT6775")
        return QString("Cannon");
    if(chip_id == "MT6765")
        return QString("cervino");
    if(chip_id == "MT6771")
        return QString("Sylvia");
    if(chip_id == "MT8518")
        return QString("Weber");
    if(chip_id == "MT3967")
        return QString("Eiger");
    if(chip_id == "MT6761")
        return QString("Merlot");
    if(chip_id == "MT6779")
        return QString("Lafite");
    if(chip_id == "MT8168")
        return QString("Aquila");
    if(chip_id == "MT6768")
        return QString("Talbot");
    if(chip_id == "MT6785")
        return QString("Krug");
    if(chip_id == "MT6885")
        return QString("Petrus");
    if(chip_id == "MT6873")
        return QString("Margaux");
    if(chip_id == "MT0992")
        return QString("Colgin");
    if(chip_id == "MT6853")
        return QString("Mouton");
    if(chip_id == "MT6893")
        return QString("Petrus_P");
    if(chip_id == "MT6833")
        return QString("Palmer");
    if(chip_id == "MT6877")
        return QString("Montrose");

    return QString();
}

QString boot_rom::GetChipID(quint16 hwcode)
{
    if(hwcode == 0x6261)
        return QString("MT6261");
    if(hwcode == 0x6572)
        return QString("MT6572");
    if(hwcode == 0x6580)
        return QString("MT6580");
    if(hwcode == 0x6582)
        return QString("MT6582");
    if(hwcode == 0x321)
        return QString("MT6735");
    if(hwcode == 0x335)
        return QString("MT6737");
    if(hwcode == 0x699)
        return QString("MT6739");
    if(hwcode == 0x326)
        return QString("MT6755");
    if(hwcode == 0x551)
        return QString("MT6757");
    if(hwcode == 0x717)
        return QString("MT6761");
    if(hwcode == 0x690)
        return QString("MT6763");
    if(hwcode == 0x766)
        return QString("MT6765");
    if(hwcode == 0x707)
        return QString("MT6768");
    if(hwcode == 0x788)
        return QString("MT6771");
    if(hwcode == 0x725)
        return QString("MT6779");
    if(hwcode == 0x813)
        return QString("MT6785");
    if(hwcode == 0x279)
        return QString("MT6797");
    if(hwcode == 0x562)
        return QString("MT6799");
    if(hwcode == 0x886)
        return QString("MT6873");
    if(hwcode == 0x8127)
        return QString("MT8127");
    if(hwcode == 0x8163)
        return QString("MT8163");
    if(hwcode == 0x8173)
        return QString("MT8173");
    if(hwcode == 0x8695)
        return QString("MT8695");
    if(hwcode == 0x950)
        return QString("MT6891Z/MT6893");
    if(hwcode == 0x989)
        return QString("MT6833");
    if(hwcode == 0x996)
        return QString("MT6853");
    if(hwcode == 0x816)
        return QString("MT6880/MT6883/MT6885/MT6889/MT6890");
    if(hwcode == 0x959)
        return QString("MT6877");
    if(hwcode == 0x1066)
        return QString("MT6781");

    return QString("0x%0").arg(get_u16(hwcode));
}

///**********************************************************************************************

bool boot_rom::BRom_StartCmd()
{
    qInfo() <<"boot_rom::BRom_StartCmd!";
    quint8 sync[] = {0xa0, 0x0a, 0x50, 0x05};
    quint8 cnt = sizeof(sync) / sizeof(sync[0]);
    try
    {
        for(quint8 idx = 0; idx < cnt;)
        {
            quint8 cmd = sync[idx];
            quint8 exp = ~cmd;
            quint8 ack = 0;
            quint8 cnt = 0;

            if(write8(cmd))
            {
                if(read8(&ack))
                {
                    qInfo() << QString("BRom_MT6276::BRom_StartCmd::send[%0]:read[%1]:expected[%2]:index[%3]:is_pl[%4]")
                               .arg(get_u8(cmd)).arg(get_u8_hex(ack))
                               .arg(get_u8(exp)).arg(get_u8(idx))
                               .arg(is_bootloader? "Yes" : "No");

                    if (ack == exp)
                    {
                        ++idx; //This cannot be a blocking read operation because we have a chance to reset the target and retry the whole synchronization flow.
                        cnt = 0;
                    }
                    else
                    {
                        idx = 0;
                        cnt++;
                        if((ack == 'R') && (idx == 0))//first sync char of preloader.
                        {
                            QByteArray ready = "EADY";
                            if(bootrom_sv5_read_ptr(ready, 4)){is_bootloader = true;}
                        }
                    }
                }
                else
                {
                    idx = 0;
                    cnt++;
                }
                if (cnt == 10)
                {
                    send_log_fail(QString("BRom_StartCmd::timeout{0xC0060001}"));
                    return false;
                }
            }
        }
    }
    catch (std::exception& e)
    {
        send_log_fail(QString("BRom_StartCmd::STATUS_BROM_CMD_STARTCMD_FAIL{0xC0060001}[%0]").arg(e.what()));
        return false;
    }

    qInfo() <<"boot_rom::BRom_StartCmd ok!";
    return true;
}

bool boot_rom::brom_start_cmd()
{
    qInfo() <<"boot_rom::brom_start_cmd!";

    quint8 cmd[4] = {0xA0, 0x0A, 0x50, 0x05};
    try
    {
        quint8 ack = 0;
        quint32 try_time = 0;
        for(quint32 i = 0; i < 4; ++i)
        {
            if(try_time > 600) //30sec = 50ms * 600
                return false;

            uchar exp = ~cmd[i];
            brom_send(&cmd[i], 1, timeout);
            try
            {
                brom_get(&ack, 1, 50);
            }
            catch (std::exception&)
            {
                --i;
                ++try_time;
                continue;
            }
            {
                qInfo() << QString("BRom_MT6276::BRom_StartCmd::send[%0]:read[%1]:expected[%2]:index[%3]:is_pl[%4]")
                           .arg(get_u8(cmd[i]))
                           .arg(get_u8_hex(ack))
                           .arg(get_u8(exp))
                           .arg(get_u8(i))
                           .arg(get_u8(is_bootloader));
            }
            if((ack == 'R') && (i == 0))//first sync char of preloader.
            {
                quint8 ready[8];
                brom_get(ready, 4, timeout); //receive the rest "EADY"
                --i;
                is_bootloader = TRUE;
                continue;
            }
            if((quint8)(~ack) != cmd[i])
            {
                send_log_red(QString("● BRom protocol error: ACK %0 != %1").arg(get_u8(ack), get_u8(cmd[i])));
                disconnect_brom();
                return false;
            }
        }
    }
    catch (std::exception& e)
    {
        send_log_fail(QString("boot_rom::connect_brom:error[%0]").arg(e.what()));
        qInfo() << "brom connect exception: ";
        qInfo() << e.what();
        return false;
    }

    qInfo() <<"boot_rom::connect_brom success!";

    return true;
}

bool boot_rom::get_chip_info(quint16 &hwcode)
{
    qInfo() <<"boot_rom::get_chip_info!";
    quint16 hw_code;
    quint16 hw_sub_code;
    quint16 hw_version;
    quint16 sw_version;

    quint8 cmd = BROM_CMD_GET_HW_CODE;
    quint8 ack = 0;
    write8(cmd);
    read8(&ack);
    if(cmd != ack)
    {
        send_log_fail(QString("get hw id error. return 0x%0").arg(get_u8(ack)));
        return false;
    }

    read16(&hw_code);
    quint16 status;
    read16(&status);

    cmd = BROM_CMD_GET_HW_SW_VER;

    write8(cmd);
    read8(&ack);
    if(cmd != ack)
    {
        send_log_fail(QString("get sw version error. return 0x%0").arg(get_u8(ack)));
        return false;
    }

    read16(&hw_sub_code);
    read16(&hw_version);
    read16(&sw_version);
    read16(&status);

    hwcode = hw_code;

    QString ChipInfo[4] = {};
    ChipInfo[0].append(QString("%0").arg(GetChipID(hwcode)));
    ChipInfo[1].append(QString("%0").arg(get_u16(hw_sub_code).toUpper()));
    ChipInfo[2].append(QString("%0").arg(get_u16(hw_version)).toUpper());
    ChipInfo[3].append(QString("%0").arg(get_u16(sw_version)).toUpper());

    QString chipname = ChipInfo[0];
    QString name = mCallback::GetMTKChipName(chipname);
    QString code = mCallback::GetMTKChipEvol(chipname);
    QString hsub = ChipInfo[1];
    QString hver = ChipInfo[2];
    QString sver = ChipInfo[3];

    if(chipname.size())
    {
        send_log_normal("● Chipset\t");
        send_log_info(QString("%0").arg(chipname));
    }
    if(name.size())
    {
        send_log_normal("● Identifier\t");
        send_log_red(QString("%0").arg(name));
    }

    send_log_normal("● Info\t");
    send_log_succes(QString("%0_%1_%2_%3").arg(hsub, hver, sver, code));

    qInfo() <<"boot_rom::get_chip_info success!";
    //set_boot_mode(0);
    return true;
}

bool boot_rom::get_extra_info()
{
    quint8 ack = 0;
    write8(0xe1);
    read8(&ack);
    if(ack != 0xe1)
        return 0;

    quint32 meid_len = 0;
    if(!read32(&meid_len))
        return 0;

    qInfo() <<"meid_len!" << meid_len;

    QByteArray meid;
    if(!read_pattern(meid, meid_len))
        return 0;

    qInfo() <<"meid!" << meid.toHex();

    quint16 status;
    read16(&status);
    qInfo() <<"read16!" << status;

    send_log_red(QString("● MEID:%0").arg(meid.toHex().toUpper().data()));

    return 1;
    {
        quint8 ack = 0;
        write8(0xe7);
        read8(&ack);
        if(ack != 0xe7)
            return 0;

        quint32 socid_len = 0;
        if(!read32(&socid_len))
            return 0;

        QByteArray soc_id = 0;
        if(!read_pattern(soc_id, socid_len))
            return 0;

        quint16 status;
        read16(&status);

        qstr socid0 = soc_id.mid(0, socid_len/2).toHex().toUpper();
        qstr socid1 = soc_id.mid(socid_len/2, socid_len/2).toHex().toUpper();

        send_log_succes(QString("● SOCID0:%0").arg(socid0));
        send_log_succes(QString("● SOCID1:%0").arg(socid1));
    }

    {
        quint8 ack = 0;
        write8(0xdf);
        read8(&ack);
        if(ack != 0xdf)
            return 0;

        quint32 log_len = 0;
        if(!read32(&log_len))
            return 0;

        QByteArray bromlog = 0;
        if(!read_pattern(bromlog, log_len))
            return 0;

        quint16 status;
        read16(&status);

        send_log_normal(QString("BootRom Logs:%0\n").arg(bromlog.simplified().data()));
    }

    qInfo() <<"get_extra_info ok!";

    return true;
}

bool boot_rom::get_security_config(bool &is_secure)
{
    is_secure = false;
    try
    {
        quint8 cmd = BROM_CMD_GET_TARGET_CONFIG;
        quint8 ack = 0;
        write8(cmd);
        read8(&ack);
        if(cmd != ack)
        {
            qInfo() << QString("BROM_CMD_GET_TARGET_CONFIG error. code 0x%0").arg(ack);
            return false;
        }

        quint32 cfg = 0;
        read32(&cfg);

        quint16 status;
        read16(&status);

        if(status >= E_ERROR)
        {
            qInfo() << QString("BROM_CMD_GET_TARGET_CONFIG error. code 0x%0").arg(status);
            return false;
        }

        BOOL SBC = (cfg & SEC_CFG_SBC_EN)?TRUE:FALSE;
        BOOL SLA = (cfg & SEC_CFG_SLA_EN)?TRUE:FALSE;
        BOOL SDA = (cfg & SEC_CFG_DAA_EN)?TRUE:FALSE;

        is_secure = bool(SLA || SDA);

        QString sec_level = {};
        if(!SBC && !SLA && !SDA)
            sec_level = "NON_SECURE";

        if(SBC && SLA && SDA)
            sec_level = "SBC+SLA+SDA";

        if(SBC && !SLA && !SDA)
            sec_level = "SBC";

        if(!SBC && SLA && !SDA)
            sec_level = "SLA";

        if(!SBC && !SLA && SDA)
            sec_level = "SDA";

        if(SBC && SLA && !SDA)
            sec_level = "SBC+SLA";

        if(SBC && !SLA && SDA)
            sec_level = "SBC+SDA";

        if(!SBC && SLA && SDA)
            sec_level = "SLA+SDA";

        send_log_normal("● SecCFG\t");
        send_log_red(sec_level);

        //send_log_red(QString("● BR_SECURE:%0").arg(sec_level));

    }

    catch (std::exception& e)
    {
        qInfo() << "BROM_CMD_GET_TARGET_CONFIG:exception " << e.what();
    }

    return true;
}

bool boot_rom::set_boot_mode(quint16 b_mode)
{
    send_log_normal("● set_boot_mode\t");

    if(b_mode == 0)
    {
        quint8 ack = {};
        write8(0xda);
        read8(&ack);
        qInfo() << "send 0xda " << get_u32(ack);

        qInfo() << "BRom_JumpBLCmd " ;//<< jump_pl(0x22000000);
        write8(BROM_CMD_JUMP_BL);
        read8(&ack);
        if(BROM_CMD_JUMP_BL != ack)
            return false;
    }
    else
    {
        if(!write_pattern("METAMETA")) //PreloaderCmd::CMD_BootAsMETA():
            return false;

        //        para_t param;
        //        if (0 == brom_get((quint8*)&param.v1, sizeof(param.v1), timeout))
        //        {
        //           // g_meta_com_id = param.v1.usb_type;
        //            qInfo() << "param.v1.usb_type " << param.v1.usb_type << param.v1.usb_num << param.v1.header.ver << param.v1.header.len;
        //            qInfo() << "PreloaderCmd::CMD_BootAsMETA():  tValue.m_cKernelUSBType:1 (PreloaderCmd.cpp:336) & PreloaderCmd::CMD_BootAsMETA(): Need para V1!" ;
        //        }

        if(!write_pattern("0400000001000000010000C0")) //Send META parameters OK!
            return false;

        if(!write_pattern("0400000001000000010000C0"))
            return false;

        if(!write_pattern("0600000001000000010000C000800000"))
            return false;

        QByteArray read = {};
        if(!read_pattern(read, 13)) //!READYATEM0001 //Dump data size: 13 (PreloaderCmd.cpp:183)
            return false;

        if(!write_pattern("DISCONNECT")) //Need disconnect CMD!
            return false;
    }

}

