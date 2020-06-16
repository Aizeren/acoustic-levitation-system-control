#include "dialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QFile style(":/style.css");
    style.open(QFile::ReadOnly);
    app.setStyleSheet(style.readAll());
    Dialog window;
    window.setFixedSize(500, 350);
    window.setWindowTitle("Acoustic Levitation System Control");
    window.show();
    return app.exec();
}
