#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void readSerial();
    void sendDataToArduino(char);
    void drawNodes();
    void on_turnOnPushButton_clicked();
    void on_turnOffPushButton_clicked();
    void on_upPushButton_clicked();
    void on_initPushButton_clicked();
    void on_downPushButton_clicked();
    void on_stopPushButton_clicked();

private:
    Ui::Dialog *ui;
    QSerialPort *arduino;
    static const quint16 ARDUINO_VENDOR_ID = 6790;
    static const quint16 ARDUINO_PRODUCT_ID = 29987;
    QString arduinoPortName;
    bool isArduinoAvailable, isArduinoOn;

    QFile curPhaseFile, nodesStatesFile;
    QString receivedNodesStates;
    int xCoord, yInitCoord, pxInQuarterWavelength,
        receivedFrameNum, numOfNodes;
};
#endif // DIALOG_H
