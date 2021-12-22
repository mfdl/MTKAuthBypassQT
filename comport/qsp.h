//#ifndef QSP_H
//#define QSP_H

//#include <QtCore>
//#include <QSerialPort/QSerialPort>
//#include <windows.h>

//class QSP : public QObject
//{
//    Q_OBJECT
//public:
//    explicit QSP(QString pname, QObject *parent = 0);
//    ~QSP();

//    uint32_t write(QByteArray data);
//    uint32_t read(QByteArray &data, uint32_t len);

//    virtual PVOID DestoryOnce();

//signals:
//    void serialPortError(QSerialPort::SerialPortError serialPortError);
//private slots:
//    void GetReadyRead();
//    void GetBytesWritten(qint64 bytes);
//private:
//    QSerialPort port;
//    QByteArray ouput;
//};

//#endif // QSP_H
