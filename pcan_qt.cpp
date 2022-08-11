#include "pcan_qt.h"
#include "ui_pcan_qt.h"

PCAN_QT::PCAN_QT(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PCAN_QT)
{
    ui->setupUi(this);
}

PCAN_QT::~PCAN_QT()
{
    delete ui;
}

