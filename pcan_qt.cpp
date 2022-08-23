#include "pcan_qt.h"
#include "include/PCANBasic.h"
#include "ui_pcan_qt.h"

PCAN_QT::PCAN_QT(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PCAN_QT)
{
    ui->setupUi(this);



    scan_channels();

    ui->CB_can_baud->clear();
    ui->CB_tpdo_channel->clear();
    ui->CB_tpdo_hz->clear();
    QStringList t_can_baud_list={"125", "250", "500", "1000"};
    QStringList t_TPDO_list={"1", "2", "3", "4", "5"};
    QStringList CB_tpdo_hz={"0", "5", "10", "20", "50", "100", "200"};
    ui->CB_bitrate->addItems({"125 kbit/s","250 kbit/s","500 kbit/s","1000 kbit/s"});
    ui->CB_bitrate->setCurrentIndex(2);
    ui->CB_can_baud->addItems(t_can_baud_list);
    ui->CB_can_baud->setCurrentIndex(2);
    ui->CB_tpdo_channel->addItems(t_TPDO_list);
    ui->CB_tpdo_hz->addItems(CB_tpdo_hz);
    ui->CB_tpdo_hz->setCurrentIndex(5);
    ui->SB_change_node_id->setRange(1,255);
    ui->SB_change_node_id->setValue(8);
    ui->SB_curr_node_id->setRange(1,255);
    ui->SB_curr_node_id->setValue(8);
    ui->BTN_init->setEnabled(true);
    ui->BTN_release->setEnabled(false);
    ui->GB_can_qsc->setEnabled(false);


    tmr_read= new QTimer();
    connect(tmr_read, &QTimer::timeout, this, &PCAN_QT::pcan_read);
    tmr_read->setInterval(3);

    tmr_1000ms= new QTimer();
    connect(tmr_1000ms, &QTimer::timeout, this, &PCAN_QT::calc_hz);
    tmr_1000ms->setInterval(1000);

}

PCAN_QT::~PCAN_QT()
{
    delete ui;


}

void PCAN_QT::pop_msgbox(QString text)
{
    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();
}

QString PCAN_QT::uchar_to_qstr( unsigned char *str, const int len ){
    QString result = "";
    int lengthOfString = len;

    // print string in reverse order
    QString s;
    for( int i = 0; i < lengthOfString; i++ ){
        s = QString("%1").arg(str[i],0,16);

        // account for single-digit hex values (always must serialize as two digits)
        if( s.length()== 1 )
            result.append("0");
        result.append(s.toUpper());
    }
    return result;
}
void PCAN_QT::qstr_to_uchar(QString qstr, uchar *str){
    QString hex_str="";

    if(qstr.length()<=16){
        for (int c = 0; c < qstr.length(); ++c) {
            hex_str += qstr[c];
            if ( c % 2){
                bool bStatus = false;
                uchar nHex = uchar(hex_str.toUInt(&bStatus,16));
                *(str++)=nHex;
                hex_str="";
            }
        }
    }
}

void PCAN_QT::scan_channels()
{
    DWORD channelsCount;
    ui->CB_can_channels->clear();
    if (CAN_GetValue(PCAN_NONEBUS, PCAN_ATTACHED_CHANNELS_COUNT, &channelsCount, 4) == PCAN_ERROR_OK)
    {
        if (channelsCount > 0)
        {

            if (CAN_GetValue(PCAN_NONEBUS, PCAN_ATTACHED_CHANNELS, &channels, channelsCount *sizeof(TPCANChannelInformation)) == PCAN_ERROR_OK)
            {
                for (int i = 0; i < (int)channelsCount; i++)
                {
                    QString dev_name=tr("%1,HND=%2,PCANID=%3").arg(channels.device_name).arg(channels.channel_handle).arg(channels.device_id);
                    ui->CB_can_channels->addItem(dev_name);
                }
            }
        }
    }

}

