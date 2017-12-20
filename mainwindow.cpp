#include "mainwindow.h"
#include "config.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ConfigureConnectionControls(); //configure the connection ui: ports, baud rate, etc
    EnableControls(true);

    CreateSerialWorker(); // create the serial worker thread

    timeTicker = QSharedPointer<QCPAxisTickerTime>(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");

    //configure the plots; add 3 lines (pid input, setpoint, output) for each; configure colors
    ConfigurePidPlot(ui->customPlotPid1);
    ConfigurePidPlot(ui->customPlotPid2);
    ConfigurePidPlot(ui->customPlotPid3);

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    QTimer* dataTimer = new QTimer(this);
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(RealTimeDataSlot()));
    dataTimer->start(0); // Interval 0 means to refresh as fast as possible
}

MainWindow::~MainWindow()
{
    emit requestDisconnect();
    serialWorkerThread.terminate();
    serialWorkerThread.wait();
    delete ui;
}

//--------------------------- Ui and buttons ------------------------------------------------------//
void MainWindow::ConfigureConnectionControls()
{
    /* Check if there are any ports at all; if not, disable controls and return */
    if (QSerialPortInfo::availablePorts().size() == 0) {
        //  enable_com_controls (false);
        // ui->statusBar->showMessage ("No ports detected.");
        // ui->savePNGButton->setEnabled (false);
        return;
    }

    /* List all available serial ports and populate ports combo box */
    for (QSerialPortInfo port : QSerialPortInfo::availablePorts()) {
        ui->comboPort->addItem(port.portName());
    }

    /* Populate baud rate combo box with standard rates */
    ui->comboBaud->addItem("1200");
    ui->comboBaud->addItem("2400");
    ui->comboBaud->addItem("4800");
    ui->comboBaud->addItem("9600");
    ui->comboBaud->addItem("19200");
    ui->comboBaud->addItem("38400");
    ui->comboBaud->addItem("57600");
    ui->comboBaud->addItem("115200");
    /* And some not-so-standard */
    ui->comboBaud->addItem("128000");
    ui->comboBaud->addItem("153600");
    ui->comboBaud->addItem("230400");
    ui->comboBaud->addItem("250000");
    ui->comboBaud->addItem("256000");
    ui->comboBaud->addItem("460800");
    ui->comboBaud->addItem("921600");

    /* Select 115200 bits by default */
    ui->comboBaud->setCurrentIndex(7);

    /* Populate data bits combo box */
    ui->comboData->addItem("8 bits");
    ui->comboData->addItem("7 bits");

    /* Populate parity combo box */
    ui->comboParity->addItem("none");
    ui->comboParity->addItem("odd");
    ui->comboParity->addItem("even");

    /* Populate stop bits combo box */
    ui->comboStop->addItem("1 bit");
    ui->comboStop->addItem("2 bits");
}

void MainWindow::CreateSerialWorker()
{
    serialWorker = new SerialWorker;
    serialWorker->moveToThread(&serialWorkerThread);

    //this -> serialWorker
    connect(this, &MainWindow::requestConnect, serialWorker, &SerialWorker::PortConnect);
    connect(this, &MainWindow::requestDisconnect, serialWorker, &SerialWorker::PortDisconnect);
    connect(this, &MainWindow::requestSendData, serialWorker, &SerialWorker::PortSendData);

    //serialWorker -> this
    //qRegisterMetaType<QHash<int,QVector<double>>>("QHash<int,QVector<double>>");
    connect(serialWorker, &SerialWorker::ForwardReceivedDataDouble, this, &MainWindow::ReceiveDataDoubleFromWroker);
    connect(serialWorker, &SerialWorker::portOpenOK, this, &MainWindow::serialConnectOk);
    connect(serialWorker, &SerialWorker::portOpenFail, this, &MainWindow::serialConnectFailed);
    connect(serialWorker, &SerialWorker::portClosed, this, &MainWindow::serialPortClosed);

    serialWorkerThread.start();
}

