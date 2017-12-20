#ifndef SERIALWORKHER_H
#define SERIALWORKHER_H

#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialWorker : public QObject {
    Q_OBJECT

public:
    explicit SerialWorker(QObject* parent = nullptr);
    ~SerialWorker();

public slots:
    void PortConnect(QString portName, int baudRate, int dataBitsIndex, int parityIndex, int stopBitsIndex);
    void PortDisconnect();
    void PortReadData();
    void PortSendData(const QByteArray data);
signals:
    void ForwardReceivedDataDouble(char cmd, double value, double timestamp);
    void portOpenOK();
    void portOpenFail();
    void portClosed();

private:
    QHash<int, QVector<double> > ReceivedData;
    QHash<int, QVector<double> > ReceivedDataTimestamps;

    QSerialPort* serialPort;
    void CloseConnection();

    void ProcessDataLine(char* line);
};

#endif // SERIALWORKHER_H