void PCAN_QT::can_init()
{
    if(ui->CB_can_channels->count()){
        channel_handle=ui->CB_can_channels->currentText().split(",")[1].remove("HND=").toUShort();
    }else{channel_handle=0;}

    if(ui->CB_bitrate->count()){

        uint tmp_bitrate=ui->CB_bitrate->currentText().split(" ")[0].toUInt();

        switch(tmp_bitrate){
        case(125):
            bitrate=PCAN_BAUD_125K;
            break;
        case(250):
            bitrate=PCAN_BAUD_250K;
            break;
        case(500):
            bitrate=PCAN_BAUD_500K;
            break;
        case(1000):
            bitrate=PCAN_BAUD_1M;
            break;
        default:
            bitrate=0;
            break;
        }
    }

    if(channel_handle&&bitrate){
        TPCANStatus result = 0;
        char strMsg[256];
        // The Plug & Play Channel (PCAN-USB) is initialized
        result = CAN_Initialize(channel_handle,bitrate);
        if(result != PCAN_ERROR_OK)
        {
            // An error occurred, get a text describing the error and show it
            CAN_GetErrorText(result, 0, strMsg);
            pop_msgbox(strMsg);
        }
        else{
            ui->TB_fastsdo_msgbox->append("PCAN-USB (Ch-1) was initialized.");

            ui->BTN_init->setEnabled(false);
            ui->BTN_release->setEnabled(true);
            ui->GB_can_qsc->setEnabled(true);

        }

        tmr_read->start();
        tmr_1000ms->start();
    }
    ui->GB_qsc_content->setEnabled(false);
}

void PCAN_QT::can_uninit()
{
    tmr_read->stop();
    tmr_1000ms->stop();
    CAN_Uninitialize(channel_handle);
    channel_handle=0;
    bitrate=0;
    tdpo_data.clear();
    tdpo_hz.clear();
    tdpo_ctr.clear();
    config_tpdo_hz[5]={0};
    ui->BTN_init->setEnabled(true);
    ui->BTN_release->setEnabled(false);
    ui->GB_can_qsc->setEnabled(false);
}

void PCAN_QT::calc_hz()
{
    QMap<QString, int>::const_iterator itr = tdpo_ctr.constBegin();
    while (itr != tdpo_ctr.constEnd()) {
        tdpo_hz[itr.key()]=itr.value();
        tdpo_ctr[itr.key()]=0;
        ++itr;
    }
}