void MainWindow::serialConnectOk()
{
    EnableControls(false);
    Connected = true;
    SendCommand(CUTE_GET_ALL_PID_CFGS);
}
void MainWindow::serialConnectFailed()
{
    EnableControls(true);
    Connected = false;
}

void MainWindow::serialPortClosed()
{
    EnableControls(true);
    Connected = false;
}

void MainWindow::EnableControls(bool enable)
{
    /* Com port properties */
    ui->comboBaud->setEnabled(enable);
    ui->comboData->setEnabled(enable);
    ui->comboParity->setEnabled(enable);
    ui->comboPort->setEnabled(enable);
    ui->comboStop->setEnabled(enable);
    ui->pushButtonConnect->setEnabled(enable);

    ui->pushButtonDisconnect->setEnabled(!enable);

    ui->doubleSpinBoxP1Kp->setEnabled(!enable);
    ui->doubleSpinBoxP1Ki->setEnabled(!enable);
    ui->doubleSpinBoxP1Kd->setEnabled(!enable);
    ui->doubleSpinBoxP1Setpoint->setEnabled(!enable);

    ui->doubleSpinBoxP2Kp->setEnabled(!enable);
    ui->doubleSpinBoxP2Ki->setEnabled(!enable);
    ui->doubleSpinBoxP2Kd->setEnabled(!enable);
    ui->doubleSpinBoxP2Setpoint->setEnabled(!enable);

    ui->doubleSpinBoxP3Kp->setEnabled(!enable);
    ui->doubleSpinBoxP3Ki->setEnabled(!enable);
    ui->doubleSpinBoxP3Kd->setEnabled(!enable);
    ui->doubleSpinBoxP3Setpoint->setEnabled(!enable);

    ui->checkBoxGiroToMot->setEnabled(!enable);
    ui->checkBoxP1Prints->setEnabled(!enable);
    ui->checkBoxP2Prints->setEnabled(!enable);
    ui->checkBoxP3Prints->setEnabled(!enable);
}

//---------------------------- PLOT STUFF ---------------------------------------------------//

void MainWindow::ConfigurePidPlot(QCustomPlot* plot)
{
#ifdef USE_OPENGL
    plot->setOpenGl(true);
#endif

#ifdef HIGH_PERF
    /* Used for higher performance (see QCustomPlot real time example) */
    plot->setNotAntialiasedElements(QCP::aeAll);
    QFont font;
    font.setStyleStrategy(QFont::NoAntialias);
    plot->legend->setFont(font);
#endif

    plot->addGraph(); // red line
    plot->graph(0)->setPen(QPen(Qt::red));
    plot->graph(0)->setName("Input");

    plot->addGraph(); // blue line
    plot->graph(1)->setPen(QPen(Qt::blue));
    plot->graph(1)->setName("Output");

    plot->addGraph(); // green line
    plot->graph(2)->setPen(QPen(Qt::green));
    plot->graph(2)->setName("Setpoint");

    plot->legend->setVisible(true);
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft | Qt::AlignTop); // make legend align in top left corner or axis rect

    plot->xAxis->setTicker(timeTicker);
    plot->axisRect()->setupFullAxesBox();
    plot->yAxis->setRange(-5, 5);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));
}

void MainWindow::BlockSignals(bool enable)
{
    ui->doubleSpinBoxP1Kp->blockSignals(enable);
    ui->doubleSpinBoxP1Ki->blockSignals(enable);
    ui->doubleSpinBoxP1Kp->blockSignals(enable);
    ui->doubleSpinBoxP1Setpoint->blockSignals(enable);

    ui->doubleSpinBoxP2Kp->blockSignals(enable);
    ui->doubleSpinBoxP2Ki->blockSignals(enable);
    ui->doubleSpinBoxP2Kp->blockSignals(enable);
    ui->doubleSpinBoxP2Setpoint->blockSignals(enable);

    ui->doubleSpinBoxP3Kp->blockSignals(enable);
    ui->doubleSpinBoxP3Ki->blockSignals(enable);
    ui->doubleSpinBoxP3Kp->blockSignals(enable);
    ui->doubleSpinBoxP3Setpoint->blockSignals(enable);
}

