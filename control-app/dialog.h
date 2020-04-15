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

private slots:
    void readSerial();
    void on_turnOnPushButton_clicked();
    void on_turnOffPushButton_clicked();
    void on_upPushButton_clicked();
    void on_initPushButton_clicked();
    void on_downPushButton_clicked();
    void sendDataToArduino(QString);

private:
    Ui::Dialog *ui;
    QSerialPort *arduino;
    static const quint16 arduino_vendor_id = 6790;
    static const quint16 arduino_product_id = 29987;
    QString arduino_port_name;
    bool arduino_is_available;

};
#endif // DIALOG_H
