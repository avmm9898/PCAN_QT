#include "pcan_qt.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    QFont font("Droid Sans Mono, Microsoft JhengHei", 10);
    font.setPointSize(12);
    QApplication::setFont(font);


    PCAN_QT w;
    w.show();
    return a.exec();
}