void MainWindow::UpdatePidValues()
{

    BlockSignals(true);
    //PID1
    if (!receivedData[ARD_PID1_INPUT].isEmpty()) {
        ui->lineEditP1Input->setText(QString::number(receivedData[ARD_PID1_INPUT].last()));
    }
    if (!receivedData[ARD_PID1_OUTPUT].isEmpty()) {
        ui->lineEditP1Output->setText(QString::number(receivedData[ARD_PID1_OUTPUT].last()));
    }
    if (!receivedData[ARD_PID1_SETPOINT].isEmpty()) {
        ui->doubleSpinBoxP1Setpoint->setValue(receivedData[ARD_PID1_SETPOINT].last());
    }
    if (!receivedData[ARD_PID1_KP].isEmpty()) {
        ui->doubleSpinBoxP1Kp->setValue(receivedData[ARD_PID1_KP].last());
    }
    if (!receivedData[ARD_PID1_KI].isEmpty()) {
        ui->doubleSpinBoxP1Ki->setValue(receivedData[ARD_PID1_KI].last());
    }
    if (!receivedData[ARD_PID1_KD].isEmpty()) {
        ui->doubleSpinBoxP1Kd->setValue(receivedData[ARD_PID1_KD].last());
    }

    //PID2
    if (!receivedData[ARD_PID2_INPUT].isEmpty()) {
        ui->lineEditP2Input->setText(QString::number(receivedData[ARD_PID2_INPUT].last()));
    }
    if (!receivedData[ARD_PID2_OUTPUT].isEmpty()) {
        ui->lineEditP2Output->setText(QString::number(receivedData[ARD_PID2_OUTPUT].last()));
    }
    if (!receivedData[ARD_PID2_SETPOINT].isEmpty()) {
        ui->doubleSpinBoxP2Setpoint->setValue(receivedData[ARD_PID2_SETPOINT].last());
    }
    if (!receivedData[ARD_PID2_KP].isEmpty()) {
        ui->doubleSpinBoxP2Kp->setValue(receivedData[ARD_PID2_KP].last());
    }
    if (!receivedData[ARD_PID2_KI].isEmpty()) {
        ui->doubleSpinBoxP2Ki->setValue(receivedData[ARD_PID2_KI].last());
    }
    if (!receivedData[ARD_PID2_KD].isEmpty()) {
        ui->doubleSpinBoxP2Kd->setValue(receivedData[ARD_PID2_KD].last());
    }

    //PID3
    if (!receivedData[ARD_PID3_INPUT].isEmpty()) {
        ui->lineEditP3Input->setText(QString::number(receivedData[ARD_PID3_INPUT].last()));
    }
    if (!receivedData[ARD_PID3_OUTPUT].isEmpty()) {
        ui->lineEditP3Output->setText(QString::number(receivedData[ARD_PID3_OUTPUT].last()));
    }
    if (!receivedData[ARD_PID3_SETPOINT].isEmpty()) {
        ui->doubleSpinBoxP3Setpoint->setValue(receivedData[ARD_PID3_SETPOINT].last());
    }
    if (!receivedData[ARD_PID3_KP].isEmpty()) {
        ui->doubleSpinBoxP3Kp->setValue(receivedData[ARD_PID3_KP].last());
    }
    if (!receivedData[ARD_PID3_KI].isEmpty()) {
        ui->doubleSpinBoxP3Ki->setValue(receivedData[ARD_PID3_KI].last());
    }
    if (!receivedData[ARD_PID3_KD].isEmpty()) {
        ui->doubleSpinBoxP3Kd->setValue(receivedData[ARD_PID3_KD].last());
    }
    BlockSignals(false);
}

