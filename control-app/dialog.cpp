#include "dialog.h"
#include "ui_dialog.h"
#include <QtSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtWidgets>
#include <QFile>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->heightNumber->display("--");

    arduino_is_available = false;
    arduino_port_name = "";
    arduino = new QSerialPort();
    isArduinoOn = true;

    nodesStatesFile.setFileName("../resources/nodesStates.txt");
    curPhaseFile.setFileName("../resources/curPhase.txt");

    nodesStatesFile.open(QFile::ReadOnly);
    curPhaseFile.open(QFile::WriteOnly|QFile::Truncate);

    if(!nodesStatesFile.isOpen()){
        QMessageBox::warning(this, "File error", "Couldn't open nodesStates.txt file!");
    }
    if(!nodesStatesFile.isOpen()){
        QMessageBox::warning(this, "File error", "Couldn't open curPhase.txt file!");
    }

    curPhaseFile.close();
    nodesStatesFile.close();

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
    receivedFrameNum = QString::fromStdString(serialData.toHex().toStdString()).toInt(&ok, 16);
    ui->heightNumber->display(receivedFrameNum);
    curPhaseFile.open(QFile::WriteOnly|QFile::Truncate);
    curPhaseFile.write(QByteArray::number(receivedFrameNum));
    curPhaseFile.close();

    nodesStatesFile.open(QFile::ReadOnly);
    receivedNodesStates = QString::fromStdString(nodesStatesFile.readAll().toStdString());
    nodesStatesFile.close();
}

void Dialog::on_turnOnPushButton_clicked()
{
    if(isArduinoOn == false){
        isArduinoOn = true;
        sendDataToArduino(1);
    }
}

void Dialog::on_turnOffPushButton_clicked()
{
    isArduinoOn = false;
    ui->heightNumber->display("--");
    sendDataToArduino(2);
}

void Dialog::on_upPushButton_clicked()
{
    if(isArduinoOn)
        sendDataToArduino(4+1);
    else
        sendDataToArduino(4);
}

void Dialog::on_initPushButton_clicked()
{
    if(isArduinoOn)
        sendDataToArduino(8+1);
    else
        sendDataToArduino(8);
}

void Dialog::on_downPushButton_clicked()
{
    if(isArduinoOn)
        sendDataToArduino(16+1);
    else
        sendDataToArduino(16);
}

void Dialog::on_stopPushButton_clicked()
{
    if(isArduinoOn)
        sendDataToArduino(32+1);
    else
        sendDataToArduino(32);
}
void Dialog::sendDataToArduino(char val){
    if(arduino->isWritable()){
        arduino->write(QByteArray(&val));
    } else {
        QMessageBox::warning(this, "Port error", "Couldn't write data to Arduino!");
    }
}


