#ifndef PCAN_QT_H
#define PCAN_QT_H

#include <QMainWindow>
#include <QDebug>
#include <QTimer>


QT_BEGIN_NAMESPACE
namespace Ui { class PCAN_QT; }
QT_END_NAMESPACE

class PCAN_QT : public QMainWindow
{
    Q_OBJECT

public:
    PCAN_QT(QWidget *parent = nullptr);
    ~PCAN_QT();

private slots:
    void pcan_read();


private:
    Ui::PCAN_QT *ui;
    QString getStringFromUnsignedChar(unsigned char *str, const int len );


    QTimer *tmr_read;

};
#endif // PCAN_QT_H