void PCAN_QT::data_parser(TPCANMsg msg)
{

    QString rx_display="";
    rx_display.append(tr("%1%2%3%4\n").arg(tr("TPDO").leftJustified(10,' '))
                      .arg(tr("DLC").leftJustified(10,' '))
                      .arg(tr("DATA").leftJustified(30,' '))
                      .arg(tr("Hz").leftJustified(10,' ')));

    QString tpdo_id_hex = QString("0x%1").arg(msg.ID, 3, 16, QLatin1Char( '0' ));
    QString data=uchar_to_qstr(msg.DATA,msg.LEN);



    tdpo_data[tpdo_id_hex]=data;

    QMap<QString, QString>::const_iterator itr = tdpo_data.constBegin();
    while (itr != tdpo_data.constEnd()) {
        QString tmp_tdpo=itr.key();
        int tmp_len=itr.value().length()/2;

        QString tmp_data=itr.value();
        for (int i = 2; i <= tmp_data.size(); i+=2+1)
            tmp_data.insert(i, " ");

        QString tmp_hz;
        if (tdpo_hz.contains(tmp_tdpo))
            tmp_hz = tr("%1").arg(tdpo_hz.value(tmp_tdpo));

        rx_display.append(tr("%1%2%3%4\n").arg(tmp_tdpo.leftJustified(10,' '))
                          .arg(QString::number(tmp_len).leftJustified(10,' '))
                          .arg(tmp_data.leftJustified(30,' '))
                          .arg(tmp_hz.leftJustified(10,' ')));
        ++itr;
    }


    ui->Label_can_rx->setText(rx_display);

    if(!tdpo_ctr.contains(tpdo_id_hex))
        tdpo_ctr[tpdo_id_hex]=0;
    else{
        tdpo_ctr[tpdo_id_hex]+=1;
    }
    ui->Label_can_rx->setText(rx_display);

    //read config
    if(flag_reading_cfg){
        if(msg.ID-node_id==0x580){
            flag_readcfg_succ=true;

            QString qstr_data=uchar_to_qstr(msg.DATA,msg.LEN);

            QString line=tr("(%1) PTO:%2, DLC:%3, DATA:%4")
                    .arg("RX")
                    .arg(tpdo_id_hex)
                    .arg(msg.LEN)
                    .arg(qstr_data);
            ui->TB_fastsdo_msgbox->append(line);

            QStringList datalist;
            QRegularExpressionMatchIterator i = QRegularExpression("..?").globalMatch(qstr_data);
            while (i.hasNext()) {
                datalist << i.next().captured(0);
            }
            if(datalist.at(0)!="60"){
                if(datalist.at(2)+datalist.at(1)=="2100"){//BAUD
                    uint baud=(datalist.at(7).toUInt(NULL,16)<<24)
                            +(datalist.at(6).toUInt(NULL,16)<<16)
                            +(datalist.at(5).toUInt(NULL,16)<<8)
                            +datalist.at(4).toUInt(NULL,16);
                    for(int i=0;i<ui->CB_can_baud->count();i++){
                        if(ui->CB_can_baud->itemText(i).toInt()==baud/1000){
                            ui->CB_can_baud->setCurrentIndex(i);
                        }
                    }
                }
                else if(datalist.at(2)+datalist.at(1)=="2101"){//ID
                    uint id=(datalist.at(7).toUInt(NULL,16)<<24)
                            +(datalist.at(6).toUInt(NULL,16)<<16)
                            +(datalist.at(5).toUInt(NULL,16)<<8)
                            +datalist.at(4).toUInt(NULL,16);
                    ui->SB_change_node_id->setValue(id);
                }
                else if(datalist.at(2)=="18"){//TPDO FREQUENCY

                    uint freq=1000/((datalist.at(5).toUInt(NULL,16)<<8)
                                    +datalist.at(4).toUInt(NULL,16));

                    if(datalist.at(1)=="00"){config_tpdo_hz[0]=freq;}
                    else if(datalist.at(1)=="01"){config_tpdo_hz[1]=freq;}
                    else if(datalist.at(1)=="02"){config_tpdo_hz[2]=freq;}
                    else if(datalist.at(1)=="03"){config_tpdo_hz[3]=freq;}
                    else if(datalist.at(1)=="04"){config_tpdo_hz[4]=freq;}
                    update_config_tpdo_hz();
                }

            }
        }
    }
}

