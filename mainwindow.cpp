#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "command.h"

#include <QSerialPortInfo>
#include <QDebug>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Electric Charge Station");
    serialPort = new QSerialPort(this);
    init();
}

MainWindow::~MainWindow()
{
    delete serialPort;
    delete ui;
}

void MainWindow::receiveMessage()
{
    QByteArray dataBA = serialPort->readAll();
    QString msg(dataBA);
    ui->textBrowser->setTextColor(Qt::blue); // Receieved message's color is blue.
    ui->textBrowser->append(msg);
    qInfo() << "msg" << msg;

    changeControllerState(msg);
}

void MainWindow::init()
{
    sPortInfo.reset(new QSerialPortInfo);

    //add ports to comboBox
    QList <QSerialPortInfo> ports = sPortInfo->availablePorts();
    QStringList stringPorts;
    for(size_t i = 0 ; i < ports.size() ; i++)
        stringPorts.append(ports.at(i).portName());

    ui->cmbBox_port->addItems(stringPorts);

    //add baudRates to comboBox
    QList <qint32> baudRates = sPortInfo->standardBaudRates();
    QStringList stringBaudRates;
    for(size_t i = 6; i < (baudRates.size() - 2); i++)
    {
        if (i < 7 or (i > 7 and i < 10) or i > 10)
            stringBaudRates.append(QString::number(baudRates.at(i)));
    }
    ui->cmbBox_baudRate->addItems(stringBaudRates);
    ui->cmbBox_baudRate->setCurrentIndex(stringBaudRates.size() - 1);

    //add data bits
    QStringList dataBits = {"5","6","7","8"};
    ui->cmbBox_dataBits->addItems(dataBits);
    ui->cmbBox_dataBits->setCurrentIndex(dataBits.size() - 1);

    //add stop bits
    QStringList stopBits = {"1","2"};
    ui->cmbBox_stopBits->addItems(stopBits);

    //add parity
    QStringList parity = {"NoParity","EvenParity","OddParity","SpaceParity","MarkParity"};
    ui->cmbBox_parity->addItems(parity);

    //add flow control
    QStringList flowControl = {"NoFlowControl","HardwareControl","SoftwareControl"};
    ui->cmbBox_flowControl->addItems(flowControl);

    //add select to get data
    QStringList toGetData = {"U and I", "only U", "one time U,I", "one time I"};
    ui->cmbBox_getData->addItems(toGetData);

}

void MainWindow::changeControllerState(QString receiveMsg)
{
    QString command;
    command.append(receiveMsg[1]).append(receiveMsg[2]);
    qInfo() << "command.toInt() -> " << command.toInt();
    switch (command.toInt())
    {
        case 1:
        if(receiveMsg[4] == '1')
        {
            state = Connect;
            changeTransferData();
            break;
        }
        else if (receiveMsg[4] == '0')
        {
            state = Disconnect;
            ui->btn_connectToPC->setEnabled(true);
            break;
        }
        case 2:
        if(receiveMsg[4] == '1')
        {
            state = DataTransfer;
            ui->btn_connectToPC->setEnabled(true);
            break;
        }
        else if (receiveMsg[4] == '0')
            changeTransferData();
        break;
    }
}

void MainWindow::changeTransferData()
{
    if (state == ControllerState::Connect)
    {
        QString message = ENABLE_DATA_TRANSFER;
        ui->textBrowser->setTextColor(Qt::darkGreen); // Color of message to send is green.
        ui->textBrowser->append("C02:1 - разрешить передачу данных");
        qInfo() << "msg -> " << message.toLocal8Bit() << message.toUtf8();
        serialPort->write(message.toLocal8Bit());
        return;
    }

    QString message = DISABLE_DEVICE;
    ui->textBrowser->setTextColor(Qt::darkGreen); // Color of message to send is green.
    ui->textBrowser->append("C01:0 - отключить устройство от ПК");
    qInfo() << "msg -> " << message.toLocal8Bit() << message.toUtf8();
    serialPort->write(message.toLocal8Bit());
    state = Connect;
}


void MainWindow::on_btn_connect_clicked()
{
    serialPort->setPortName(ui->cmbBox_port->currentText());
    serialPort->open(QIODevice::ReadWrite);

    if(!serialPort->isOpen())
    {
        ui->textBrowser->setTextColor(Qt::red);
        ui->textBrowser->append("Problem with opening Serial Port");
    }
    else
    {
        serialPort->setBaudRate(ui->cmbBox_baudRate->currentText().toInt());
        serialPort->setDataBits(static_cast<QSerialPort::DataBits>(ui->cmbBox_dataBits->currentText().toInt()));
        serialPort->setStopBits(static_cast<QSerialPort::StopBits>(ui->cmbBox_stopBits->currentText().toInt()));

        if(ui->cmbBox_parity->currentIndex() == QSerialPort::NoParity)
            serialPort->setParity(QSerialPort::NoParity);
        else
            serialPort->setParity(static_cast<QSerialPort::Parity>(ui->cmbBox_parity->currentIndex() + 1));

        serialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(ui->cmbBox_flowControl->currentIndex()));

        ui->statusbar->showMessage("Connenction enstablished",3000);

        connect(serialPort,&QSerialPort::readyRead,this, &MainWindow::receiveMessage);
    }
}


void MainWindow::on_btn_refreshPorts_clicked()
{
    sPortInfo.reset(new QSerialPortInfo);

    ui->cmbBox_port->clear();
    QList <QSerialPortInfo> ports = sPortInfo.data()->availablePorts();
    QStringList stringPorts;
    for(size_t i = 0 ; i < ports.size() ; i++)
        stringPorts.append(ports.at(i).portName());

    ui->cmbBox_port->addItems(stringPorts);
}


void MainWindow::on_btn_disconnect_clicked()
{
    if(serialPort->isOpen())
        serialPort->close();

    ui->statusbar->showMessage("Disconnected",3000);
    if(!ui->btn_connectToPC->isEnabled())
    {
        ui->btn_connectToPC->setEnabled(true);
        ui->statusbar->showMessage("Reload Controller!",3000);
    }
}


void MainWindow::on_btn_connectToPC_clicked()
{

    if(!serialPort->isOpen())
        return;

    if(state == ControllerState::Disconnect)
    {
        QString message = ENABLE_DEVICE;
        ui->textBrowser->setTextColor(Qt::darkGreen); // Color of message to send is green.
        ui->textBrowser->append("C01:1 - подключить устройство к ПК");
        qInfo() << "msg -> " << message.toLocal8Bit() << message.toUtf8();
        serialPort->write(message.toLocal8Bit());
        ui->btn_connectToPC->setText("DisconnectToPc");
        ui->btn_connectToPC->setEnabled(false);
        return;
    }
    else
    {
        QString message = DISABLE_DATA_TRANSFER;
        ui->textBrowser->setTextColor(Qt::darkGreen); // Color of message to send is green.
        ui->textBrowser->append("C02:0 - - запретить передачу данных");
        serialPort->write(message.toLocal8Bit());
        ui->btn_connectToPC->setText("ConnectToPc");
        ui->btn_connectToPC->setEnabled(false);
        return;
    }

    ui->statusbar->showMessage("Error ControllerState, go to step",3000);
}


void MainWindow::on_btn_getData_clicked()
{
//    switch (ui->cmbBox_getData->text)
//    {

//    }
}
