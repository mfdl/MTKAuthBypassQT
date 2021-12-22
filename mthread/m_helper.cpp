#include "m_helper.h"

int m_helper::read_data_from_file(QByteArray &data, QString path, qint64 offset, qint64 length)
{
    QFile file;
    QDataStream stream;

    const qint64 BUFF_SIZE = 1048576;

    qint64 read_len, ret;

    if (!QFileInfo(path).exists())
    {
        return -1;
    }

    file.setFileName(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return -1;
    }

    stream.setDevice(&file);

    if (!file.seek(offset))
    {
        file.close();
        return -1;
    }

    if (length == 0)
    {
        length = file.size() - offset;
    }

    if (offset + length > file.size())
    {
        file.close();
        return -1;
    }

    data.clear();

    while (length > 0)
    {
        read_len = qMin(BUFF_SIZE, length);

        char *tmp_data = reinterpret_cast<char*>(malloc(read_len));

        if (!tmp_data)
        {
            file.close();
            return -1;
        }

        ret = stream.readRawData(tmp_data, read_len);

        if (ret != read_len)
        {
            free(tmp_data);
            file.close();
            return -1;
        }

        data.append(tmp_data, ret);

        free(tmp_data);

        length -= read_len;
    }

    file.close();

    return 0;
}

uint64_t m_helper::str_to_bin(QString input)
{
    return str_to_num(input, 2);
}
uint64_t m_helper::str_to_oct(QString input)
{
    return str_to_num(input, 8);
}
uint64_t m_helper::str_to_dec(QString input)
{
    return str_to_num(input, 10);
}
uint64_t m_helper::str_to_hex(QString input)
{
    return str_to_num(input, 16);
}
uint64_t m_helper::str_to_hex(char *input, int count)
{
    QString data = QString::fromUtf8((const char*)input, count);
    return str_to_num(data, 16);
}

uint16_t m_helper::read_be16(const void *ptr)
{
    return ((const unalignA*)ptr)->u16;
}
uint32_t m_helper::read_be32(const void *ptr)
{
    return ((const unalignA*)ptr)->u32;
}

size_t m_helper::read_arch(const void *ptr)
{
    return ((const unalignA*)ptr)->uArch;
}
uint16_t m_helper::read_le16(const void *ptr)
{
    const uint8_t* p = (const uint8_t*)ptr;
    return (uint16_t)((uint16_t)p[0] + (p[1] << 8));
}
uint32_t m_helper::read_le32(const void *ptr)
{
    const uint8_t* p = (const uint8_t*)ptr;
    uint32_t value32 = p[0];

    value32 += (p[1] << 8);
    value32 += (p[2] << 16);
    value32 += (((uint32_t)p[3]) << 24);

    return value32;
}
uint64_t m_helper::read_le64(const void *ptr)
{
    const uint8_t* p = (const uint8_t*)ptr;
    uint64_t value64 = p[0];

    value64 += ((uint64_t)p[1]<<8);
    value64 += ((uint64_t)p[2]<<16);
    value64 += ((uint64_t)p[3]<<24);
    value64 += ((uint64_t)p[4]<<32);
    value64 += ((uint64_t)p[5]<<40);
    value64 += ((uint64_t)p[6]<<48);
    value64 += ((uint64_t)p[7]<<56);

    return value64;
}

void m_helper::write_be16(void *ptr, uint16_t value)
{
    ((unalignA*)ptr)->u16 = value;
}
void m_helper::write_be32(void *ptr, uint32_t value)
{
    ((unalignA*)ptr)->u32 = value;
}
void m_helper::write_le16(void *ptr, uint16_t value)
{
    uint8_t* p = (uint8_t*)ptr;

    p[0] = (uint8_t) value;
    p[1] = (uint8_t) (value >> 8);
}
void m_helper::write_le32(void *ptr, uint32_t value)
{
    uint8_t* p = (uint8_t*)ptr;

    p[0] = (uint8_t) value;
    p[1] = (uint8_t) (value >> 8);
    p[2] = (uint8_t) (value >> 16);
    p[3] = (uint8_t) (value >> 24);
}
void m_helper::write_le64(void *ptr, uint64_t value)
{
    uint8_t* const dstPtr = (uint8_t*)ptr;

    dstPtr[0] = (uint8_t)(value);
    dstPtr[1] = (uint8_t)(value >> 8);
    dstPtr[2] = (uint8_t)(value >> 16);
    dstPtr[3] = (uint8_t)(value >> 24);
    dstPtr[4] = (uint8_t)(value >> 32);
    dstPtr[5] = (uint8_t)(value >> 40);
    dstPtr[6] = (uint8_t)(value >> 48);
    dstPtr[7] = (uint8_t)(value >> 56);
}

