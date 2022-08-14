#include "pcan_qt.h"
#include "include/PCANBasic.h"
#include "ui_pcan_qt.h"

PCAN_QT::PCAN_QT(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PCAN_QT)
{
    ui->setupUi(this);

    TPCANStatus result;
    char strMsg[256];

    // The Plug & Play Channel (PCAN-USB) is initialized
    //
    result = CAN_Initialize(PCAN_USBBUS1,PCAN_BAUD_500K);
    if(result != PCAN_ERROR_OK)
    {
        // An error occurred, get a text describing the error and show it
        //
        CAN_GetErrorText(result, 0, strMsg);
        qDebug()<<strMsg;
    }
    else
        qDebug()<<"PCAN-USB (Ch-1) was initialized";

    tmr_read= new QTimer();
    connect(tmr_read, &QTimer::timeout, this, &PCAN_QT::pcan_read);
    tmr_read->setInterval(10);
    tmr_read->start();
}

PCAN_QT::~PCAN_QT()
{
    delete ui;
}
QString PCAN_QT::getStringFromUnsignedChar( unsigned char *str, const int len ){
    QString result = "";
    int lengthOfString = len;

    // print string in reverse order
    QString s;
    for( int i = 0; i < lengthOfString; i++ ){
        s = QString( "%1" ).arg( str[i], 0, 16 );

        // account for single-digit hex values (always must serialize as two digits)
        if( s.length() == 1 )
            result.append( "0" );

        result.append( s );
    }

    return result;
}
void PCAN_QT::pcan_read()
{
    TPCANMsg msg;
    TPCANTimestamp timestamp;
    TPCANStatus result;
    char strMsg[256];

    do
    {
        // Check the receive queue for new messages
        //
        result = CAN_Read(PCAN_USBBUS1,&msg,&timestamp);
        if(result != PCAN_ERROR_QRCVEMPTY)
        {
            // Process the received message
            //
            qDebug()<<"A message was received";
            qDebug()<<msg.ID<<msg.LEN<<getStringFromUnsignedChar(msg.DATA,8);

        }
        else
        {
            // An error occurred, get a text describing the error and show it
            // and handle the error
            //
            CAN_GetErrorText(result, 0, strMsg);
            qDebug()<<strMsg;
            // Here can be decided if the loop has to be  terminated (eg. the bus
            // status is  bus-off)
            //

        }
    // Try to read a message from the receive queue of the PCAN-USB, Channel 1,
    // until the queue is empty
    //
    }while((result & PCAN_ERROR_QRCVEMPTY) != PCAN_ERROR_QRCVEMPTY);

}