void PCAN_QT::imu_parser(TPCANMsg msg)
{

    if(node_id!=0){
        int TPDO=msg.ID-node_id;

        if(TPDO==0x180){
            //Accelerometer[G]
            m_imu_data.acc[0]=((float)(int16_t)((uint)msg.DATA[0]|(uint)msg.DATA[1]<<8))/1000;
            m_imu_data.acc[1]=((float)(int16_t)((uint)msg.DATA[2]|(uint)msg.DATA[3]<<8))/1000;
            m_imu_data.acc[2]=((float)(int16_t)((uint)msg.DATA[4]|(uint)msg.DATA[5]<<8))/1000;
        }
        else if(TPDO==0x280){
            //Gyroscope[deg/s]
            m_imu_data.gyr[0]=((float)(int16_t)((uint)msg.DATA[0]|(uint)msg.DATA[1]<<8))/10;
            m_imu_data.gyr[1]=((float)(int16_t)((uint)msg.DATA[2]|(uint)msg.DATA[3]<<8))/10;
            m_imu_data.gyr[2]=((float)(int16_t)((uint)msg.DATA[4]|(uint)msg.DATA[5]<<8))/10;
        }
        else if(TPDO==0x380){
            //Euler Angle[deg]
            m_imu_data.eul[0]=((float)(int16_t)((uint)msg.DATA[0]|(uint)msg.DATA[1]<<8))/100;
            m_imu_data.eul[1]=((float)(int16_t)((uint)msg.DATA[2]|(uint)msg.DATA[3]<<8))/100;
            m_imu_data.eul[2]=((float)(int16_t)((uint)msg.DATA[4]|(uint)msg.DATA[5]<<8))/100;
        }
        else if(TPDO==0x480){
            //Quaternion
            m_imu_data.quat[0]=((float)(int16_t)((uint)msg.DATA[0]|(uint)msg.DATA[1]<<8))/10000;
            m_imu_data.quat[1]=((float)(int16_t)((uint)msg.DATA[2]|(uint)msg.DATA[3]<<8))/10000;
            m_imu_data.quat[2]=((float)(int16_t)((uint)msg.DATA[4]|(uint)msg.DATA[5]<<8))/10000;
            m_imu_data.quat[3]=((float)(int16_t)((uint)msg.DATA[6]|(uint)msg.DATA[7]<<8))/10000;
        }
        else if(TPDO==0x680){
            m_imu_data.prs=0;
        }

        QString str;

        str.clear();

        str.append(QString("%1%2%3%4\n").arg(tr(" "),20).arg(tr("X"),10).arg(tr("Y"),10).arg(tr("Z"),10));
        str.append(QString("%1%2%3%4\n").arg(tr("Accelerometer[G] :").leftJustified(20,' ')).arg(QString::number(m_imu_data.acc[0], 'f', 3), 10).arg(QString::number(m_imu_data.acc[1], 'f', 3), 10).arg(QString::number(m_imu_data.acc[2], 'f', 3), 10));
        str.append(QString("%1%2%3%4\n").arg(tr("Gyroscope[deg/s] :").leftJustified(20,' ')).arg(QString::number(m_imu_data.gyr[0], 'f', 3), 10).arg(QString::number(m_imu_data.gyr[1], 'f', 3), 10).arg(QString::number(m_imu_data.gyr[2], 'f', 3), 10));
        str.append(QString("%1%2%3%4\n").arg(tr("Euler Angle[deg] :").leftJustified(20,' ')).arg(QString::number(m_imu_data.eul[0], 'f', 3), 10).arg(QString::number(m_imu_data.eul[1], 'f', 3), 10).arg(QString::number(m_imu_data.eul[2], 'f', 3), 10));
        str.append(QString("%1%2%3%4%5\n").arg(tr(" "),20).arg(tr("W"),10).arg(tr("X"),10).arg(tr("Y"),10).arg(tr("Z"),10));
        str.append(QString("%1%2%3%4%5\n").arg(tr("Quaternion :").leftJustified(20,' ')).arg(QString::number(m_imu_data.quat[0], 'f', 3), 10).arg(QString::number(m_imu_data.quat[1], 'f', 3), 10).arg(QString::number(m_imu_data.quat[2], 'f', 3), 10).arg(QString::number(m_imu_data.quat[3], 'f', 3), 10));


        ui->Label_imudata->setText(str);
    }
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
        result = CAN_Read(PCAN_USBBUS1,&msg,&timestamp);
        if(result != PCAN_ERROR_QRCVEMPTY)
        {
            // Process the received message
            data_parser(msg);
            imu_parser(msg);
        }
        else
        {
            // An error occurred, get a text describing the error and show it
            // and handle the error
            CAN_GetErrorText(result, 0, strMsg);
        }
        // Try to read a message from the receive queue of the PCAN-USB, Channel 1,
        // until the queue is empty
    }while((result & PCAN_ERROR_QRCVEMPTY) != PCAN_ERROR_QRCVEMPTY);

}

