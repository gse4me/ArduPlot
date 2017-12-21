#include "serialworker.h"
#include "config.h"

#include <QTime>

SerialWorker::SerialWorker(QObject* parent)
{
    this->setParent(parent);
    serialPort = nullptr;
}

SerialWorker::~SerialWorker()
{
    CloseConnection();
}

void SerialWorker::CloseConnection()
{
    if (serialPort != nullptr) {
        serialPort->close();
        delete (serialPort);
        serialPort = nullptr;
    }
    emit portClosed();
}

void SerialWorker::PortConnect(QString portName, int baudRate, int dataBitsIndex, int parityIndex, int stopBitsIndex)
{

    QSerialPortInfo portInfo(portName);
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;

    /* Set data bits according to the selected index */
    switch (dataBitsIndex) {
    case 0:
        dataBits = QSerialPort::Data8;
        break;
    default:
        dataBits = QSerialPort::Data7;
    }

    /* Set parity according to the selected index */
    switch (parityIndex) {
    case 0:
        parity = QSerialPort::NoParity;
        break;
    case 1:
        parity = QSerialPort::OddParity;
        break;
    default:
        parity = QSerialPort::EvenParity;
    }

    /* Set stop bits according to the selected index */
    switch (stopBitsIndex) {
    case 0:
        stopBits = QSerialPort::OneStop;
        break;
    default:
        stopBits = QSerialPort::TwoStop;
    }

    if (serialPort != nullptr) //close any existing connections
    {
        CloseConnection();
    }
    serialPort = new QSerialPort(portInfo, 0); // Create a new serial port

    connect(serialPort, SIGNAL(readyRead()), this, SLOT(PortReadData()));

    if (serialPort->open(QIODevice::ReadWrite)) {
        serialPort->setBaudRate(baudRate);
        serialPort->setParity(parity);
        serialPort->setDataBits(dataBits);
        serialPort->setStopBits(stopBits);
        emit portOpenOK();
    } else {
        qDebug() << serialPort->errorString();
        emit portOpenFail();
    }
}

void SerialWorker::PortDisconnect()
{
    CloseConnection();
    qDebug() << "Disconnected";
}

void SerialWorker::PortReadData()
{

    while (serialPort->canReadLine()) {
        char buf[1024];
        serialPort->readLine(buf, sizeof(buf));

        ProcessDataLine(buf);
    }
}

void SerialWorker::ProcessDataLine(char* line)
{
    //Time in case we need to timestamp the received data
    static QTime time(QTime::currentTime());
    double curTime = time.elapsed() / 1000.0; // time elapsed since start, in seconds

    uint8_t target = line[0];

    if (target != ARD_LOG) {
        double value = std::atof(line + 1);
        if (target >= 10 && target <= 18) {
            //qDebug()<<"Target: "<<target<<" Value: "<<value;
        }
        if (value < -255 || value > 255) {
            qDebug() << "Discarding Target: " << target << " Value: " << value << " Meaning: " << QString::number(value);
            qDebug() << "Was received as: " << line << endl;
        } else {
            emit ForwardReceivedDataDouble(target, value, curTime);
        }

    } else {
        qDebug() << "Received" << line + 1;
    }
}

void SerialWorker::PortSendData(const QByteArray data)
{
    serialPort->write(data);
}
