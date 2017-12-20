#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "qcustomplot.h"
#include "serialworker.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

signals:
    void requestConnect(QString portName, int baudRate, int dataBitsIndex, int parityIndex, int stopBitsIndex);
    void requestDisconnect();
    void requestSendData(const QByteArray data);

private:
    bool Connected = false;
    bool scaleData = false;
    double SecondsToPlot = 8;

    QMutex mutexRecData;
    Ui::MainWindow* ui;
    SerialWorker* serialWorker;
    QThread serialWorkerThread;
    QSharedPointer<QCPAxisTickerTime> timeTicker;
    QHash<int, QVector<double> > receivedData;
    QHash<int, QVector<double> > receivedDataTimestamps;

    void ConfigurePidPlot(QCustomPlot*);
    void CreateSerialWorker(); //Create the serialWorker thread
    void ConfigureConnectionControls(); // Populate the controls
    void EnableControls(bool enable); // Enable/disable controls
    void UpdatePidValues();
    void SendCommand(uint8_t cmd, QString val);
    void SendCommand(uint8_t cmd);
    void BlockSignals(bool enable);
    void clearPidGraphData(QCustomPlot* plot);

private slots:

    void RealTimeDataSlot();
    void ReceiveDataDoubleFromWroker(char cmd, double value, double timestamp);
    void serialConnectOk();
    void serialConnectFailed();
    void serialPortClosed();

    void on_pushButtonConnect_clicked();
    void on_pushButtonDisconnect_clicked();
    void on_pushButtonGetPidCfgs_clicked();

    void on_checkBoxGiroToMot_stateChanged(int arg1);
    void on_checkBoxP1Prints_stateChanged(int arg1);
    void on_checkBoxP2Prints_stateChanged(int arg1);
    void on_checkBoxP3Prints_stateChanged(int arg1);

    void on_doubleSpinBoxP1Kp_valueChanged(const QString& arg1);
    void on_doubleSpinBoxP1Ki_valueChanged(const QString& arg1);
    void on_doubleSpinBoxP1Kd_valueChanged(const QString& arg1);
    void on_doubleSpinBoxP1Setpoint_valueChanged(const QString& arg1);

    void on_doubleSpinBoxP2Kp_valueChanged(const QString& arg1);
    void on_doubleSpinBoxP2Ki_valueChanged(const QString& arg1);
    void on_doubleSpinBoxP2Kd_valueChanged(const QString& arg1);
    void on_doubleSpinBoxP2Setpoint_valueChanged(const QString& arg1);

    void on_doubleSpinBoxP3Kp_valueChanged(const QString& arg1);
    void on_doubleSpinBoxP3Ki_valueChanged(const QString& arg1);
    void on_doubleSpinBoxP3Kd_valueChanged(const QString& arg1);
    void on_doubleSpinBoxP3Setpoint_valueChanged(const QString& arg1);
    void on_checkboxScaleGraphData_stateChanged(int arg1);
    void on_doubleSpinBoxSecondsToPlot_valueChanged(double arg1);
    void on_pushButtonSavePidConfigs_clicked();
};

#endif // MAINWINDOW_H