void MainWindow::RealTimeDataSlot()
{
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double curTime = time.elapsed() / 1000.0; // time elapsed since start of demo, in seconds
    static double lastTime = 0;

    if (Connected == false) {
        return;
    }

    if (curTime - lastTime > 0.002) // at most add point every 2 ms
    {
        // add data to lines:
        QMutexLocker locker(&mutexRecData);
        ui->customPlotPid1->graph(0)->addData(receivedDataTimestamps[ARD_PID1_INPUT], receivedData[ARD_PID1_INPUT], true);
        ui->customPlotPid1->graph(1)->addData(receivedDataTimestamps[ARD_PID1_OUTPUT], receivedData[ARD_PID1_OUTPUT], true);
        ui->customPlotPid1->graph(2)->addData(receivedDataTimestamps[ARD_PID1_SETPOINT], receivedData[ARD_PID1_SETPOINT], true);

        ui->customPlotPid2->graph(0)->addData(receivedDataTimestamps[ARD_PID2_INPUT], receivedData[ARD_PID2_INPUT], true);
        ui->customPlotPid2->graph(1)->addData(receivedDataTimestamps[ARD_PID2_OUTPUT], receivedData[ARD_PID2_OUTPUT], true);
        ui->customPlotPid2->graph(2)->addData(receivedDataTimestamps[ARD_PID2_SETPOINT], receivedData[ARD_PID2_SETPOINT], true);

        ui->customPlotPid3->graph(0)->addData(receivedDataTimestamps[ARD_PID3_INPUT], receivedData[ARD_PID3_INPUT], true);
        ui->customPlotPid3->graph(1)->addData(receivedDataTimestamps[ARD_PID3_OUTPUT], receivedData[ARD_PID3_OUTPUT], true);
        ui->customPlotPid3->graph(2)->addData(receivedDataTimestamps[ARD_PID3_SETPOINT], receivedData[ARD_PID3_SETPOINT], true);

        ui->customPlotPid1->yAxis->rescale();
        ui->customPlotPid2->yAxis->rescale();
        ui->customPlotPid3->yAxis->rescale();

        UpdatePidValues();
        receivedData.clear();
        receivedDataTimestamps.clear();

        lastTime = curTime;
    }

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlotPid1->xAxis->setRange(curTime, 32, Qt::AlignRight);
    ui->customPlotPid1->replot();
    ui->customPlotPid2->xAxis->setRange(curTime, 32, Qt::AlignRight);
    ui->customPlotPid2->replot();
    ui->customPlotPid3->xAxis->setRange(curTime, 32, Qt::AlignRight);
    ui->customPlotPid3->replot();

    // calculate frames per second:
    static double lastFpsTimeSlice;
    static int frameCount;

    ++frameCount;
    if (curTime - lastFpsTimeSlice > 2) // average fps over 2 seconds
    {
        ui->statusBar->showMessage(
            QString("%1 FPS, Total Data points: %2")
                .arg(frameCount / (curTime - lastFpsTimeSlice), 0, 'f', 0)
                .arg(ui->customPlotPid1->graph(0)->data()->size() + ui->customPlotPid1->graph(1)->data()->size()),
            0);
        lastFpsTimeSlice = curTime;
        frameCount = 0;
    }
}

void MainWindow::ReceiveDataDoubleFromWroker(char cmd, double value, double timestamp)
{

    QMutexLocker locker(&mutexRecData);
    receivedData[cmd].append(value);
    receivedDataTimestamps[cmd].append(timestamp);
}

void MainWindow::SendCommand(uint8_t cmd)
{
    QByteArray data;
    data.append(cmd);
    data.append('\n');
    emit requestSendData(data);
}

void MainWindow::SendCommand(uint8_t cmd, QString val)
{
    QByteArray data;
    data.append(cmd);
    data.append(val);
    data.append('\n');
    emit requestSendData(data);
}

//------------------------------------------ ON SLOTS -------------------------------------------//

