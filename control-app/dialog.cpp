#include "dialog.h"
#include "ui_dialog.h"
#include <QtSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtWidgets>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->brightnessLcdNumber->display("----");

    arduino_is_available = false;
    arduino_port_name = "";
    arduino = new QSerialPort();

//    qDebug() << "Number of available ports: " << QSerialPortInfo::availablePorts().length();
//    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
//        qDebug() << "Has vedor ID: " << serialPortInfo.hasVendorIdentifier();
//        if(serialPortInfo.hasVendorIdentifier()){
//            qDebug() << "Vendor ID: " << serialPortInfo.vendorIdentifier();
//        }
//        qDebug() << "Has product ID: " << serialPortInfo.hasProductIdentifier();
//        if(serialPortInfo.hasProductIdentifier()){
//            qDebug() << "Product ID: " << serialPortInfo.productIdentifier();
//        }
//    }

    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        if (serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()){
            if(serialPortInfo.vendorIdentifier() == arduino_vendor_id &&
                    serialPortInfo.productIdentifier() == arduino_product_id){
                arduino_port_name = serialPortInfo.portName();
                arduino_is_available = true;
            }
        }
    }

    if(arduino_is_available){
        // configure serialport
        arduino->setPortName(arduino_port_name);
        arduino->open(QSerialPort::ReadWrite);
        arduino->setBaudRate(QSerialPort::Baud115200);
        arduino->setDataBits(QSerialPort::Data8);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        arduino->setFlowControl(QSerialPort::NoFlowControl);
        QObject::connect(arduino, SIGNAL(readyRead()), this, SLOT(readSerial()));
    } else {
        QMessageBox::warning(this, "Port error", "Couldn't access Arduino!");
    }
}

Dialog::~Dialog()
{
    if(arduino->isOpen()){
        arduino->close();
    }
    delete ui;
}


void Dialog::on_brightnessSlider_valueChanged(int value)
{
    ui->brightnessLabel->setText(QString("%1").arg(value));
    Dialog::updateBrightness(value);
}

void Dialog::updateBrightness(int value){
    if(arduino->isWritable()){
        arduino->write(QString("i%1").arg(value).toStdString().c_str());
    } else {
        QMessageBox::warning(this, "Port error", "Couldn't write data to Arduino!");
    }
}

void Dialog::readSerial(){
    QByteArray serialData = arduino->readAll();
    QString receivedBrightness = QString::fromStdString(serialData.toStdString());
    qDebug() << receivedBrightness;
   // ui->brightnessLcdNumber->display(receivedBrightness);
}

void Dialog::on_turnOnPushButton_clicked()
{
    sendDataToArduino(QString("c%1").arg(1));
}

void Dialog::on_turnOffPushButton_clicked()
{
    sendDataToArduino(QString("c%1").arg(2));
}

void Dialog::on_upPushButton_clicked()
{
    sendDataToArduino(QString("c%1").arg(4));
}

void Dialog::on_initPushButton_clicked()
{
    sendDataToArduino(QString("c%1").arg(8));
}

void Dialog::on_downPushButton_clicked()
{
    sendDataToArduino(QString("c%1").arg(16));
}

void Dialog::sendDataToArduino(QString val){
    if(arduino->isWritable()){
        arduino->write(val.toStdString().c_str());
    } else {
        QMessageBox::warning(this, "Port error", "Couldn't write data to Arduino!");
    }
}
