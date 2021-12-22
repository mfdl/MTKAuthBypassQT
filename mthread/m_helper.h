#ifndef M_HELPER_H
#define M_HELPER_H

#include <QtCore>
#include <QObject>
#include <QProcess>
#include <QMutex>
#include <QApplication>
#include <QDir>
#include "QDebug"

using namespace std;

typedef union { uint16_t u16; uint32_t u32; size_t uArch; } __attribute__((packed)) unalignA;

class m_helper
{
public:
    m_helper(){};

private:
    QMutex mutex;
    QProcess *proc;
    bool cancelOpt;
public:
    void
    runProcessNoReturn(QString executePath, QStringList arguments) {

        mutex.lock();
        proc = new QProcess();

        if (proc) {

            proc->setProcessChannelMode(QProcess::MergedChannels);
            proc->start(executePath, arguments);

            if (proc->waitForStarted()) {

                if (!proc->waitForBytesWritten()) {}
                if (!proc->waitForFinished()) {}

            }

            while (proc->state() != QProcess::NotRunning) {
                QApplication::processEvents();
            }
        }

        mutex.unlock();
    }

    QString
    runProcessWithReturn(QString executePath, QStringList arguments) {

        QString result;

        mutex.lock();
        proc = new QProcess();

        if (proc) {

            proc->setProcessChannelMode(QProcess::MergedChannels);
            proc->start(executePath, arguments);

            if (proc->waitForStarted()) {

                if (!proc->waitForBytesWritten()) {}
                if (!proc->waitForFinished()) {}

                result = QString(proc->readAll() + proc->readAllStandardOutput());

            }

            while (proc->state() != QProcess::NotRunning) {
                QApplication::processEvents();
            }
        }

        mutex.unlock();
        return result;
    }

    int read_data_from_file(QByteArray &data, QString path, qint64 offset = 0, qint64 length = 0);

    uint64_t str_to_bin(QString input);
    uint64_t str_to_oct(QString input);
    uint64_t str_to_dec(QString input);
    uint64_t str_to_hex(QString input);
    uint64_t str_to_hex(char* input, int count);

    qint64 storage_conv(const QString input);

    const QString bin_to_str(uint64_t input, int count, int endian = 0);
    const QString oct_to_str(uint64_t input, int count, int endian = 0);
    const QString dec_to_str(uint64_t input, int count, int endian = 0);
    const QString hex_to_str(uint64_t input, int count, int endian = 0);

    const QString get_unit(uint64_t input);
    const QString storage_conv(uint64_t input);
    const QString transfer_conv(uint64_t input);

    uint16_t read_be16(const void* ptr);
    uint32_t read_be32(const void* ptr);
    size_t read_arch(const void* ptr);
    uint16_t read_le16(const void* ptr);
    uint32_t read_le32(const void* ptr);
    uint64_t read_le64(const void* ptr);

    void write_be16(void* ptr, uint16_t value);
    void write_be32(void* ptr, uint32_t value);
    void write_le16(void* ptr, uint16_t value);
    void write_le32(void* ptr, uint32_t value);
    void write_le64(void* ptr, uint64_t value);

    char* convert_to_char(QString text);

    QString read_guid_partition_name(char *data);

    int padding(uint32_t size, uint32_t padding_size);

public:
    QString get_hex(quint64 hex, int count = 10);
    QString get_hex_num(char *raw);
    QString FormatMacAddr(const QString rawMAC);
    bool MakeExt4FS(QString partName, QString output, quint64 offset);
    QByteArray EraseBuff(quint64 length);
    QByteArray GetMD5Hash(QByteArray buff);
    QString GetHex(quint64 hex);
    bool MakeExt4FSBuff(quint64 length, QByteArray &out_buff, bool is_ufs);
    QByteArray decimalToBytes(quint64 input, quint8 count, bool endian = 1);
    QString decimaltohex(quint64 input, quint8 count, bool endian = 0);
    bool MakeExt4FS(quint64 length, QString path, bool is_ufs);
    QByteArray qChecksum(QByteArray data);
    uint32_t readBytesFromBytes(QByteArray input, QByteArray &output, uint32_t startaddr, uint32_t offset, bool endian = 0);
    QByteArray splitqba(const QByteArray &seq, int span = 2);
private:
    QString num_to_str(uint64_t input, int base, int count, int endian = 0);
    uint64_t str_to_num(QString input, int base);
};

#endif // M_HELPER_H