void PCAN_QT::pcan_send(TPCANMsg msg)
{

    TPCANStatus result;
    char strMsg[256];

    // A CAN message is configured
    //
    msg.MSGTYPE = PCAN_MESSAGE_STANDARD;

    // The message is sent using the PCAN-USB Channel 1
    //
    result = CAN_Write(PCAN_USBBUS1, &msg);
    if(result != PCAN_ERROR_OK)
    {
        // An error occurred, get a text describing the error and show it
        //
        CAN_GetErrorText(result, 0, strMsg);
        pop_msgbox(strMsg);

    }
    else{
        QString tpdo_id_hex = QString("0x%1").arg(msg.ID, 3, 16, QLatin1Char( '0' ));
        QString line=tr("(%1) PTO:%2, DLC:%3, DATA:%4")
                .arg("TX")
                .arg(tpdo_id_hex)
                .arg(msg.LEN)
                .arg(uchar_to_qstr(msg.DATA,msg.LEN));
        ui->TB_fastsdo_msgbox->append(line);
    }

}

void PCAN_QT::fastsdo_readcfg()
{
    TPCANMsg ask_msg;

    ask_msg.ID=node_id + 0x600;
    ask_msg.LEN=8;

    //bitrate
    qstr_to_uchar("4000210000000000",ask_msg.DATA);
    pcan_send(ask_msg);

    //read node id
    qstr_to_uchar("4001210000000000",ask_msg.DATA);
    pcan_send(ask_msg);

    //read freq of TPDO 1
    qstr_to_uchar("4000180500000000",ask_msg.DATA);
    pcan_send(ask_msg);

    //read freq of TPDO 2
    qstr_to_uchar("4001180500000000",ask_msg.DATA);
    pcan_send(ask_msg);

    //read freq of TPDO 3
    qstr_to_uchar("4002180500000000",ask_msg.DATA);
    pcan_send(ask_msg);

    //read freq of TPDO 4
    qstr_to_uchar("4003180500000000",ask_msg.DATA);
    pcan_send(ask_msg);

    //read freq of TPDO 5
    qstr_to_uchar("4004180500000000",ask_msg.DATA);
    pcan_send(ask_msg);

}

void PCAN_QT::reading_cfg()
{
    if(flag_readcfg_succ){
        ui->GB_qsc_content->setEnabled(true);
    }else{
        ui->GB_qsc_content->setEnabled(false);
    }
    flag_reading_cfg=false;
    flag_readcfg_succ=false;
}

void PCAN_QT::update_config_tpdo_hz()
{
    for(int i=0;i<ui->CB_tpdo_hz->count();i++){
        if(ui->CB_tpdo_hz->itemText(i).toUInt()==config_tpdo_hz[ui->CB_tpdo_channel->currentIndex()]){
            ui->CB_tpdo_hz->setCurrentIndex(i);
        }
    }
}


void PCAN_QT::on_BTN_refresh_channel_clicked()
{
    scan_channels();
}
void PCAN_QT::on_BTN_init_clicked()
{
    can_init();
}
void PCAN_QT::on_BTN_release_clicked()
{
    can_uninit();
}


