#include "dialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile style(":/style.css");
    style.open(QFile::ReadOnly);
    a.setStyleSheet(style.readAll());
    Dialog w;
    w.setFixedSize(500, 350);
    w.setWindowTitle("Acoustic Levitation System Control");
    w.show();
    return a.exec();
}
