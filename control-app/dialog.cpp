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

    isArduinoAvailable = false;
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

    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        if (serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()){
            if(serialPortInfo.vendorIdentifier() == ARDUINO_VENDOR_ID &&
                    serialPortInfo.productIdentifier() == ARDUINO_PRODUCT_ID){
                arduinoPortName = serialPortInfo.portName();
                isArduinoAvailable = true;
            }
        }
    }

    if(isArduinoAvailable){
        arduino->setPortName(arduinoPortName);
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

    ui->imgLabel->setScaledContents(true);
    ui->imgLabel->setPixmap(QPixmap("../resources/black.png"));

    xCoord = int(ui->imgLabel->pixmap()->width() / 2);
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

    qDebug() << receivedFrameNum;

    curPhaseFile.open(QFile::WriteOnly|QFile::Truncate);
    curPhaseFile.write(QByteArray::number(receivedFrameNum));
    curPhaseFile.close();

    nodesStatesFile.open(QFile::ReadOnly);
    receivedNodesStates = QString::fromStdString(nodesStatesFile.readAll().toStdString());
    nodesStatesFile.close();

    drawNodes();
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
    sendDataToArduino(2);
    isArduinoOn = false;
}

void Dialog::on_upPushButton_clicked()
{
    sendDataToArduino(16);
}

void Dialog::on_initPushButton_clicked()
{
    sendDataToArduino(8);
}

void Dialog::on_downPushButton_clicked()
{
    sendDataToArduino(4);
}

void Dialog::on_stopPushButton_clicked()
{
    sendDataToArduino(32);
}
void Dialog::sendDataToArduino(char val){
    if(isArduinoOn){
        if(arduino->isWritable()){
            arduino->write(QByteArray(&val));
        } else {
            QMessageBox::warning(this, "Port error", "Couldn't write data to Arduino!");
        }
    }
}

void Dialog::drawNodes(){
    QString wavelengthsBetweenEmitters = QString::number(ceil(numOfNodes / 2));
    ui->imgLabel->setPixmap(QPixmap("../resources/" + wavelengthsBetweenEmitters + "/" +
                                    wavelengthsBetweenEmitters + "_" +
                                    QString::number(receivedFrameNum) + ".png"));
    repaint();
}

void Dialog::paintEvent(QPaintEvent *event){
    Q_UNUSED(event)
    int brushHeight = 14;
    int brushWidth = 14;
    numOfNodes = receivedNodesStates.length();
    if(isArduinoOn && numOfNodes != 0){
        if(numOfNodes % 2 == 0){
            pxInQuarterWavelength = int((ui->imgLabel->pixmap()->height() / (numOfNodes + 2)) / 2);
        } else {
            pxInQuarterWavelength = int((ui->imgLabel->pixmap()->height() / (numOfNodes + 3)) / 2);
        }
        yInitCoord = int(3 * pxInQuarterWavelength +
                         2 * pxInQuarterWavelength * float(receivedFrameNum) / 24);

        QPixmap pmap(*ui->imgLabel->pixmap());
        QPainter painter(&pmap);
        painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap));

        painter.setBrush(QBrush(QColor(55, 159, 39), Qt::SolidPattern));
        for(int i = 0; i < numOfNodes; i++){
            if(receivedNodesStates.at(i) == '1'){
                painter.drawEllipse(xCoord - int(brushWidth / 2),
                                    yInitCoord - int(brushHeight / 2.0) + i * 2 * pxInQuarterWavelength,
                                    brushWidth, brushHeight);
            }
        }

        ui->imgLabel->setPixmap(pmap);
    } else {
        ui->imgLabel->setPixmap(QPixmap("../resources/black.png"));
    }
}