void PCAN_QT::on_BTN_read_config_clicked()
{
    node_id=ui->SB_curr_node_id->value();
    ui->Line_fastsdo_txid->setText(QString::number(node_id + 0x600, 16));

    flag_reading_cfg=true;
    fastsdo_readcfg();
    QTimer::singleShot(500, this,SLOT(reading_cfg()));

}
void PCAN_QT::on_BTN_fastsdo_send_clicked()
{

    TPCANMsg msg;

    //convert string to upper size
    QString upper_str=ui->Line_fastsdo_data->text().toUpper();
    ui->Line_fastsdo_data->setText(upper_str);
    QString tmp_data=ui->Line_fastsdo_data->text();

    //convert hex of id string to int
    bool bStatus = false;
    uint id_Hex = ui->Line_fastsdo_txid->text().toUInt(&bStatus,16);

    //save data in the msg package
    msg.ID=id_Hex;
    msg.LEN=uchar(ui->SB_fastsdo_dlc->text().toUInt());
    qstr_to_uchar(tmp_data,msg.DATA);

    pcan_send(msg);
}
void PCAN_QT::on_BTN_change_baud_clicked()
{
    uint decimal = ui->CB_can_baud->currentText().toUInt()*1000;
    QString hex = QString("%1").arg(decimal, 8, 16, QLatin1Char( '0' ));
    QString reverse_hex="";

    for (int c = 0; c < hex.length(); c=c+2) {
        QString t_hex = "";
        t_hex=hex[c];
        t_hex+=hex[c+1];
        reverse_hex=t_hex+reverse_hex;
    }
    ui->Line_fastsdo_txid->setText(QString::number(node_id + 0x600, 16));
    ui->SB_fastsdo_dlc->setValue(8);
    ui->Line_fastsdo_data->setText("23002100"+reverse_hex);

    on_BTN_fastsdo_send_clicked();
    pop_msgbox(tr("Re-Power the module to apply change."));
}
void PCAN_QT::on_BTN_change_node_id_clicked()
{
    uint decimal = uint(ui->SB_change_node_id->value());
    QString hex = QString("%1").arg(decimal, 8, 16, QLatin1Char( '0' ));
    QString reverse_hex="";

    for (int c = 0; c < hex.length(); c=c+2) {
        QString t_hex = "";
        t_hex=hex[c];
        t_hex+=hex[c+1];
        reverse_hex=t_hex+reverse_hex;
    }

    ui->Line_fastsdo_txid->setText(QString::number(node_id + 0x600, 16));
    ui->SB_fastsdo_dlc->setValue(8);
    ui->Line_fastsdo_data->setText("23012100"+reverse_hex);

    on_BTN_fastsdo_send_clicked();
    pop_msgbox(tr("Re-Power the module to apply change."));
}
void PCAN_QT::on_BTN_change_tpdo_hz_clicked()
{
    uint channel_tpdo_decimal = ui->CB_tpdo_channel->currentText().toUInt()-1;
    uint hz_tpdo_decimal = ui->CB_tpdo_hz->currentText().toUInt();
    uint interval_tpdo_decimal;

    if(hz_tpdo_decimal==0){
        interval_tpdo_decimal=0;
    }
    else{
        interval_tpdo_decimal=1000/hz_tpdo_decimal;
    }

    QString channel_tpdo_hex = QString("%1").arg(channel_tpdo_decimal, 2, 16, QLatin1Char( '0' ));
    QString interval_tpdo_hex = QString("%1").arg(interval_tpdo_decimal, 8, 16, QLatin1Char( '0' ));
    QString reverse_hex="";

    for (int c = 0; c < interval_tpdo_hex.length(); c=c+2) {
        QString t_hex = "";
        t_hex=interval_tpdo_hex[c];
        t_hex+=interval_tpdo_hex[c+1];
        reverse_hex=t_hex+reverse_hex;
    }
    ui->Line_fastsdo_txid->setText(QString::number(node_id + 0x600, 16));
    ui->SB_fastsdo_dlc->setValue(8);
    ui->Line_fastsdo_data->setText("2B"+channel_tpdo_hex+"1805"+reverse_hex);

    on_BTN_fastsdo_send_clicked();
    pop_msgbox(tr("Re-Power the module to apply change."));
}
void PCAN_QT::on_CB_tpdo_channel_currentIndexChanged(int index)
{
    update_config_tpdo_hz();
}
void PCAN_QT::on_BTN_clr_TB_fastsdo_msgbox_clicked()
{
    ui->TB_fastsdo_msgbox->clear();
}

