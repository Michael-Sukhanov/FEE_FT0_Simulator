#include "mainwindow.h"

#include <QApplication>
#include <ipbuspacket.h>
#include <auxiliary/mrand.h>
#include<QTime>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("FEE T0 Emulator");
    QCoreApplication::setApplicationVersion("0.2");
    MainWindow w;
    w.show();
//    Log log(true, "IPbusServer");
//    FEE T0;
//    Server server(&T0, &log);
    return a.exec();
}
