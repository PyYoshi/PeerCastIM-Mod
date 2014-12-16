#include "mainwindow.h"
#include <QApplication>

#include "peercast.h"
#include "servmgr.h"
#include "channel.h"
#include "version2.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
