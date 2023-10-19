// #include "mainwindow.h"
#include "loadability.h"

// #include <QApplication>


int main(int argc, char *argv[])
{
    // QApplication a(argc, argv);
    Loadability load(true, true, true);

    // load.getNetworkUsage();
    load.getCPUCurrentValue();
    // MainWindow w;
    // w.setFixedSize(1280, 720);
    // w.show();
    // return a.exec();
    return 0;
}
