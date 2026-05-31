#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("三相电力负荷监测系统");
    app.setApplicationVersion("1.0");

    MainWindow w;
    w.setWindowTitle("三相电力负荷监测系统 v1.0");
    w.resize(1000, 700);
    w.show();

    return app.exec();
}
