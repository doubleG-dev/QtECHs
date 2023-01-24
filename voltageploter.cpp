#include "voltageploter.h"
#include "ui_voltageploter.h"
#include "command.h"

VoltagePloter::VoltagePloter(QSerialPort *ptrSeriaPort,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VoltagePloter),
    serialPort(ptrSeriaPort)
{
    ui->setupUi(this);
    this->setWindowTitle("Voltage Ploter");
    init();
}

VoltagePloter::~VoltagePloter()
{
    delete ui;
}

void VoltagePloter::init()
{
    ui->widget->yAxis->setRange(0,1000);
    ui->widget->yAxis->setLabel("Voltage, Volt");
    ui->widget->xAxis->setLabel("Time, Sec");
    ui->widget->setInteraction(QCP::iRangeZoom, true);
    ui->widget->setInteraction(QCP::iRangeDrag, true);

    connect(serialPort, &QSerialPort::readyRead, this, &VoltagePloter::receiveMsgSerialPort);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VoltagePloter::timerOfPointer);
    timer->start();
}

void VoltagePloter::openVoltagePloter(bool state)
{
    if(state && this->isHidden())
    {
        this->show();
        return;
    }

    if(!state && this->isVisible())
    {
        this->hide();
        return;
    }
}

void VoltagePloter::timerOfPointer()
{
    timer->stop();
    serialPort->write(QString(READ_DATA)
}

void VoltagePloter::receiveMsgSerialPort()
{

}
