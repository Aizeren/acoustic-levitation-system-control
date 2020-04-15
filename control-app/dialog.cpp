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
    ui->heightNumber->display("--");

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

void Dialog::readSerial(){
    bool ok;
    QByteArray serialData = arduino->readAll();
    int receivedFrameNum = QString::fromStdString(serialData.toHex().toStdString()).toInt(&ok, 16);
    ui->heightNumber->display(receivedFrameNum);
    qDebug() << receivedFrameNum;
}

void Dialog::on_turnOnPushButton_clicked()
{
    sendDataToArduino(1);
}

void Dialog::on_turnOffPushButton_clicked()
{
    sendDataToArduino(2);
}

void Dialog::on_upPushButton_clicked()
{
    sendDataToArduino(4);
}

void Dialog::on_initPushButton_clicked()
{
   sendDataToArduino(8);
}

void Dialog::on_downPushButton_clicked()
{
   sendDataToArduino(16);
}

void Dialog::sendDataToArduino(char val){
    if(arduino->isWritable()){
        arduino->write(QByteArray(&val));
    } else {
        QMessageBox::warning(this, "Port error", "Couldn't write data to Arduino!");
    }
}
