//#include "qsp.h"

//QSP::QSP(QString pname, QObject *parent)
//    : QObject(parent)
//    , port(new QSerialPort(this))
//{
//    if(this->port.isOpen())
//        this->port.close();

//    this->port.setPortName(pname);
//    this->port.setBaudRate(QSerialPort::Baud115200);
//    this->port.setDataBits(QSerialPort::Data8);
//    this->port.setParity(QSerialPort::NoParity);
//    this->port.setStopBits(QSerialPort::OneStop);
//    this->port.setFlowControl(QSerialPort::NoFlowControl);
//    if(!this->port.open(QIODevice::ReadWrite))
//        throw QString(port.errorString());

//    connect (&this->port, &QSerialPort::readyRead,        [=]() { GetReadyRead();});
//    connect (&this->port, &QSerialPort::bytesWritten,     [=](qint64 bytes)
//    {
//        GetBytesWritten(bytes);
//    });
//    connect (&this->port, &QSerialPort::errorOccurred,    [=](QSerialPort::SerialPortError err)
//    {
//        serialPortError(err);
//    });
//}

//QSP::~QSP()
//{
//    if(this->port.isOpen())
//        this->port.close();

//    this->port.deleteLater();
//}

//void QSP::GetBytesWritten(qint64 bytes)
//{
//    qInfo().noquote() << "GetBytesWritten !" << bytes;
//    this->port.waitForReadyRead(1);
//}

//PVOID QSP::DestoryOnce()
//{
//    this->~QSP();
//    return this;
//}

//void QSP::GetReadyRead()
//{
//    qint64 ret = port.bytesAvailable();
//    if (ret <= 0)
//        return;

//    qInfo() << "QSP::readLen " << ret;

//    //    for (int i = 0; i < ret; ++i) {

//    //        char b;
//    //        if(!port.read(&b, 1))
//    //            break;

//    //        ouput.append(b);
//    //    }
//    ouput.append(this->port.readAll());
//}

//uint32_t QSP::write(QByteArray data)
//{
//    if (!this->port.isOpen())
//        return 0;

//    {
//        this->ouput.clear();
//        this->port.clearError();
//        this->port.clear();
//    }

//    qint64 ret = this->port.write(data);
//    if (ret <= 0)
//        return 0;

//    qInfo() << "QSP::sendLen " << ret;

//    this->port.flush();
//    this->port.waitForBytesWritten();
//    return ret;
//}

//uint32_t QSP::read(QByteArray &data, uint32_t len)
//{
//    data.clear();
//    data.append(ouput);
//    data.resize(len);
//    return data.size();
//}
