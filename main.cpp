#include "pcan_qt.h"

#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    QFontDatabase::addApplicationFont(":/fonts/DroidSansMono.ttf");
    QFont font("Droid Sans Mono, Microsoft JhengHei", 10);
    font.setPointSize(11);
    QApplication::setFont(font);


    PCAN_QT w;
    w.show();
    return a.exec();
}