void MainWindow::on_pushButtonConnect_clicked()
{
    /* If application is not connected, connect */
    /* Get parameters from controls first */
    QString portName = ui->comboPort->currentText(); // Get port name from combo box
    int baudRate = ui->comboBaud->currentText().toInt(); // Get baud rate from combo box
    int dataBitsIndex = ui->comboData->currentIndex(); // Get index of data bits combo box
    int parityIndex = ui->comboParity->currentIndex(); // Get index of parity combo box
    int stopBitsIndex = ui->comboStop->currentIndex(); // Get index of stop bits combo box

    /* Open serial port and connect its signals */
    emit requestConnect(portName, baudRate, dataBitsIndex, parityIndex, stopBitsIndex);
}

void MainWindow::on_pushButtonDisconnect_clicked()
{
    emit requestDisconnect();
}

void MainWindow::on_pushButtonGetPidCfgs_clicked()
{
    SendCommand(CUTE_GET_ALL_PID_CFGS);
}

void MainWindow::on_checkBoxGiroToMot_stateChanged(int arg1)
{
    arg1 == Qt::Checked ? SendCommand(CUTE_GIRO_TO_MOT_ON) : SendCommand(CUTE_GIRO_TO_MOT_OFF);
}

void MainWindow::on_checkBoxP1Prints_stateChanged(int arg1)
{
    arg1 == Qt::Checked ? SendCommand(CUTE_P1_PRNT_ON) : SendCommand(CUTE_P1_PRNT_OFF);
}

void MainWindow::on_checkBoxP2Prints_stateChanged(int arg1)
{
    arg1 == Qt::Checked ? SendCommand(CUTE_P2_PRNT_ON) : SendCommand(CUTE_P2_PRNT_OFF);
}

void MainWindow::on_checkBoxP3Prints_stateChanged(int arg1)
{
    arg1 == Qt::Checked ? SendCommand(CUTE_P3_PRNT_ON) : SendCommand(CUTE_P3_PRNT_OFF);
}

void MainWindow::on_doubleSpinBoxP1Kp_valueChanged(const QString& arg1)
{
    SendCommand(CUTE_PID1_KP, arg1);
}

void MainWindow::on_doubleSpinBoxP1Ki_valueChanged(const QString& arg1)
{
    SendCommand(CUTE_PID1_KI, arg1);
}

void MainWindow::on_doubleSpinBoxP1Kd_valueChanged(const QString& arg1)
{
    SendCommand(CUTE_PID1_KD, arg1);
}

void MainWindow::on_doubleSpinBoxP1Setpoint_valueChanged(const QString& arg1)
{
    SendCommand(CUTE_PID1_SETP, arg1);
}

void MainWindow::on_doubleSpinBoxP2Kp_valueChanged(const QString& arg1)
{
    SendCommand(CUTE_PID2_KP, arg1);
}

void MainWindow::on_doubleSpinBoxP2Ki_valueChanged(const QString& arg1)
{
    SendCommand(CUTE_PID2_KI, arg1);
}

void MainWindow::on_doubleSpinBoxP2Kd_valueChanged(const QString& arg1)
{
    SendCommand(CUTE_PID2_KD, arg1);
}

void MainWindow::on_doubleSpinBoxP2Setpoint_valueChanged(const QString& arg1)
{
    SendCommand(CUTE_PID2_SETP, arg1);
}

void MainWindow::on_doubleSpinBoxP3Kp_valueChanged(const QString& arg1)
{
    SendCommand(CUTE_PID3_KP, arg1);
}

void MainWindow::on_doubleSpinBoxP3Ki_valueChanged(const QString& arg1)
{
    SendCommand(CUTE_PID3_KI, arg1);
}

void MainWindow::on_doubleSpinBoxP3Kd_valueChanged(const QString& arg1)
{
    SendCommand(CUTE_PID3_KD, arg1);
}

void MainWindow::on_doubleSpinBoxP3Setpoint_valueChanged(const QString& arg1)
{
    SendCommand(CUTE_PID3_SETP, arg1);
}
