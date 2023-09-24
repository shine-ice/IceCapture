#include <QApplication>
#include "IceCapture.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    IceCapture w;
    w.show();
    return a.exec();
}
