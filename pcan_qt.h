#ifndef PCAN_QT_H
#define PCAN_QT_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class PCAN_QT; }
QT_END_NAMESPACE

class PCAN_QT : public QMainWindow
{
    Q_OBJECT

public:
    PCAN_QT(QWidget *parent = nullptr);
    ~PCAN_QT();

private:
    Ui::PCAN_QT *ui;
};
#endif // PCAN_QT_H