const QString m_helper::bin_to_str(uint64_t input, int count, int endian)
{
    return num_to_str(input, 2, count, endian);
}
const QString m_helper::oct_to_str(uint64_t input, int count, int endian)
{
    return num_to_str(input, 8, count, endian);
}
const QString m_helper::dec_to_str(uint64_t input, int count, int endian)
{
    return num_to_str(input, 10, count, endian);
}
const QString m_helper::hex_to_str(uint64_t input, int count, int endian)
{
    return num_to_str(input, 16, count, endian);
}

const QString m_helper::storage_conv(uint64_t input)
{
    if (input >= 1099511627776)
    {
        return QString::number((double)input/1099511627776, 'f', 2) + " TB";
    }
    if (input >= 1073741824)
    {
        return QString::number((double)input/1073741824, 'f', 2) + " GB";
    }
    if (input >= 1048576)
    {
        return QString::number((double)input/1048576, 'f', 2) + " MB";
    }
    if (input >= 1024)
    {
        return QString::number((double)input/1024, 'f', 2) + " KB";
    }
    if (input == 1 || input == 0)
    {
        return QString::number(input) + " BT";
    }

    return QString::number(input) + " BT";
}
const QString m_helper::transfer_conv(uint64_t input)
{
    /*if (input >= 1073741824)
    {
        return QString::number((double)(input*8)/1073741824, 'f', 2) + " GB/S";
    }*/
    if (input >= 1048576)
    {
        return QString::number((double)(input*8)/1048576, 'f', 2) + " MB/S";
    }
    if (input >= 1024)
    {
        return QString::number((double)(input*8)/1024, 'f', 2) + " KB/S";
    }
    return QString::number((double)input, 'f', 2) + " BT/S";
}
qint64 m_helper::storage_conv(const QString input)
{
    bool check;
    QString number = input.split(' ').at(0);
    QString postfix = input.split(' ').at(1);

    qint64 ret = number.toULongLong(&check, 10);

    if (!check) return -1;

    /*if (postfix == "TB")
    {
        ret *= (qint64)(1 << 40);
    }
    else */if (postfix == "GB")
    {
        ret *= (qint64)(1 << 30);
    }
    else if (postfix == "MB")
    {
        ret *= (qint64)(1 << 20);
    }
    else if (postfix == "KB")
    {
        ret *= (qint64)(1 << 10);
    }
    else if (postfix.isEmpty())
    {
        // do nothing
    }
    else
    {
        return -1;
    }

    return ret;
}

char* m_helper::convert_to_char(QString text)
{
    char* str;
    std::string tmp = text.toStdString();
    str = new char[tmp.size() + 1];
    strcpy(str, tmp.c_str());

    return str;
}

QString m_helper::read_guid_partition_name(char *data)
{
    char *part = reinterpret_cast<char*>(malloc(36));

    for (int i = 0; i < 72; i++)
    {
        if (data+i != 0)
        {
            if (!i)
                strcpy(part, (const char*)data+i);
            else
                strcat(part, (const char*)data+i);
        }
    }

    return QString::fromUtf8((const char*)part);
}

uint64_t m_helper::str_to_num(QString input, int base)
{
    return input.toULongLong(nullptr, base);
}

QString m_helper::num_to_str(uint64_t input, int base, int count, int endian)
{
    QString tmp, ret;
    tmp.setNum(input, base);

    if (count == 0)
    {
        tmp = tmp.toUpper();
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            tmp.prepend("0");
        }

        tmp = tmp.right(count).toUpper();
    }

    if (endian)
    {
        int index = (count == 0)? tmp.length() : count;
        while (index > 0)
        {
            if (index == 1)
            {
                ret += "0" + tmp.mid(0, 1);
                index--;
            }
            else
            {
                ret += tmp.mid(index - 2, 2);
                index -= 2;
            }
        }
    }
    else
    {
        ret = tmp;
    }

    return ret;
}

int m_helper::padding(uint32_t size, uint32_t padding_size)
{
    uint32_t mask = padding_size - 1;

    if ((size & mask) == 0)
    {
        return 0;
    }

    return padding_size - (size & mask);
}
//*********************************************************************************
QString m_helper::get_hex(quint64 hex, int count)
{
    return QString("0x%1").arg(hex_to_str(hex, count));
}

QString m_helper::GetHex(quint64 hex)
{
    return QString("0x%1").arg(hex_to_str(hex, 0, 0)).toLower();
}

QString m_helper::get_hex_num(char *raw)
{
    return QByteArray::fromRawData(raw, strlen(raw)).toHex().toUpper();
}

const QString m_helper::get_unit(uint64_t input)
{
    QString conv(storage_conv(input));
    return conv.replace(".00", "").trimmed();
}

QString m_helper::FormatMacAddr(const QString rawMAC)
{
    if (rawMAC.isEmpty())
        return QString();

    QString result;
    int macLen = rawMAC.length() / 2;
    for (int i = 0; i < macLen; i++)
    {
        QString spl = rawMAC.mid(i * 2, 2);
        if(i == 5)
            result.append(QString("%0").arg(spl));
        else
            result.append(QString("%0:").arg(spl));
    }

    return result;
}

QByteArray m_helper::GetMD5Hash(QByteArray buff)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(buff);
    return hash.result().toHex();
}

//QString m_helper::GetMD5Hash(QString path)
//{
//    QFile in(path);
//    if (in.open(QIODevice::ReadOnly))
//        return QString();

//    char buf[2048];
//    int bytesRead;
//    qint64 overallBytesRead = 0;

//    QCryptographicHash hash(QCryptographicHash::Md5);

//    while ((bytesRead = in.read(buf, 2048)) > 0) {
//        overallBytesRead += bytesRead;
//        hash.addData(buf, 2048);
//    }

//    in.close();

//    return hash.result().toHex();
//}

QByteArray m_helper::EraseBuff(quint64 length)
{
    return QByteArray().fill('\x00', length);
}

bool m_helper::MakeExt4FS(QString partName, QString output, quint64 offset)
{
    QTemporaryDir tDir;

    if (!tDir.isValid())
        return false;

    QString usrPath = tDir.path();
    QDir mainDir(usrPath + "/" + partName + "_/");
    mainDir.mkdir(".");
    QDir lostDir(mainDir.absolutePath() + "/lost+found");
    lostDir.mkdir(".");

    QString ext4fs = QDir::currentPath() + "/ASPT/aosp_internal/extandroid/mkext4fs.exe";

    QString aa = runProcessWithReturn(ext4fs, QStringList() << "-s" << "-l" <<
                                      QString::number(offset) << "-a" << partName << output << mainDir.absolutePath() + "/");

    bool is_ok = QFileInfo(output).exists();

    tDir.remove();
    return is_ok;
}

QString m_helper::decimaltohex(quint64 input, quint8 count, bool endian)
{
    QString value;
    QString ans;

    if (count == 0)
    {

        QString tmp;
        tmp.setNum(input, 0x10);
        value = tmp.toUpper();
    }
    else
    {

        QString tmp;
        tmp.setNum(input, 0x10);
        QString ind;

        for (quint8 i = 0; i < count; i++)
            ind.prepend("0");

        QString res = ind + tmp;
        value = res.right(count).toUpper();
    }

    if (endian)
    {

        quint8 ind = (count == 0) ? value.length() : count;
        while (ind > 0)
        {

            if (ind == 1)
            {
                ans += "0" + value.mid(0, 1);
                ind--;
            }
            else
            {
                ans += value.mid(ind - 2, 2);
                ind -= 2;
            }
        }

        value = ans;
    }

    return value;
}

QByteArray m_helper::decimalToBytes(quint64 input, quint8 count, bool endian)
{
    return QByteArray::fromHex(QString(decimaltohex(input, count, endian)).toLatin1());
}

uint32_t m_helper::readBytesFromBytes(QByteArray input, QByteArray &output, uint32_t startaddr, uint32_t offset, bool endian)
{
    output.clear();

    if (startaddr >= (uint32_t) input.length())
        return 0;

    if (!endian)
    {
        output.append(input.mid(startaddr, offset));
        return output.length();
    }

    for (uint32_t i = 0; i < offset; i++)
    {
        output.prepend(input.at(startaddr + i));
    }

    return output.length();
}

QByteArray m_helper::qChecksum(QByteArray data)
{
    uint32_t len = data.length();
    uint32_t i, c, csum = 0;

    unsigned int cconst[] = {0,0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF};

    unsigned char *buf = (unsigned char*)data.data();

    for (i = 0; i < len; i++)
    {
        c = (buf[i] & 0xff);
        csum = ((csum << 4) & 0xffff) ^ cconst[(c >> 4) ^ (csum >> 12)];
        csum = ((csum << 4) & 0xffff) ^ cconst[(c & 0xf) ^ (csum >> 12)];
    }

    QByteArray res;
    res.append((csum >> 8) & 0xff);
    res.append(csum & 0xff);

    return res;
}

QByteArray m_helper::splitqba(const QByteArray &seq, int span)
{
//    if (seq.isEmpty() || span <= 0)
//        return QByteArray();

//    const int oldArrSize = seq.size();
//    QByteArray result(oldArrSize + (oldArrSize / span) - (oldArrSize % span == 0), ' ');
//    auto sourceIter = seq.cbegin();
//    auto destIter = result.data();
//    const auto srcEnd = seq.cend();

//    for (int dstnc = std::distance(sourceIter, srcEnd); dstnc > 0; dstnc -= span)
//    {
//        memcpy(destIter, sourceIter, qMin(dstnc, span));
//        destIter += span + 1;
//        sourceIter += span;
//    }

    return seq.toUpper();
}
